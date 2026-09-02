#include "DualSense.hpp"

#include "../Types.hpp"

#include "Decode.hpp"
#include "OutputReport.hpp"
#include "PlayStation.hpp"

#include <algorithm>

namespace Controller
{
  namespace driver
  {
    namespace
    {
      constexpr uint8_t ds_report_usb {0x01};
      constexpr uint8_t ds_report_bt {0x31};
      constexpr size_t ds_size_usb {64};
      constexpr size_t ds_size_bt {78};
      constexpr size_t ds_common_usb {1};
      constexpr size_t ds_common_bt {2};

      bool
      resolve_frame (std::span<const std::byte> r, connection link, size_t& common) noexcept
      {
        if (link == connection::usb)
        {
          if (r.size () < ds_size_usb || rd_u8 (r, 0) != ds_report_usb)
            return false;

          common = ds_common_usb;
          return true;
        }

        if (link == connection::bluetooth)
        {
          if (r.size () < ds_size_bt || rd_u8 (r, 0) != ds_report_bt)
            return false;

          uint32_t crc (rd_le32 (r, ds_size_bt - 4));
          if (!verify_ps_crc32 (ps_input_crc_seed,
                                r.first (ds_size_bt - 4), crc))
            return false;

          common = ds_common_bt;
          return true;
        }

        return false;
      }
    }

    bool
    decode_dualsense (std::span<const std::byte> r,
                      connection link,
                      raw_sample& raw,
                      canonical_sample& canonical,
                      bool edge) noexcept
    {
      size_t b (0);
      if (!resolve_frame (r, link, b))
        return false;

      const uint8_t lx (rd_u8 (r, b + 0));
      const uint8_t ly (rd_u8 (r, b + 1));
      const uint8_t rx (rd_u8 (r, b + 2));
      const uint8_t ry (rd_u8 (r, b + 3));

      raw.sticks[static_cast<size_t> (stick::left)]  = {lx, ly};
      raw.sticks[static_cast<size_t> (stick::right)] = {rx, ry};

      auto& left (canonical.sticks[static_cast<size_t> (stick::left)]);
      auto& right (canonical.sticks[static_cast<size_t> (stick::right)]);
      left = {};
      right = {};
      left.raw = {lx, ly};
      left.normalized = normalize_ps_stick (lx, ly);
      right.raw = {rx, ry};
      right.normalized = normalize_ps_stick (rx, ry);

      const uint8_t z (rd_u8 (r, b + 4));
      const uint8_t rz (rd_u8 (r, b + 5));
      raw.triggers[static_cast<size_t> (trigger_side::left)]  = z;
      raw.triggers[static_cast<size_t> (trigger_side::right)] = rz;
      canonical.triggers[static_cast<size_t> (trigger_side::left)] =
        {z, static_cast<float> (z) / 255.0f};
      canonical.triggers[static_cast<size_t> (trigger_side::right)] =
        {rz, static_cast<float> (rz) / 255.0f};

      const uint8_t b0 (rd_u8 (r, b + 7));
      const uint8_t b1 (rd_u8 (r, b + 8));
      const uint8_t b2 (rd_u8 (r, b + 9));
      const uint8_t b3 (rd_u8 (r, b + 10));

      button_set btns;
      btns.set (button::face_west,  (b0 & 0x10u) != 0);
      btns.set (button::face_south, (b0 & 0x20u) != 0);
      btns.set (button::face_east,  (b0 & 0x40u) != 0);
      btns.set (button::face_north, (b0 & 0x80u) != 0);
      apply_hat (btns, static_cast<uint8_t> (b0 & 0x0Fu));

      btns.set (button::l1,    (b1 & 0x01u) != 0);
      btns.set (button::r1,    (b1 & 0x02u) != 0);
      btns.set (button::l2,    (b1 & 0x04u) != 0);
      btns.set (button::r2,    (b1 & 0x08u) != 0);
      btns.set (button::back,  (b1 & 0x10u) != 0);
      btns.set (button::start, (b1 & 0x20u) != 0);
      btns.set (button::l3,    (b1 & 0x40u) != 0);
      btns.set (button::r3,    (b1 & 0x80u) != 0);

      btns.set (button::guide,    (b2 & 0x01u) != 0);
      btns.set (button::touchpad, (b2 & 0x02u) != 0);
      btns.set (button::mute,     (b2 & 0x04u) != 0);

      if (edge)
      {
        btns.set (button::edge_fn_left,      (b2 & 0x10u) != 0);
        btns.set (button::edge_fn_right,     (b2 & 0x20u) != 0);
        btns.set (button::edge_paddle_left,  (b2 & 0x40u) != 0);
        btns.set (button::edge_paddle_right, (b2 & 0x80u) != 0);
      }

      canonical.buttons = btns;
      raw.buttons = static_cast<uint32_t> (b0) |
                    (static_cast<uint32_t> (b1) << 8) |
                    (static_cast<uint32_t> (b2) << 16) |
                    (static_cast<uint32_t> (b3) << 24);

      motion_sample motion;
      motion.gyro.angular_velocity =
        {static_cast<float> (rd_le16s (r, b + 15)),
         static_cast<float> (rd_le16s (r, b + 17)),
         static_cast<float> (rd_le16s (r, b + 19))};
      motion.accel.acceleration =
        {static_cast<float> (rd_le16s (r, b + 21)),
         static_cast<float> (rd_le16s (r, b + 23)),
         static_cast<float> (rd_le16s (r, b + 25))};
      motion.device_timestamp = rd_le32 (r, b + 27);
      raw.motion = motion;

      auto decode_point = [&r] (size_t off) -> touch_point
      {
        ps_touch_point p (decode_touch_point (r.subspan (off, 4)));
        return {p.active, p.id, p.x, p.y};
      };

      touchpad tp;
      tp.points[0] = decode_point (b + 32);
      tp.points[1] = decode_point (b + 36);
      canonical.touch = tp;
      raw.touch = tp;

      const uint8_t st0 (rd_u8 (r, b + 52));
      const uint8_t capacity (static_cast<uint8_t> (st0 & 0x0Fu));
      const uint8_t charging (static_cast<uint8_t> ((st0 >> 4) & 0x0Fu));

      battery_state bat;
      switch (charging)
      {
        case 0x0:
          bat.state = battery_state::status::discharging;
          bat.percent = static_cast<uint8_t> (std::min (capacity * 10 + 5, 100));
          break;
        case 0x1:
          bat.state = battery_state::status::charging;
          bat.percent = static_cast<uint8_t> (std::min (capacity * 10 + 5, 100));
          break;
        case 0x2:
          bat.state = battery_state::status::full;
          bat.percent = uint8_t {100};
          break;
        default:
          bat.state = battery_state::status::unknown;
          break;
      }
      canonical.battery = bat;
      raw.battery = bat;

      canonical.motion.reset ();

      capabilities caps (capability::gyroscope | capability::accelerometer |
                         capability::touchpad | capability::battery |
                         capability::rumble |
                         capability::adaptive_triggers | capability::light_bar |
                         capability::player_leds | capability::microphone_button);

      if (link == connection::usb || link == connection::bluetooth)
        caps.add (capability::haptics);
      if (edge)
        caps.add (capability::back_buttons);

      canonical.caps = caps;
      return true;
    }

    dualsense_driver::
    dualsense_driver (const context& ctx,
                      transport::hid_device& hid,
                      device_id device)
      : ctx_ (ctx), hid_ (hid), device_ (device), link_ (hid.link ())
    {
    }

    bool
    dualsense_driver::
    read_and_decode (raw_sample& raw,
                     canonical_sample& canonical,
                     bool edge) noexcept
    {
      std::array<std::byte, ds_size_bt> buf;
      bool decoded (false);

      for (size_t i (0); i != max_reports_per_poll; ++i)
      {
        std::optional<size_t> n (hid_.read (buf));

        if (!n || *n == 0)
          break;

        const std::span<const std::byte> r (buf.data (), *n);

        if (decode_dualsense (r, link_, raw, canonical, edge))
          decoded = true;
        else if (minimal_bluetooth_report (r, link_))
        {
          if (!minimal_reported_)
          {
            minimal_reported_ = true;

            ctx_.report (severity::info, facility::decode, errc::none, device_,
                         "DualSense is still sending minimal Bluetooth reports and "
                         "produces no input; the controller did not switch to its "
                         "extended report");
          }
        }
        else
          ctx_.report (severity::warning, facility::decode, errc::report_malformed,
                       device_, "DualSense report failed framing or CRC validation "
                                "and was dropped");
      }

      return decoded;
    }

    bool
    dualsense_driver::
    poll (raw_sample& raw, canonical_sample& canonical) noexcept
    {
      return read_and_decode (raw, canonical, false);
    }

    bool
    dualsense_driver::
    ensure_haptics () noexcept
    {
      if (haptics_failed_ ||
          (link_ != connection::usb && link_ != connection::bluetooth))
        return false;

      if (haptics_ == nullptr)
      {
        try
        {
          haptics_ = std::make_unique<haptic::stream> (ctx_, device_, link_, hid_);
        }
        catch (const std::exception& e)
        {
          haptics_failed_ = true;

          ctx_.report (severity::warning, facility::driver, errc::transport_failure,
                       device_, std::string ("controller haptics could not be "
                                             "started: ") + e.what ());
          return false;
        }
      }

      return haptics_->running ();
    }

    bool
    dualsense_driver::
    play_waveform (const rumble_request& r) noexcept
    {
      if (!ensure_haptics ())
        return false;

      haptics_->set_rumble (r.low_frequency, r.high_frequency);
      return true;
    }

    void
    dualsense_driver::
    submit (const output_request& request) noexcept
    {
      if (const auto* e = std::get_if<haptic::effect> (&request))
      {
        if (policy_.rumble && play_effect (*e))
          return;

        ctx_.report (severity::info, facility::driver, errc::output_rejected,
                     device_, "DualSense dropped a haptic effect: its actuators are "
                              "not being driven");
        return;
      }

      if (const auto* r = std::get_if<rumble_request> (&request))
      {
        if (policy_.haptics == haptic_mode::waveform)
        {
          if (emulating_)
          {
            emulating_ = false;
            submit_report (rumble_request {});
          }

          if (play_waveform (*r))
          {
            reported_fallback_ = false;
            return;
          }

          if (!reported_fallback_)
          {
            reported_fallback_ = true;

            ctx_.report (severity::warning, facility::driver, errc::output_rejected,
                         device_,
                         "controller haptics are enabled but the actuators are not "
                         "being driven, so rumble is going through the firmware's "
                         "motor emulation instead; controller_status says why");
          }
        }

        emulating_ = r->low_frequency > 0.0f || r->high_frequency > 0.0f;
      }

      submit_report (request);
    }

    bool
    dualsense_driver::
    play_effect (const haptic::effect& e) noexcept
    {
      return ensure_haptics () && haptics_->play (e);
    }

    void
    dualsense_driver::
    configure (const output_policy& p) noexcept
    {
      policy_ = p;

      const bool wanted (p.rumble && p.haptics == haptic_mode::waveform);

      if (wanted)
        ensure_haptics ();

      if (!wanted && haptics_ != nullptr)
      {
        haptics_.reset ();
        reported_fallback_ = false;
      }

      if (haptics_ != nullptr)
        haptics_->poll_diagnostics ();
    }

    void
    dualsense_driver::
    stop_haptic (uint32_t tag) noexcept
    {
      if (haptics_ != nullptr)
        haptics_->stop (tag);
    }

    std::string
    dualsense_driver::
    diagnostics () const
    {
      if (haptics_ == nullptr)
      {
        if (link_ != connection::usb && link_ != connection::bluetooth)
          return "haptics: unavailable, because this link's framing is unknown";

        if (!policy_.rumble)
          return "haptics: off, because rumble is off (gpad_rumble)";

        if (policy_.haptics != haptic_mode::waveform)
          return "haptics: off, playing rumble through the motor emulation "
                 "(gpad_haptics)";

        return "haptics: not started (no rumble has been asked for yet)";
      }

      return "haptics: " + haptics_->status ();
    }

    void
    dualsense_driver::
    submit_report (const output_request& request) noexcept
    {
      std::array<std::byte, ds_output_bt_size> buf;

      const std::optional<size_t> n (
        encode_dualsense_output (request, link_, bt_output_sequence_, buf));

      if (!n)
      {
        ctx_.report (severity::info, facility::driver, errc::output_rejected,
                     device_, "DualSense driver dropped an output request it cannot "
                              "encode for this link");
        return;
      }

      if (!hid_.write (std::span<const std::byte> (buf.data (), *n)))
        ctx_.report (severity::warning, facility::driver, errc::output_rejected,
                     device_, "DualSense output report write failed");
    }
  }
}
