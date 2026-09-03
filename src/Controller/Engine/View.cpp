#include "View.hpp"

#include "../Types.hpp"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>

#include "Key.hpp"

namespace Controller
{
  namespace engine
  {
    using namespace Controller::aim;

    namespace
    {
      constexpr float move_scale {127.0f};

      signed char
      clamp_move (int v) noexcept
      {
        return static_cast<signed char> (std::clamp (v, -128, 127));
      }

      std::optional<std::vector<knot>>
      read_engine_graph (int index)
      {
        if (index < 0 ||
            static_cast<unsigned> (index) >= AIM_ASSIST_GRAPH_COUNT)
          return std::nullopt;

        const GraphFloat& g (aaInputGraph[index]);
        const size_t n (static_cast<size_t> (g.knotCount));

        if (n < 2 || n > max_graph_knots)
          return std::nullopt;

        std::vector<knot> knots;
        knots.reserve (n);

        for (size_t i (0); i != n; ++i)
          knots.push_back (knot {g.knots[i][0], g.knots[i][1] * g.scale});

        return knots;
      }

      aim_profile
      make_profile (const dvars& d, float yaw, float pitch)
      {
        aim_profile p;
        p.yaw_rate = deg_per_s {yaw};
        p.pitch_rate = deg_per_s {pitch};
        p.deadzone = deadzone_params {
          magnitude {read (d.stick_deadzone_min, 0.2f)},
          magnitude {read (d.stick_deadzone_max, 0.01f)},
          magnitude {0.0f}};
        p.curve = response_curve {curve_kind::linear, 1.0f};
        return p;
      }

      size_t
      tuning_signature (const dvars& d)
      {
        const float values[] {
          read (d.turnrate_yaw, 260.0f),
          read (d.turnrate_yaw_ads, 90.0f),
          read (d.turnrate_pitch, 90.0f),
          read (d.turnrate_pitch_ads, 55.0f),
          read (d.stick_deadzone_min, 0.2f),
          read (d.stick_deadzone_max, 0.01f),
          read (d.accel_rate, 1200.0f),
          static_cast<float> (read (d.accel_enabled, false)),
          static_cast<float> (read (d.graph_enabled, true)),
          static_cast<float> (read (d.graph_index, 3)),
        };

        size_t h (0);
        for (float v: values)
          h = h * 31u + std::hash<float> {} (v);

        return h;
      }

      constexpr float slowdown_pitch_hip {0.4f};
      constexpr float slowdown_pitch_ads {0.5f};
      constexpr float slowdown_yaw_hip {0.4f};
      constexpr float slowdown_yaw_ads {0.5f};

      bool
      in_region (const AimScreenTarget& t, float half_w, float half_h) noexcept
      {
        return half_w >= t.clipMins[0] && t.clipMaxs[0] >= -half_w &&
               half_h >= t.clipMins[1] && t.clipMaxs[1] >= -half_h;
      }

      const AimScreenTarget*
      best_target (const AimAssistGlobals& aa,
                   float range,
                   float half_w,
                   float half_h) noexcept
      {
        const float range_sqr (range * range);

        for (int i (0); i != aa.screenTargetCount; ++i)
        {
          const AimScreenTarget& t (aa.screenTargets[i]);

          if (t.distSqr <= range_sqr && in_region (t, half_w, half_h))
            return &t;
        }

        return nullptr;
      }

      float
      assist_range (const AimAssistGlobals& aa, float scale) noexcept
      {
        const WeaponDef* wd (aa.ps.weapIndex != 0
                             ? BG_GetWeaponDef (static_cast<unsigned> (aa.ps.weapIndex))
                             : nullptr);

        if (wd == nullptr)
          return 0.0f;

        return std::lerp (wd->aimAssistRange,
                          wd->aimAssistRangeAds,
                          std::clamp (aa.adsLerp, 0.0f, 1.0f)) * scale;
      }

      bool
      using_offhand (const AimAssistPlayerState& ps) noexcept
      {
        if ((ps.weapFlags & PWF_USING_OFFHAND) == 0 || ps.weapIndex == 0)
          return false;

        const WeaponDef* wd (BG_GetWeaponDef (static_cast<unsigned> (ps.weapIndex)));
        return wd != nullptr && wd->offhandClass != OFFHAND_CLASS_NONE;
      }

      void
      advance_view (int client, degrees yaw_delta, degrees pitch_delta) noexcept
      {
        view_yaw (client) -= yaw_delta.value;
        view_pitch (client) -= pitch_delta.value;
      }
    }

    view_driver::
    view_driver (const context& ctx, const dvars& d)
      : ctx_ (ctx), dvars_ (d)
    {
    }

    bool
    view_driver::
    ensure_processor ()
    {
      const size_t sig (tuning_signature (dvars_));

      if (have_signature_ && sig == tuning_signature_ && processor_)
        return true;

      tuning_signature_ = sig;
      have_signature_ = true;

      aim_settings s;
      s.hip = make_profile (dvars_,
                            read (dvars_.turnrate_yaw, 260.0f),
                            read (dvars_.turnrate_pitch, 90.0f));
      s.ads = make_profile (dvars_,
                            read (dvars_.turnrate_yaw_ads, 90.0f),
                            read (dvars_.turnrate_pitch_ads, 55.0f));

      const float accel (read (dvars_.accel_enabled, false)
                         ? read (dvars_.accel_rate, 1200.0f)
                         : 0.0f);

      s.accel = turn_integrator::limits {deg_per_s2 {accel}, deg_per_s2 {0.0f}};

      if (read (dvars_.graph_enabled, true))
        s.graph_knots = read_engine_graph (read (dvars_.graph_index, 3));

      s.graph_monotonic = false;

      std::string why;
      std::optional<aim_calibration> cal (aim_calibration::make (s, why));

      if (!cal)
      {
        if (!reported_invalid_)
        {
          reported_invalid_ = true;
          ctx_.report (severity::warning, facility::aim, errc::graph_invalid,
                       "aim configuration rejected (" + why +
                       "); controller view runs unshaped until it is corrected");
        }

        calibration_.reset ();
        processor_.reset ();
        return false;
      }

      reported_invalid_ = false;
      calibration_.emplace (std::move (*cal));
      processor_.emplace (calibration_->processor_config ());
      return true;
    }

    void
    view_driver::
    observe (const canonical_sample& s) noexcept
    {
      const stick_vector left (s.sticks[static_cast<size_t> (stick::left)].calibrated);
      const stick_vector right (s.sticks[static_cast<size_t> (stick::right)].calibrated);

      const mapping::stick_layout layout (
        mapping::stick_layout_from_name (read (dvars_.sticks_config,
                                               "thumbstick_default")));

      axes_ = mapping::resolve (layout, left, right);
    }

    void
    view_driver::
    idle () noexcept
    {
      axes_ = mapping::resolved_axes {};

      if (processor_)
        processor_->reset ();
    }

    bool
    view_driver::
    view_active (int client) const noexcept
    {
      if (Key_IsCatcherActive (client, KEYCATCH_MASK_ANY) &&
          UI_GetActiveMenu (client) != UIMENU_SCOREBOARD)
        return false;

      return (pm_flags (client) & PMF_FROZEN) == 0;
    }

    void
    view_driver::
    apply_move (int client, usercmd_s& cmd, float frame_time) noexcept
    {
      cmd.forwardmove = clamp_move (cmd.forwardmove +
        static_cast<int> (std::lround (axes_.forward * move_scale)));
      cmd.rightmove = clamp_move (cmd.rightmove +
        static_cast<int> (std::lround (axes_.side * move_scale)));

      if (last_weapon_hand (client) == WEAPON_HAND_LEFT)
      {
        const int b (cmd.buttons);
        cmd.buttons &= ~(CMD_BUTTON_ATTACK | CMD_BUTTON_THROW);

        if (b & CMD_BUTTON_ATTACK)
          cmd.buttons |= CMD_BUTTON_THROW;
        if (b & CMD_BUTTON_THROW)
          cmd.buttons |= CMD_BUTTON_ATTACK;
      }

      if (!view_active (client) || !ensure_processor ())
        return;

      AimInput in {};
      AimOutput out {};
      in.deltaTime = frame_time;
      in.deltaTimeScaled = client_frame_time ();
      in.pitch = view_pitch (client);
      in.pitchAxis = axes_.pitch;
      in.pitchMax = max_pitch_speed (client);
      in.yaw = view_yaw (client);
      in.yawAxis = axes_.yaw;
      in.yawMax = max_yaw_speed (client);
      in.forwardAxis = axes_.forward;
      in.rightAxis = axes_.side;
      in.buttons = cmd.buttons;
      in.localClientNum = client;

      aim_assist_update (in, out);

      view_pitch (client) = out.pitch;
      view_yaw (client) = out.yaw;
      cmd.meleeChargeYaw = out.meleeChargeYaw;
      cmd.meleeChargeDist = out.meleeChargeDist;

      stick_vector look {axes_.yaw, axes_.pitch};

      if (read (dvars_.scale_view_axis, true))
        look = scale_dominant_axis (look);

      const AimAssistGlobals& aa (aaGlobArray[client]);

      aim_frame_input fi;
      fi.look = look;
      fi.ads_lerp = aa.initialized ? aa.adsLerp : 0.0f;
      fi.fov_scale = aa.initialized ? aa.fovTurnRateScale : 1.0f;
      fi.sensitivity = read (dvars_.view_sensitivity, 1.0f);
      fi.invert_pitch = read (dvars_.invert_pitch, false);
      fi.dt = seconds {frame_time};

      const bool assist_allowed (read (dvars_.aim_assist_enabled, true));

      if (aa.initialized && assist_allowed &&
          read (dvars_.slowdown_enabled, true))
      {
        const float range (
          assist_range (aa, read (dvars_.aim_assist_range_scale, 1.0f)));

        const bool present (
          best_target (aa, range,
                       aa.tweakables.slowdownRegionWidth,
                       aa.tweakables.slowdownRegionHeight) != nullptr);

        fi.slowdown_yaw = slowdown_scale (present, slowdown_yaw_hip,
                                          slowdown_yaw_ads, aa.adsLerp);
        fi.slowdown_pitch = using_offhand (aa.ps)
          ? 1.0f
          : slowdown_scale (present, slowdown_pitch_hip,
                            slowdown_pitch_ads, aa.adsLerp);
      }

      if (max_yaw_speed (client) > 0.0f)
        fi.yaw_max = deg_per_s {max_yaw_speed (client)};
      if (max_pitch_speed (client) > 0.0f)
        fi.pitch_max = deg_per_s {max_pitch_speed (client)};

      aim_frame_output o (processor_->process (fi));

      if (aa.initialized && assist_allowed &&
          read (dvars_.lockon_enabled, true))
      {
        const float deflection (std::sqrt (look.x * look.x + look.y * look.y));

        if (deflection <= read (dvars_.lockon_deflection, 0.05f))
        {
          const float range (
            assist_range (aa, read (dvars_.aim_assist_range_scale, 1.0f)));

          if (const AimScreenTarget* t = best_target (
                aa, range,
                aa.tweakables.lockOnRegionWidth,
                aa.tweakables.lockOnRegionHeight);
              t != nullptr && t->distSqr > 0.0f)
          {
            lock_on_target lt;
            lt.target_velocity = {t->velocity[0], t->velocity[1], t->velocity[2]};
            lt.player_velocity = {aa.ps.velocity[0], aa.ps.velocity[1],
                                  aa.ps.velocity[2]};
            lt.view_yaw_axis = {-aa.viewAxis[1][0], -aa.viewAxis[1][1],
                                -aa.viewAxis[1][2]};
            lt.view_pitch_axis = {aa.viewAxis[2][0], aa.viewAxis[2][1],
                                  aa.viewAxis[2][2]};
            lt.distance = std::sqrt (t->distSqr);

            lock_on_params lp;
            lp.yaw_strength = read (dvars_.lockon_strength, 0.6f);
            lp.pitch_strength = read (dvars_.lockon_pitch_strength, 0.6f);

            const aim_frame_output l (lock_on (lt, lp, seconds {frame_time}));
            o.yaw_delta = o.yaw_delta + l.yaw_delta;
            o.pitch_delta = o.pitch_delta + l.pitch_delta;
          }
        }
      }

      advance_view (client, o.yaw_delta, o.pitch_delta);
    }

    void
    view_driver::
    apply_remote_move (int, usercmd_s& cmd) noexcept
    {
      const float sensitivity (read (dvars_.view_sensitivity, 1.0f));

      cmd.remoteControlAngles[0] = clamp_move (
        cmd.remoteControlAngles[0] +
        static_cast<int> (std::lround (-(axes_.forward + axes_.pitch) *
                                       move_scale * sensitivity)));

      cmd.remoteControlAngles[1] = clamp_move (
        cmd.remoteControlAngles[1] +
        static_cast<int> (std::lround (-(axes_.side + axes_.yaw) *
                                       move_scale * sensitivity)));
    }

    void
    view_driver::
    apply_location_selection (int client) noexcept
    {
      Game::cg_s* const cg (Game::CL_GetLocalClientGlobals (client));

      if (cg == nullptr)
        return;

      const float frame_time (static_cast<float> (cg->frametime) * 0.001f);

      const float aspect (cg->compassMapWorldSize[1] != 0.0f
                          ? cg->compassMapWorldSize[0] / cg->compassMapWorldSize[1]
                          : 1.0f);

      float up (axes_.forward);
      float right (axes_.side);

      const float magnitude (up * up + right * right);

      if (magnitude > 1.0f)
      {
        const float m (std::sqrt (magnitude));
        up /= m;
        right /= m;
      }

      if (cursor_speed_ == nullptr)
        cursor_speed_ = Dvar_FindVar ("cg_mapLocationSelectionCursorSpeed");

      const float speed (read (cursor_speed_, 100.0f));

      cg->selectedLocation[0] += right * speed * frame_time;
      cg->selectedLocation[1] -= up * aspect * speed * frame_time;

      if ((cg->predictedPlayerState.locationSelectionInfo & 0x80) != 0 &&
          (axes_.pitch != 0.0f || axes_.yaw != 0.0f))
      {
        Game::vec2_t v {axes_.pitch, -axes_.yaw};

        cg->selectedLocationAngle =
          Game::AngleNormalize360 (Game::vectoryaw (&v));
        cg->selectedAngleLocation[0] = cg->selectedLocation[0];
        cg->selectedAngleLocation[1] = cg->selectedLocation[1];
      }
      else
      {
        cg->selectedAngleLocation[0] = cg->selectedLocation[0];
        cg->selectedAngleLocation[1] = cg->selectedLocation[1];
      }
    }
  }
}
