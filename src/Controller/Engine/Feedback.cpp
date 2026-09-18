#include "Feedback.hpp"

#include "../Types.hpp"

#include <cstring>

#include "Key.hpp"
#include "../Mapping/Key.hpp"

namespace Controller
{
  namespace engine
  {
    namespace
    {
      using driver::adaptive_trigger_request;
      using driver::trigger_effect;

      enum class trigger_role : uint8_t
      {
        none,
        firing,
        aiming,
        primary_offhand,
        secondary_offhand,
      };

      trigger_role
      role_for (mapping::engine_key k) noexcept
      {
        const char* const b (
          playerKeys[local_client].keys[static_cast<int> (k)].binding);

        if (b == nullptr)
          return trigger_role::none;

        if (std::strcmp (b, "+attack") == 0)
          return trigger_role::firing;

        if (std::strcmp (b, "+speed_throw") == 0 ||
            std::strcmp (b, "+toggleads_throw") == 0)
          return trigger_role::aiming;

        if (std::strcmp (b, "+frag") == 0)
          return trigger_role::primary_offhand;

        if (std::strcmp (b, "+smoke") == 0)
          return trigger_role::secondary_offhand;

        return trigger_role::none;
      }

      adaptive_trigger_request
      released (trigger_side side) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::off;
        return r;
      }

      adaptive_trigger_request
      profile (trigger_side side, const driver::trigger_profile& zones) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::feedback;
        r.zones = zones;
        return r;
      }

      driver::trigger_profile
      ramp (uint8_t from, uint8_t to) noexcept
      {
        driver::trigger_profile z {};

        constexpr size_t last (driver::trigger_zone_count - 1);

        for (size_t i (0); i != driver::trigger_zone_count; ++i)
          z[i] = static_cast<uint8_t> (
            from + (static_cast<int> (to) - from) * static_cast<int> (i) / last);

        return z;
      }

      driver::trigger_profile
      flat (uint8_t strength) noexcept
      {
        driver::trigger_profile z {};
        z.fill (strength);
        return z;
      }

      adaptive_trigger_request
      section (trigger_side side,
               uint8_t start,
               uint8_t end,
               uint8_t strength) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::weapon;
        r.start_position = start;
        r.end_position = end;
        r.strength = strength;
        return r;
      }

      constexpr uint8_t slight {7};
      constexpr uint8_t heavy {8};

      constexpr uint8_t light_break_start {2};
      constexpr uint8_t light_break_end {5};
      constexpr uint8_t hard_break_start {3};
      constexpr uint8_t hard_break_end {7};

      constexpr uint8_t max_strength {8};
      constexpr uint8_t max_position {
        static_cast<uint8_t> (driver::trigger_zone_count - 1)};

      struct tuning
      {
        uint8_t light {0};
        uint8_t heavy {0};

        uint8_t light_start {0};
        uint8_t light_end {0};
        uint8_t hard_start {0};
        uint8_t hard_end {0};

        uint8_t ads {0};
      };

      uint8_t
      clamp_to (int v, uint8_t limit) noexcept
      {
        return static_cast<uint8_t> (v < 0 ? 0 : (v > limit ? limit : v));
      }

      uint8_t
      scaled_strength (int configured, float scale) noexcept
      {
        const float v (static_cast<float> (clamp_to (configured, max_strength)) *
                       (scale < 0.0f ? 0.0f : (scale > 1.0f ? 1.0f : scale)));

        return clamp_to (static_cast<int> (v + 0.5f), max_strength);
      }

      tuning
      read_tuning (const dvars& d) noexcept
      {
        const float scale (read (d.adaptive_trigger_strength, 1.0f));

        tuning t;

        t.light = scaled_strength (
          read (d.adaptive_trigger_light, static_cast<int> (slight)), scale);
        t.heavy = scaled_strength (
          read (d.adaptive_trigger_heavy, static_cast<int> (heavy)), scale);
        t.ads = scaled_strength (read (d.adaptive_trigger_ads, 0), scale);

        t.light_start = clamp_to (
          read (d.adaptive_trigger_light_start,
                static_cast<int> (light_break_start)), max_position);
        t.light_end = clamp_to (
          read (d.adaptive_trigger_light_end,
                static_cast<int> (light_break_end)), max_position);
        t.hard_start = clamp_to (
          read (d.adaptive_trigger_heavy_start,
                static_cast<int> (hard_break_start)), max_position);
        t.hard_end = clamp_to (
          read (d.adaptive_trigger_heavy_end,
                static_cast<int> (hard_break_end)), max_position);

        t.light_end = std::max (t.light_end, t.light_start);
        t.hard_end = std::max (t.hard_end, t.hard_start);

        return t;
      }

      adaptive_trigger_request
      firing_feedback (trigger_side side,
                       const playerState_s& ps,
                       const tuning& t) noexcept
      {
        const int index (BG_GetViewModelWeaponIndex (&ps));

        if (index == 0)
          return released (side);

        const WeaponDef* const w (
          BG_GetWeaponDef (static_cast<unsigned> (index)));

        if (w == nullptr)
          return released (side);

        switch (w->weapClass)
        {
          case Game::WEAPCLASS_MG:
          case Game::WEAPCLASS_RIFLE:
          case Game::WEAPCLASS_TURRET:
            return profile (side, flat (t.heavy));

          case Game::WEAPCLASS_SMG:
            return profile (side, ramp (t.light, t.heavy));

          case Game::WEAPCLASS_PISTOL:
            return section (side, t.light_start, t.light_end, t.light);

          case Game::WEAPCLASS_SPREAD:
          case Game::WEAPCLASS_SNIPER:
          case Game::WEAPCLASS_ROCKETLAUNCHER:
            return section (side, t.hard_start, t.hard_end, t.heavy);

          default:
            return released (side);
        }
      }

      adaptive_trigger_request
      offhand_feedback (trigger_side side,
                        const playerState_s& ps,
                        bool primary,
                        const tuning& t) noexcept
      {
        const int held (primary ? ps.weapCommon.offhandPrimary
                                : ps.weapCommon.offhandSecondary);

        return held != Game::OFFHAND_CLASS_NONE
          ? section (side, t.light_start, t.light_end, t.light)
          : released (side);
      }

      adaptive_trigger_request
      aiming_feedback (trigger_side side, const tuning& t) noexcept
      {
        return t.ads != 0 ? profile (side, flat (t.ads)) : released (side);
      }
    }

    bool
    evaluate_trigger_feedback (const dvars& d,
                               int client,
                               adaptive_trigger_request& left,
                               adaptive_trigger_request& right) noexcept
    {
      if (!read (d.adaptive_triggers, false))
        return false;

      Game::cg_s* const cg (Game::CL_GetLocalClientGlobals (client));

      if (cg == nullptr || cg->snap == nullptr)
        return false;

      const playerState_s& ps (cg->snap->ps);

      const trigger_role l (role_for (mapping::engine_key::button_ltrig));
      const trigger_role r (role_for (mapping::engine_key::button_rtrig));

      const tuning t (read_tuning (d));

      const auto effect_for = [&ps, &t] (trigger_side side, trigger_role role)
      {
        switch (role)
        {
          case trigger_role::firing:
            return firing_feedback (side, ps, t);

          case trigger_role::aiming:
            return aiming_feedback (side, t);

          case trigger_role::none:
            return released (side);

          case trigger_role::primary_offhand:
            return offhand_feedback (side, ps, true, t);

          case trigger_role::secondary_offhand:
            return offhand_feedback (side, ps, false, t);
        }

        return released (side);
      };

      left = effect_for (trigger_side::left, l);
      right = effect_for (trigger_side::right, r);

      if (l == trigger_role::firing && r == trigger_role::firing)
      {
        const int index (BG_GetViewModelWeaponIndex (&ps));

        if (index != 0)
        {
          const PlayerEquippedWeaponState* const e (
            BG_GetEquippedWeaponState (const_cast<playerState_s*> (&ps),
                                       static_cast<unsigned> (index)));

          if (e != nullptr && e->dualWielding)
          {
            right = left;
            right.side = trigger_side::right;
          }
        }
      }

      return true;
    }
  }
}
