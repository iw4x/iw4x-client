#include "Dvar.hpp"

#include "../Types.hpp"

#include <initializer_list>

namespace Controller
{
  namespace engine
  {
    dvars&
    registered_dvars () noexcept
    {
      static dvars d;
      return d;
    }

    namespace
    {
      void
      player_settings (std::initializer_list<dvar_t*> settings) noexcept
      {
        for (dvar_t* v: settings)
        {
          if (v != nullptr)
            v->flags &= ~DVAR_CHEAT;
        }
      }
    }

    void
    register_dvars ()
    {
      dvars& d (registered_dvars ());

      d.enabled = Dvar_RegisterBool (
        "gpad_enabled", true, DVAR_ARCHIVE, "Game pad enabled");
      d.present = Dvar_RegisterBool (
        "gpad_present", false, DVAR_ROM, "A game pad is present");
      d.in_use = Dvar_RegisterBool (
        "gpad_in_use", false, DVAR_ROM, "A game pad is in use");
      d.rumble = Dvar_RegisterBool (
        "gpad_rumble", true, DVAR_ARCHIVE, "Enable game pad rumble");
      d.style = Dvar_RegisterInt (
        "gpad_style", 0, 0, 2, DVAR_ARCHIVE,
        "Which button glyphs to present: 0 follows the controller that is "
        "connected, 1 is PlayStation and 2 is Xbox");

      d.haptics = Dvar_RegisterBool (
        "gpad_haptics", true, DVAR_ARCHIVE,
        "Play rumble on the PlayStation controller's actuators, through the audio "
        "endpoint it presents, rather than through its motor emulation");
      d.haptic_intensity = Dvar_RegisterFloat (
        "gpad_haptic_intensity", 1.0f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Scale applied to every haptic effect played on the controller's "
        "actuators");
      d.rumble_scale_low = Dvar_RegisterFloat (
        "gpad_rumble_scale_low", 1.0f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Scale applied to the low-frequency (heavy) rumble motor");
      d.rumble_scale_high = Dvar_RegisterFloat (
        "gpad_rumble_scale_high", 1.0f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Scale applied to the high-frequency (light) rumble motor");

      d.adaptive_triggers = Dvar_RegisterBool (
        "gpad_adaptive_triggers", false, DVAR_ARCHIVE,
        "Resist the PlayStation controller's triggers according to the weapon held");
      d.adaptive_trigger_strength = Dvar_RegisterFloat (
        "gpad_adaptive_trigger_strength", 1.0f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Scale applied to every adaptive trigger resistance. Zero leaves the "
        "triggers free without turning the effects off");
      d.adaptive_trigger_light = Dvar_RegisterInt (
        "gpad_adaptive_trigger_light", 7, 0, 8, DVAR_ARCHIVE,
        "Resistance of the light effects: the submachine gun ramp's start, the "
        "pistol's break and the grenade's break");
      d.adaptive_trigger_heavy = Dvar_RegisterInt (
        "gpad_adaptive_trigger_heavy", 8, 0, 8, DVAR_ARCHIVE,
        "Resistance of the heavy effects: the rifle and machine gun's constant "
        "load, the submachine gun ramp's end, and the shotgun, sniper and "
        "launcher's break");
      d.adaptive_trigger_light_start = Dvar_RegisterInt (
        "gpad_adaptive_trigger_light_start", 2, 0, 9, DVAR_ARCHIVE,
        "Zone at which a light break begins, out of ten along the trigger's travel");
      d.adaptive_trigger_light_end = Dvar_RegisterInt (
        "gpad_adaptive_trigger_light_end", 5, 0, 9, DVAR_ARCHIVE,
        "Zone at which a light break ends, out of ten along the trigger's travel");
      d.adaptive_trigger_heavy_start = Dvar_RegisterInt (
        "gpad_adaptive_trigger_heavy_start", 3, 0, 9, DVAR_ARCHIVE,
        "Zone at which a heavy break begins, out of ten along the trigger's travel");
      d.adaptive_trigger_heavy_end = Dvar_RegisterInt (
        "gpad_adaptive_trigger_heavy_end", 7, 0, 9, DVAR_ARCHIVE,
        "Zone at which a heavy break ends, out of ten along the trigger's travel");
      d.adaptive_trigger_ads = Dvar_RegisterInt (
        "gpad_adaptive_trigger_ads", 0, 0, 8, DVAR_ARCHIVE,
        "Constant resistance held on the trigger bound to aiming down the "
        "sights. Zero leaves that trigger free");

      d.output_interval = Dvar_RegisterInt (
        "gpad_output_interval", 4, 0, 50, DVAR_ARCHIVE,
        "Shortest gap between rumble output reports, in milliseconds. The game "
        "changes the rumble level every frame, which is faster than the "
        "controller's output endpoint can carry, so the reports queue up in the "
        "driver and the rumble falls further behind the longer it runs. Zero "
        "sends every change");

      d.light_bar = Dvar_RegisterBool (
        "gpad_light_bar", true, DVAR_ARCHIVE,
        "Light the PlayStation controller's bar in the menu accent colour");
      d.light_bar_brightness = Dvar_RegisterFloat (
        "gpad_light_bar_brightness", 1.0f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Scale applied to the light bar colour, dimming the bar without "
        "changing its hue");
      d.light_bar_r = Dvar_RegisterInt (
        "gpad_light_bar_r", 196, 0, 255, DVAR_ARCHIVE, "Light bar red");
      d.light_bar_g = Dvar_RegisterInt (
        "gpad_light_bar_g", 151, 0, 255, DVAR_ARCHIVE, "Light bar green");
      d.light_bar_b = Dvar_RegisterInt (
        "gpad_light_bar_b", 54, 0, 255, DVAR_ARCHIVE, "Light bar blue");

      d.stick_deadzone_min = Dvar_RegisterFloat (
        "gpad_stick_deadzone_min", 0.2f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Game pad inner stick deadzone");
      d.stick_deadzone_max = Dvar_RegisterFloat (
        "gpad_stick_deadzone_max", 0.01f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Game pad outer stick deadzone");
      d.stick_anti_deadzone = Dvar_RegisterFloat (
        "gpad_stick_anti_deadzone", 0.0f, 0.0f, 0.9f, DVAR_ARCHIVE,
        "Deflection the stick jumps to the moment it leaves the inner "
        "deadzone, to cancel a weapon or engine dead band");
      d.button_deadzone = Dvar_RegisterFloat (
        "gpad_button_deadzone", 0.13f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Game pad trigger button deadzone");
      d.button_deadzone_hysteresis = Dvar_RegisterFloat (
        "gpad_button_deadzone_hysteresis", 0.05f, 0.0f, 0.5f, DVAR_ARCHIVE,
        "How far below the press point a trigger must fall again before it "
        "counts as released, so a trigger resting on the threshold does not "
        "chatter");
      d.stick_pressed = Dvar_RegisterFloat (
        "gpad_stick_pressed", 0.4f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Deflection at which a stick counts as pressed");
      d.stick_pressed_hysteresis = Dvar_RegisterFloat (
        "gpad_stick_pressed_hysteresis", 0.1f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "No-change band around the stick pressed threshold");

      d.buttons_config = Dvar_RegisterString (
        "gpad_buttonConfig", "buttons_default", DVAR_ARCHIVE,
        "Game pad button configuration");
      d.sticks_config = Dvar_RegisterString (
        "gpad_sticksConfig", "thumbstick_default", DVAR_ARCHIVE,
        "Game pad stick configuration");

      d.menu_scroll_delay_first = Dvar_RegisterInt (
        "gpad_menu_scroll_delay_first", 420, 0, 1000, DVAR_ARCHIVE,
        "Menu scroll key-repeat delay, for the first repeat, in milliseconds");
      d.menu_scroll_delay_rest = Dvar_RegisterInt (
        "gpad_menu_scroll_delay_rest", 210, 0, 1000, DVAR_ARCHIVE,
        "Menu scroll key-repeat delay, for later repeats, in milliseconds");
      d.menu_scroll_delay_min = Dvar_RegisterInt (
        "gpad_menu_scroll_delay_min", 50, 0, 1000, DVAR_ARCHIVE,
        "Menu scroll key-repeat delay at maximum acceleration, in milliseconds");
      d.menu_scroll_accel_time = Dvar_RegisterInt (
        "gpad_menu_scroll_accel_time", 1500, 0, 5000, DVAR_ARCHIVE,
        "How long a held direction takes to reach the minimum scroll delay, in "
        "milliseconds");

      d.use_hold_time = Dvar_RegisterInt (
        "gpad_use_hold_time", 250, 0, 10000, DVAR_ARCHIVE,
        "How long the use/reload button must be held on a game pad before it "
        "uses rather than reloads");

      d.release_delay_enabled = Dvar_RegisterBool (
        "gpad_button_release_delay_enabled", true, DVAR_ARCHIVE,
        "Hold a tapped button down long enough for the server to see it, so a "
        "quick tap is not swallowed on a laggy connection");
      d.release_delay = Dvar_RegisterInt (
        "gpad_button_release_delay", 50, 0, 2000, DVAR_ARCHIVE,
        "Shortest press the server is shown, in milliseconds. Scales up with "
        "ping through gpad_button_release_delay_scale");
      d.release_delay_scale = Dvar_RegisterFloat (
        "gpad_button_release_delay_scale", 3.5f, 0.0f, 10.0f, DVAR_ARCHIVE,
        "Ping multiplier for the release delay (delay = ping * scale). Zero "
        "pins the delay at gpad_button_release_delay");
      d.release_delay_sprint_only = Dvar_RegisterBool (
        "gpad_button_release_delay_sprint_only", true, DVAR_ARCHIVE,
        "Only hold the sprint button, so firing and menu navigation stay "
        "immediate");
      d.release_grace = Dvar_RegisterInt (
        "gpad_button_release_grace", 75, 0, 500, DVAR_ARCHIVE,
        "How long after the physical release the button is still reported "
        "held, in milliseconds");

      d.invert_pitch = Dvar_RegisterBool (
        "input_invertPitch", false, DVAR_ARCHIVE, "Invert game pad pitch");
      d.view_sensitivity = Dvar_RegisterFloat (
        "input_viewSensitivity", 1.0f, 0.0001f, 5.0f, DVAR_ARCHIVE,
        "Game pad look sensitivity multiplier");
      d.aim_assist_enabled = Dvar_FindVar ("sv_allowAimAssist");
      d.turnrate_pitch = Dvar_RegisterFloat (
        "aim_turnrate_pitch", 90.0f, 0.0f, 1080.0f, DVAR_ARCHIVE,
        "Hip vertical turn rate (deg/s)");
      d.turnrate_pitch_ads = Dvar_RegisterFloat (
        "aim_turnrate_pitch_ads", 55.0f, 0.0f, 1080.0f, DVAR_ARCHIVE,
        "ADS vertical turn rate (deg/s)");
      d.turnrate_yaw = Dvar_RegisterFloat (
        "aim_turnrate_yaw", 260.0f, 0.0f, 1080.0f, DVAR_ARCHIVE,
        "Hip horizontal turn rate (deg/s)");
      d.turnrate_yaw_ads = Dvar_RegisterFloat (
        "aim_turnrate_yaw_ads", 90.0f, 0.0f, 1080.0f, DVAR_ARCHIVE,
        "ADS horizontal turn rate (deg/s)");
      d.accel_enabled = Dvar_RegisterBool (
        "aim_accel_turnrate_enabled", true, DVAR_ARCHIVE,
        "Ramp the stick's turn rate up while a direction is held, rather than "
        "turning at the full rate immediately");
      d.accel_rate = Dvar_RegisterFloat (
        "aim_accel_turnrate_lerp", 1200.0f, 0.0f, 4000.0f, DVAR_ARCHIVE,
        "Turn-rate acceleration (deg/s per second)");
      d.graph_enabled = Dvar_RegisterBool (
        "aim_input_graph_enabled", true, DVAR_ARCHIVE,
        "Use the aim graph to shape view input");
      d.graph_index = Dvar_RegisterInt (
        "aim_input_graph_index", 3, 0, 3, DVAR_ARCHIVE, "Which aim graph to use");
      d.scale_view_axis = Dvar_RegisterBool (
        "aim_scale_view_axis", true, DVAR_ARCHIVE,
        "Scale the view axes by the dominant axis");

      d.slowdown_enabled = Dvar_RegisterBool (
        "aim_slowdown_enabled", true, DVAR_ARCHIVE, "Enable aim slowdown");
      d.gpad_slowdown_enabled = Dvar_RegisterBool (
        "gpad_slowdown_enabled", true, DVAR_ARCHIVE,
        "Game pad slowdown aim assist enabled");
      d.slowdown_pitch_scale = Dvar_FindVar ("aim_slowdown_pitch_scale");
      d.slowdown_pitch_scale_ads = Dvar_FindVar ("aim_slowdown_pitch_scale_ads");
      d.slowdown_yaw_scale = Dvar_FindVar ("aim_slowdown_yaw_scale");
      d.slowdown_yaw_scale_ads = Dvar_FindVar ("aim_slowdown_yaw_scale_ads");
      d.lockon_enabled = Dvar_RegisterBool (
        "aim_lockon_enabled", true, DVAR_ARCHIVE, "Enable lock-on aim assist");
      d.gpad_lockon_enabled = Dvar_RegisterBool (
        "gpad_lockon_enabled", true, DVAR_ARCHIVE,
        "Game pad lockon aim assist enabled");
      d.lockon_deflection = Dvar_RegisterFloat (
        "aim_lockon_deflection", 0.05f, 0.0f, 1.0f, DVAR_CHEAT,
        "Stick deflection at which lock-on activates");
      d.lockon_strength = Dvar_RegisterFloat (
        "aim_lockon_strength", 0.6f, 0.0f, 1.0f, DVAR_CHEAT,
        "Lock-on yaw assistance");
      d.lockon_pitch_strength = Dvar_RegisterFloat (
        "aim_lockon_pitch_strength", 0.6f, 0.0f, 1.0f, DVAR_CHEAT,
        "Lock-on pitch assistance");
      d.aim_assist_range_scale = Dvar_RegisterFloat (
        "aim_aimAssistRangeScale", 1.0f, 0.0f, 10.0f, DVAR_CHEAT,
        "Aim-assist target range scale");

      player_settings ({d.enabled,
                        d.rumble,
                        d.style,
                        d.haptics,
                        d.haptic_intensity,
                        d.rumble_scale_low,
                        d.rumble_scale_high,
                        d.adaptive_triggers,
                        d.output_interval,
                        d.adaptive_trigger_strength,
                        d.adaptive_trigger_light,
                        d.adaptive_trigger_heavy,
                        d.adaptive_trigger_light_start,
                        d.adaptive_trigger_light_end,
                        d.adaptive_trigger_heavy_start,
                        d.adaptive_trigger_heavy_end,
                        d.adaptive_trigger_ads,
                        d.light_bar,
                        d.light_bar_brightness,
                        d.light_bar_r,
                        d.light_bar_g,
                        d.light_bar_b,
                        d.stick_deadzone_min,
                        d.stick_deadzone_max,
                        d.stick_anti_deadzone,
                        d.button_deadzone,
                        d.button_deadzone_hysteresis,
                        d.stick_pressed,
                        d.stick_pressed_hysteresis,
                        d.buttons_config,
                        d.sticks_config,
                        d.menu_scroll_delay_first,
                        d.menu_scroll_delay_rest,
                        d.menu_scroll_delay_min,
                        d.menu_scroll_accel_time,
                        d.use_hold_time,
                        d.release_delay_enabled,
                        d.release_delay,
                        d.release_delay_scale,
                        d.release_delay_sprint_only,
                        d.release_grace,
                        d.invert_pitch,
                        d.view_sensitivity,
                        d.turnrate_pitch,
                        d.turnrate_pitch_ads,
                        d.turnrate_yaw,
                        d.turnrate_yaw_ads,
                        d.accel_enabled,
                        d.accel_rate,
                        d.graph_enabled,
                        d.graph_index,
                        d.scale_view_axis,
                        d.slowdown_enabled,
                        d.gpad_slowdown_enabled,
                        d.lockon_enabled,
                        d.gpad_lockon_enabled});
    }

    void
    publish_present (const dvars& d, bool present) noexcept
    {
      if (d.present != nullptr && d.present->current.enabled != present)
        Dvar_SetBool (d.present, present);
    }
  }
}
