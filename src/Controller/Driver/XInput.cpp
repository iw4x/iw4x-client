#include "XInput.hpp"

#include "../Types.hpp"

#include <cmath>
#include <variant>
#include <algorithm>

namespace Controller
{
  namespace driver
  {
    namespace
    {
      constexpr uint16_t xinput_gamepad_guide {0x0400};

      constexpr float thumb_full_scale {32767.0f};

      stick_vector
      normalize_stick (int16_t rx, int16_t ry) noexcept
      {
        float x (std::clamp (static_cast<float> (rx) / thumb_full_scale,
                             -1.0f, 1.0f));
        float y (std::clamp (static_cast<float> (ry) / thumb_full_scale,
                             -1.0f, 1.0f));

        float m (std::sqrt (x * x + y * y));
        if (m > 1.0f)
        {
          x /= m;
          y /= m;
        }

        return {x, y};
      }

      trigger_sample
      decode_trigger (uint8_t raw) noexcept
      {
        return {raw, static_cast<float> (raw) / 255.0f};
      }

      button_set
      decode_buttons (uint16_t w, bool have_guide) noexcept
      {
        button_set s;

        s.set (button::face_south, (w & XINPUT_GAMEPAD_A) != 0);
        s.set (button::face_east,  (w & XINPUT_GAMEPAD_B) != 0);
        s.set (button::face_west,  (w & XINPUT_GAMEPAD_X) != 0);
        s.set (button::face_north, (w & XINPUT_GAMEPAD_Y) != 0);

        s.set (button::dpad_up,    (w & XINPUT_GAMEPAD_DPAD_UP) != 0);
        s.set (button::dpad_down,  (w & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
        s.set (button::dpad_left,  (w & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
        s.set (button::dpad_right, (w & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);

        s.set (button::l1, (w & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
        s.set (button::r1, (w & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
        s.set (button::l3, (w & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
        s.set (button::r3, (w & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);

        s.set (button::start, (w & XINPUT_GAMEPAD_START) != 0);
        s.set (button::back,  (w & XINPUT_GAMEPAD_BACK) != 0);

        if (have_guide)
          s.set (button::guide, (w & xinput_gamepad_guide) != 0);

        return s;
      }

      WORD
      scale_motor (float v) noexcept
      {
        return static_cast<WORD> (std::clamp (v, 0.0f, 1.0f) * 65535.0f);
      }
    }

    void
    decode_xinput (const XINPUT_GAMEPAD& g,
                   bool has_guide,
                   raw_sample& raw,
                   canonical_sample& canonical) noexcept
    {
      raw.sticks[static_cast<size_t> (stick::left)]  = {g.sThumbLX, g.sThumbLY};
      raw.sticks[static_cast<size_t> (stick::right)] = {g.sThumbRX, g.sThumbRY};
      raw.triggers[static_cast<size_t> (trigger_side::left)]  = g.bLeftTrigger;
      raw.triggers[static_cast<size_t> (trigger_side::right)] = g.bRightTrigger;
      raw.buttons = g.wButtons;

      button_set buttons (decode_buttons (g.wButtons, has_guide));

      buttons.set (button::l2, g.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
      buttons.set (button::r2, g.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

      canonical.buttons = buttons;

      auto& left (canonical.sticks[static_cast<size_t> (stick::left)]);
      auto& right (canonical.sticks[static_cast<size_t> (stick::right)]);
      left = {};
      right = {};
      left.raw = {g.sThumbLX, g.sThumbLY};
      left.normalized = normalize_stick (g.sThumbLX, g.sThumbLY);
      right.raw = {g.sThumbRX, g.sThumbRY};
      right.normalized = normalize_stick (g.sThumbRX, g.sThumbRY);

      canonical.triggers[static_cast<size_t> (trigger_side::left)] =
        decode_trigger (g.bLeftTrigger);
      canonical.triggers[static_cast<size_t> (trigger_side::right)] =
        decode_trigger (g.bRightTrigger);

      canonical.touch.reset ();
      canonical.motion.reset ();
      canonical.battery.reset ();

      canonical.caps = capability::rumble;
    }

    xinput_driver::
    xinput_driver (const context& ctx,
                   const transport::xinput_module& module,
                   device_id device,
                   user_index index)
      : ctx_ (ctx), module_ (module), device_ (device), index_ (index)
    {
    }

    bool
    xinput_driver::
    poll (raw_sample& raw, canonical_sample& canonical) noexcept
    {
      XINPUT_STATE st {};

      if (module_.get_state (index_.value (), st) != ERROR_SUCCESS)
      {
        have_packet_ = false;
        return false;
      }

      if (have_packet_ && st.dwPacketNumber == last_packet_)
        return false;

      have_packet_ = true;
      last_packet_ = st.dwPacketNumber;

      decode_xinput (st.Gamepad, module_.has_guide_button (), raw, canonical);
      return true;
    }

    void
    xinput_driver::
    submit (const output_request& request) noexcept
    {
      if (const auto* r = std::get_if<rumble_request> (&request))
      {
        XINPUT_VIBRATION v {};
        v.wLeftMotorSpeed = scale_motor (r->low_frequency);
        v.wRightMotorSpeed = scale_motor (r->high_frequency);

        if (module_.set_state (index_.value (), v) != ERROR_SUCCESS)
          ctx_.report (severity::warning, facility::driver, errc::output_rejected,
                       device_, "XInput rumble output failed");

        return;
      }

      if (unsupported_reported_)
        return;

      unsupported_reported_ = true;

      ctx_.report (severity::info, facility::driver, errc::output_rejected,
                   device_, "XInput driver ignores a non-rumble output request");
    }

    std::string
    xinput_driver::
    diagnostics () const
    {
      return "haptics: unavailable, because an XInput controller exposes nothing "
             "but its two rumble motors";
    }
  }
}
