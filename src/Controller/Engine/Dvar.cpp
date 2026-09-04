#include "Dvar.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace engine
  {
    dvars
    register_dvars (const context& ctx)
    {
      dvars d;

      d.enabled = Dvar_RegisterBool (
        "gpad_enabled", true, DVAR_ARCHIVE, "Game pad enabled");
      d.present = Dvar_RegisterBool (
        "gpad_present", false, DVAR_ROM, "A game pad is present");
      d.in_use = Dvar_RegisterBool (
        "gpad_in_use", false, DVAR_ROM, "A game pad is in use");
      d.rumble = Dvar_RegisterBool (
        "gpad_rumble", true, DVAR_ARCHIVE, "Enable game pad rumble");
      d.style = Dvar_RegisterBool (
        "gpad_style", false, DVAR_ARCHIVE,
        "Present PlayStation rather than Xbox glyphs");

      d.haptics = Dvar_RegisterBool (
        "gpad_haptics", true, DVAR_ARCHIVE,
        "Play rumble on the PlayStation controller's actuators, through the audio "
        "endpoint it presents, rather than through its motor emulation");
      d.adaptive_triggers = Dvar_RegisterBool (
        "gpad_adaptive_triggers", true, DVAR_ARCHIVE,
        "Resist the PlayStation controller's triggers according to the weapon held");

      d.light_bar = Dvar_RegisterBool (
        "gpad_light_bar", true, DVAR_ARCHIVE,
        "Light the PlayStation controller's bar in the menu accent colour");
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
      d.button_deadzone = Dvar_RegisterFloat (
        "gpad_button_deadzone", 0.13f, 0.0f, 1.0f, DVAR_ARCHIVE,
        "Game pad trigger button deadzone");
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

      d.ads_sprint_lock = Dvar_RegisterBool (
        "gpad_ads_sprint_lock", false, DVAR_ARCHIVE,
        "Ignore a stick click bound to sprint while aiming down sight. Off by "
        "default: clicking the stick while holding aim is how a great deal of "
        "movement is cancelled, and swallowing it breaks those cancels");

      d.invert_pitch = Dvar_RegisterBool (
        "input_invertPitch", false, DVAR_ARCHIVE, "Invert game pad pitch");
      d.view_sensitivity = Dvar_RegisterFloat (
        "input_viewSensitivity", 1.0f, 0.0001f, 5.0f, DVAR_ARCHIVE,
        "Game pad look sensitivity multiplier");
      d.aim_assist_enabled = Dvar_FindVar ("sv_allowAimAssist");
      d.turnrate_pitch = Dvar_RegisterFloat (
        "aim_turnrate_pitch", 90.0f, 0.0f, 1080.0f, DVAR_CHEAT,
        "Hip vertical turn rate (deg/s)");
      d.turnrate_pitch_ads = Dvar_RegisterFloat (
        "aim_turnrate_pitch_ads", 55.0f, 0.0f, 1080.0f, DVAR_CHEAT,
        "ADS vertical turn rate (deg/s)");
      d.turnrate_yaw = Dvar_RegisterFloat (
        "aim_turnrate_yaw", 260.0f, 0.0f, 1080.0f, DVAR_CHEAT,
        "Hip horizontal turn rate (deg/s)");
      d.turnrate_yaw_ads = Dvar_RegisterFloat (
        "aim_turnrate_yaw_ads", 90.0f, 0.0f, 1080.0f, DVAR_CHEAT,
        "ADS horizontal turn rate (deg/s)");
      d.accel_enabled = Dvar_RegisterBool (
        "aim_accel_turnrate_enabled", false, DVAR_ARCHIVE,
        "Ramp the stick's turn rate up while a direction is held, rather than "
        "turning at the full rate immediately");
      d.accel_rate = Dvar_RegisterFloat (
        "aim_accel_turnrate_lerp", 1200.0f, 0.0f, 4000.0f, DVAR_CHEAT,
        "Turn-rate acceleration (deg/s per second)");
      d.graph_enabled = Dvar_RegisterBool (
        "aim_input_graph_enabled", true, DVAR_CHEAT,
        "Use the aim graph to shape view input");
      d.graph_index = Dvar_RegisterInt (
        "aim_input_graph_index", 3, 0, 3, DVAR_CHEAT, "Which aim graph to use");
      d.scale_view_axis = Dvar_RegisterBool (
        "aim_scale_view_axis", true, DVAR_CHEAT,
        "Scale the view axes by the dominant axis");

      d.slowdown_enabled = Dvar_RegisterBool (
        "aim_slowdown_enabled", true, DVAR_ARCHIVE, "Enable aim slowdown");
      d.lockon_enabled = Dvar_RegisterBool (
        "aim_lockon_enabled", true, DVAR_ARCHIVE, "Enable lock-on aim assist");
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

      ctx.report (severity::info, facility::engine, errc::none,
                  "controller dvars registered");
      return d;
    }

    void
    publish_present (const dvars& d, bool present) noexcept
    {
      if (d.present != nullptr && d.present->current.enabled != present)
        Dvar_SetBool (d.present, present);
    }
  }
}
