#include "Key.hpp"

#include "../Types.hpp"

#include <cstring>
#include <algorithm>

#include "../Aim/Deadzone.hpp"
#include "../Mapping/Physical.hpp"

namespace Controller
{
  namespace engine
  {
    using mapping::engine_key;

    namespace
    {
      constexpr char loc_sel_cancel_command[] {"+actionslot 4"};
      constexpr char loc_sel_confirm_command[] {"+attack"};

      constexpr char toggle_scores_command[] {"togglescores"};

      constexpr char sprint_commands[][16] {"+breath_sprint", "+sprint"};

      constexpr unsigned ads_sprint_hold {600};

      struct button_key
      {
        button physical;
        engine_key key;
      };

      constexpr button_key button_keys[] {
        {button::face_west,  engine_key::button_x},
        {button::face_south, engine_key::button_a},
        {button::face_east,  engine_key::button_b},
        {button::face_north, engine_key::button_y},
        {button::l2,         engine_key::button_ltrig},
        {button::r2,         engine_key::button_rtrig},
        {button::l1,         engine_key::button_lshldr},
        {button::r1,         engine_key::button_rshldr},
        {button::start,      engine_key::button_start},
        {button::back,       engine_key::button_back},
        {button::l3,         engine_key::button_lstick},
        {button::r3,         engine_key::button_rstick},
        {button::dpad_up,    engine_key::dpad_up},
        {button::dpad_down,  engine_key::dpad_down},
        {button::dpad_left,  engine_key::dpad_left},
        {button::dpad_right, engine_key::dpad_right},
      };

      struct menu_key
      {
        engine_key controller;
        int keyboard;
      };

      constexpr menu_key menu_keys[] {
        {engine_key::button_a,     K_ENTER},
        {engine_key::button_start, K_ENTER},
        {engine_key::button_b,     K_ESCAPE},
        {engine_key::button_back,  K_ESCAPE},
        {engine_key::dpad_up,      K_UPARROW},
        {engine_key::apad_up,      K_UPARROW},
        {engine_key::rstick_up,    K_UPARROW},
        {engine_key::dpad_down,    K_DOWNARROW},
        {engine_key::apad_down,    K_DOWNARROW},
        {engine_key::rstick_down,  K_DOWNARROW},
        {engine_key::dpad_left,    K_LEFTARROW},
        {engine_key::apad_left,    K_LEFTARROW},
        {engine_key::rstick_left,  K_LEFTARROW},
        {engine_key::dpad_right,   K_RIGHTARROW},
        {engine_key::apad_right,   K_RIGHTARROW},
        {engine_key::rstick_right, K_RIGHTARROW},
      };

      bool
      is_dpad (engine_key k) noexcept
      {
        return k >= engine_key::dpad_up && k <= engine_key::dpad_right;
      }

      bool
      is_stick_key (engine_key k) noexcept
      {
        return (k >= engine_key::apad_up && k <= engine_key::apad_right) ||
               (k >= engine_key::rstick_up && k <= engine_key::rstick_right);
      }

      bool
      is_scroll_key (engine_key k) noexcept
      {
        return is_dpad (k) || is_stick_key (k);
      }

      bool
      repeats_while_held (button b) noexcept
      {
        switch (b)
        {
          case button::dpad_up:
          case button::dpad_down:
          case button::dpad_left:
          case button::dpad_right:
          case button::l2:
          case button::r2:
            return true;

          default:
            return false;
        }
      }

      const char*
      command_for (engine_key k) noexcept
      {
        return playerKeys[local_client].keys[static_cast<int> (k)].binding;
      }

      bool
      command_is (const char* command, const char* name) noexcept
      {
        return command != nullptr && std::strcmp (command, name) == 0;
      }

      bool
      is_sprint (const char* command) noexcept
      {
        for (const char* name: sprint_commands)
        {
          if (command_is (command, name))
            return true;
        }

        return false;
      }

      engine_key
      stick_key (stick which, bool horizontal, bool positive) noexcept
      {
        const mapping::stick_direction d (
          horizontal
          ? (positive ? mapping::stick_direction::right
                      : mapping::stick_direction::left)
          : (positive ? mapping::stick_direction::up
                      : mapping::stick_direction::down));

        return mapping::to_engine_key (mapping::apad_input {which, d});
      }

      bool
      sprint_button_up_wanted () noexcept
      {
        Game::cg_s* const cg (Game::CL_GetLocalClientGlobals (local_client));

        return cg != nullptr &&
               cg->predictedPlayerState.sprintState.sprintButtonUpRequired != 0;
      }

      bool
      ads_active () noexcept
      {
        const AimAssistGlobals& aa (aaGlobArray[local_client]);

        return aa.initialized && aa.adsLerp > 0.0f;
      }
    }

    key_dispatcher::
    key_dispatcher (const context& ctx, const dvars& d)
      : ctx_ (ctx), dvars_ (d)
    {
    }

    void
    key_dispatcher::
    set_in_use (bool v) noexcept
    {
      if (in_use_ == v)
        return;

      in_use_ = v;

      if (dvars_.in_use != nullptr)
        Dvar_SetBool (dvars_.in_use, v);

      ctx_.report (severity::info, facility::engine, errc::none,
                   v ? "input source: controller"
                     : "input source: keyboard and mouse");
    }

    unsigned
    key_dispatcher::
    release_delay () const noexcept
    {
      if (!read (dvars_.release_delay_enabled, true))
        return 0;

      const unsigned floor_ms (
        static_cast<unsigned> (std::max (0, read (dvars_.release_delay, 50))));

      const float scale (read (dvars_.release_delay_scale, 3.5f));

      if (!(scale > 0.0f))
        return floor_ms;

      const int ping (std::max (0, Game::clients[local_client].snap.ping));
      const unsigned scaled (
        static_cast<unsigned> (static_cast<float> (ping) * scale));

      return std::min (std::max (floor_ms, scaled), 2000u);
    }

    bool
    key_dispatcher::
    defers_release (engine_key k) const noexcept
    {
      if (release_delay () == 0 &&
          read (dvars_.release_grace, 75) <= 0)
        return false;

      const bool sprint (is_sprint (command_for (k)));

      if (sprint && sprint_button_up_wanted ())
        return false;

      if (!read (dvars_.release_delay_sprint_only, true))
        return true;

      return sprint;
    }

    void
    key_dispatcher::
    note_other_input () noexcept
    {
      set_in_use (false);
    }

    void
    key_dispatcher::
    dispatch (const canonical_sample& s) noexcept
    {
      const unsigned time (static_cast<unsigned> (Sys_Milliseconds ()));

      aim::deadzone_params dz {
        aim::magnitude {read (dvars_.stick_deadzone_min, 0.2f)},
        aim::magnitude {read (dvars_.stick_deadzone_max, 0.01f)},
        aim::magnitude {0.0f}};

      std::string why;

      if (!aim::validate (dz, why))
      {
        if (!reported_deadzone_)
        {
          reported_deadzone_ = true;
          ctx_.report (severity::warning, facility::mapping, errc::calibration_invalid,
                       "gpad_stick_deadzone_min/max rejected (" + why +
                       "); using defaults for the analog pad keys");
        }

        dz = aim::deadzone_params {aim::magnitude {0.2f},
                                   aim::magnitude {0.01f},
                                   aim::magnitude {0.0f}};
      }

      const stick_vector left (
        aim::apply (dz, s.sticks[static_cast<size_t> (stick::left)].calibrated));
      const stick_vector right (
        aim::apply (dz, s.sticks[static_cast<size_t> (stick::right)].calibrated));

      const float trigger_deadzone (read (dvars_.button_deadzone, 0.13f));

      const float lt (s.triggers[static_cast<size_t> (trigger_side::left)].normalized);
      const float rt (s.triggers[static_cast<size_t> (trigger_side::right)].normalized);

      const auto trigger_down = [trigger_deadzone] (float v) noexcept
      {
        return v > 0.0f && v >= trigger_deadzone;
      };

      button_set buttons (s.buttons);
      buttons.set (button::l2, trigger_down (lt));
      buttons.set (button::r2, trigger_down (rt));

      if (left.x != 0.0f || left.y != 0.0f ||
          right.x != 0.0f || right.y != 0.0f ||
          trigger_down (lt) || trigger_down (rt))
        set_in_use (true);

      const float axis[axis_count] {right.x, right.y, left.x, left.y};

      const float pressed (read (dvars_.stick_pressed, 0.4f));
      const float hysteresis (read (dvars_.stick_pressed_hysteresis, 0.1f));

      for (size_t i (0); i != axis_count; ++i)
      {
        for (size_t end (0); end != 2; ++end)
        {
          const bool positive (end == 1);

          was_deflected_[i][end] = deflected_[i][end];
          deflected_[i][end] = mapping::axis_deflected (axis[i],
                                                        positive,
                                                        was_deflected_[i][end],
                                                        pressed,
                                                        hysteresis);
        }
      }

      dispatch_apad (time);
      dispatch_buttons (buttons, time);

      buttons_ = buttons;
    }

    void
    key_dispatcher::
    dispatch_apad (unsigned time) noexcept
    {
      for (size_t i (0); i != axis_count; ++i)
      {
        const stick which (i < 2 ? stick::right : stick::left);
        const bool horizontal ((i % 2) == 0);

        const engine_key pos (stick_key (which, horizontal, true));
        const engine_key neg (stick_key (which, horizontal, false));

        const bool pos_now (deflected_[i][1]);
        const bool neg_now (deflected_[i][0]);
        const bool pos_was (was_deflected_[i][1]);
        const bool neg_was (was_deflected_[i][0]);

        if (pos_now)
          emit (pos, pos_was ? key_event::repeated : key_event::pressed, time);
        else if (neg_now)
          emit (neg, neg_was ? key_event::repeated : key_event::pressed, time);
        else if (pos_was)
          emit (pos, key_event::released, time);
        else if (neg_was)
          emit (neg, key_event::released, time);
      }
    }

    void
    key_dispatcher::
    dispatch_buttons (const button_set& current, unsigned time) noexcept
    {
      const unsigned delay (release_delay ());
      const unsigned grace (
        static_cast<unsigned> (std::max (0, read (dvars_.release_grace, 75))));

      for (const button_key& m: button_keys)
      {
        const size_t index (static_cast<size_t> (m.physical));

        const bool now (current.down (m.physical));
        const bool was (buttons_.down (m.physical));

        if (now && !was)
        {
          deferred_.set (m.physical, false);
          pressed_at_[index] = time;

          emit_button (m.key, key_event::pressed, time);
        }
        else if (now && repeats_while_held (m.physical))
          emit_button (m.key, key_event::repeated, time);
        else if (!now && was)
        {
          if (defers_release (m.key))
          {
            released_at_[index] = time;
            deferred_.set (m.physical, true);
            emit_button (m.key, key_event::repeated, time);
            continue;
          }

          emit_button (m.key, key_event::released, time);
        }
        else if (!now && deferred_.down (m.physical))
        {
          const bool expired (time - pressed_at_[index] >= delay &&
                              time - released_at_[index] >= grace);

          const bool lowering (is_sprint (command_for (m.key)) &&
                               ads_active () &&
                               time - released_at_[index] < ads_sprint_hold);

          if (!defers_release (m.key) || (expired && !lowering))
          {
            deferred_.set (m.physical, false);
            emit_button (m.key, key_event::released, time);
          }
          else
            emit_button (m.key, key_event::repeated, time);
        }
      }
    }

    void
    key_dispatcher::
    emit_button (engine_key k, key_event e, unsigned time) noexcept
    {
      set_in_use (true);

      if (Key_IsCatcherActive (local_client, KEYCATCH_UI))
        reset_scroll (k, e == key_event::pressed, time);

      emit (k, e, time);
    }

    void
    key_dispatcher::
    emit (engine_key k, key_event e, unsigned time) noexcept
    {
      const int key (static_cast<int> (k));
      const bool down (e != key_event::released);

      PlayerKeyState& ks (playerKeys[local_client]);
      ks.keys[key].down = down ? 1 : 0;

      if (down)
      {
        if (++ks.keys[key].repeats == 1)
          ++ks.anyKeyDown;
      }
      else if (ks.keys[key].repeats > 0)
      {
        ks.keys[key].repeats = 0;

        if (--ks.anyKeyDown < 0)
          ks.anyKeyDown = 0;
      }

      if (down && ignore_repeat (k, ks.keys[key].repeats, time))
        return;

      if (down && Key_IsCatcherActive (local_client, KEYCATCH_LOCATION_SELECTION))
      {
        const char* bound (command_for (k));

        if (k == engine_key::button_b ||
            command_is (bound, loc_sel_cancel_command))
          ks.locSelInputState = LOC_SEL_INPUT_CANCEL;
        else if (k == engine_key::button_a ||
                 command_is (bound, loc_sel_confirm_command))
          ks.locSelInputState = LOC_SEL_INPUT_CONFIRM;

        return;
      }

      if (UI_GetActiveMenu (local_client) == UIMENU_SCOREBOARD &&
          e == key_event::pressed &&
          scoreboard_key_event (k))
        return;

      ks.locSelInputState = LOC_SEL_INPUT_NONE;

      const char* const binding (ks.keys[key].binding);

      char cmd[1024];

      if (down)
      {
        if (Key_IsCatcherActive (local_client, KEYCATCH_UI))
        {
          menu_key_event (k, true);
          return;
        }

        if (binding != nullptr)
        {
          if (binding[0] == '+')
          {
            sprintf_s (cmd, "%s %i %i\n", binding, key, time);
            Cbuf_AddText (local_client, cmd);
          }
          else
            Cbuf_InsertText (local_client, binding);
        }
      }
      else
      {
        if (binding != nullptr && binding[0] == '+')
        {
          sprintf_s (cmd, "-%s %i %i\n", &binding[1], key, time);
          Cbuf_AddText (local_client, cmd);
        }

        if (Key_IsCatcherActive (local_client, KEYCATCH_UI))
          menu_key_event (k, false);
      }
    }

    bool
    key_dispatcher::
    ignore_repeat (engine_key k, int repeats, unsigned time) noexcept
    {
      if (Key_IsCatcherActive (local_client, KEYCATCH_UI) && is_scroll_key (k))
      {
        const int first (read (dvars_.menu_scroll_delay_first, 420));
        const int rest (read (dvars_.menu_scroll_delay_rest, 210));
        const int least (read (dvars_.menu_scroll_delay_min, 50));
        const int accel (read (dvars_.menu_scroll_accel_time, 1500));

        if (repeats == 1)
        {
          next_scroll_ = time + static_cast<unsigned> (first);
          return false;
        }

        if (time > next_scroll_)
        {
          int delay (rest);

          if (is_dpad (k) && accel > 0 && rest > least)
          {
            const int elapsed (static_cast<int> (time - scroll_hold_start_));
            const int t (std::min (elapsed, accel));

            delay = rest - (rest - least) * t / accel;
          }

          next_scroll_ = time + static_cast<unsigned> (delay);
          return false;
        }
      }

      return repeats > 1;
    }

    void
    key_dispatcher::
    reset_scroll (engine_key k, bool down, unsigned time) noexcept
    {
      if (!down)
      {
        scroll_hold_key_.reset ();
        return;
      }

      if (!is_scroll_key (k))
        return;

      if (is_dpad (k) && scroll_hold_key_ != k)
      {
        scroll_hold_start_ = time;
        scroll_hold_key_ = k;
      }

      next_scroll_ = time +
        static_cast<unsigned> (read (dvars_.menu_scroll_delay_first, 420));
    }

    void
    key_dispatcher::
    menu_key_event (engine_key k, bool down) noexcept
    {
      if (*g_waitingForKey != 0)
      {
        UI_KeyEvent (local_client, static_cast<int> (k), down ? 1 : 0);
        return;
      }

      for (const menu_key& m: menu_keys)
      {
        if (m.controller == k)
        {
          UI_KeyEvent (local_client, m.keyboard, down ? 1 : 0);
          return;
        }
      }
    }

    bool
    key_dispatcher::
    scoreboard_key_event (engine_key k) noexcept
    {
      if (command_is (command_for (k), toggle_scores_command))
      {
        Cbuf_AddText (local_client, "togglescores\n");
        return true;
      }

      if (k == engine_key::dpad_up)
      {
        CG_ScrollScoreboardUp (Game::cgArray);
        return true;
      }

      if (k == engine_key::dpad_down)
      {
        CG_ScrollScoreboardDown (Game::cgArray);
        return true;
      }

      return false;
    }

    void
    key_dispatcher::
    release_all () noexcept
    {
      const unsigned time (static_cast<unsigned> (Sys_Milliseconds ()));
      const PlayerKeyState& ks (playerKeys[local_client]);

      for (const engine_key k: mapping::keys ())
      {
        if (ks.keys[static_cast<int> (k)].down != 0)
          emit (k, key_event::released, time);
      }

      buttons_ = button_set ();
      deferred_ = button_set ();
      pressed_at_ = {};
      released_at_ = {};
      deflected_ = {};
      was_deflected_ = {};
      scroll_hold_key_.reset ();
    }
  }
}
