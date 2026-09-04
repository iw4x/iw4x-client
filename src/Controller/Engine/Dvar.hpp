#pragma once

#include "../Types.hpp"

#include "Engine.hpp"
#include "../Context.hpp"

namespace Controller
{
  namespace engine
  {
    struct dvars
    {
      dvar_t* enabled {};
      dvar_t* present {};
      dvar_t* in_use {};
      dvar_t* rumble {};
      dvar_t* style {};

      dvar_t* haptics {};
      dvar_t* adaptive_triggers {};

      dvar_t* light_bar {};
      dvar_t* light_bar_r {};
      dvar_t* light_bar_g {};
      dvar_t* light_bar_b {};

      dvar_t* stick_deadzone_min {};
      dvar_t* stick_deadzone_max {};
      dvar_t* button_deadzone {};
      dvar_t* stick_pressed {};
      dvar_t* stick_pressed_hysteresis {};

      dvar_t* buttons_config {};
      dvar_t* sticks_config {};

      dvar_t* menu_scroll_delay_first {};
      dvar_t* menu_scroll_delay_rest {};
      dvar_t* menu_scroll_delay_min {};
      dvar_t* menu_scroll_accel_time {};

      dvar_t* ads_sprint_lock {};

      dvar_t* invert_pitch {};
      dvar_t* view_sensitivity {};
      dvar_t* aim_assist_enabled {};
      dvar_t* turnrate_pitch {};
      dvar_t* turnrate_pitch_ads {};
      dvar_t* turnrate_yaw {};
      dvar_t* turnrate_yaw_ads {};
      dvar_t* accel_enabled {};
      dvar_t* accel_rate {};
      dvar_t* graph_enabled {};
      dvar_t* graph_index {};
      dvar_t* scale_view_axis {};

      dvar_t* slowdown_enabled {};
      dvar_t* lockon_enabled {};
      dvar_t* lockon_deflection {};
      dvar_t* lockon_strength {};
      dvar_t* lockon_pitch_strength {};
      dvar_t* aim_assist_range_scale {};
    };

    dvars
    register_dvars (const context&);

    void
    publish_present (const dvars&, bool present) noexcept;

    int
    read_number (dvar_t*, int fallback) noexcept;

    void
    write_number (dvar_t*, int value) noexcept;

    inline bool
    read (dvar_t* d, bool fallback) noexcept
    {
      return d != nullptr ? d->current.enabled : fallback;
    }

    inline float
    read (dvar_t* d, float fallback) noexcept
    {
      return d != nullptr ? d->current.value : fallback;
    }

    inline int
    read (dvar_t* d, int fallback) noexcept
    {
      return d != nullptr ? d->current.integer : fallback;
    }

    inline const char*
    read (dvar_t* d, const char* fallback) noexcept
    {
      return d != nullptr && d->current.string != nullptr
        ? d->current.string
        : fallback;
    }
  }
}
