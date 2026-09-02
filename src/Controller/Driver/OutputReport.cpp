#include "OutputReport.hpp"

#include "../Types.hpp"

#include <cassert>
#include <variant>
#include <algorithm>

#include "Decode.hpp"

namespace Controller
{
  namespace driver
  {
    namespace
    {
      uint8_t
      to_byte (float v) noexcept
      {
        return static_cast<uint8_t> (std::clamp (v, 0.0f, 1.0f) * 255.0f + 0.5f);
      }

      void
      put (std::span<std::byte> b, size_t i, uint8_t v) noexcept
      {
        assert (i < b.size ());
        b[i] = static_cast<std::byte> (v);
      }

      void
      seal_bt_crc (std::span<std::byte> b) noexcept
      {
        assert (b.size () >= 4);

        const std::byte seed {static_cast<std::byte> (ps_output_crc_seed)};

        uint32_t crc (crc32_le (0xFFFFFFFFu, std::span<const std::byte> (&seed, 1)));
        crc = ~crc32_le (crc, b.subspan (0, b.size () - 4));

        const size_t o (b.size () - 4);
        put (b, o + 0, static_cast<uint8_t> (crc));
        put (b, o + 1, static_cast<uint8_t> (crc >> 8));
        put (b, o + 2, static_cast<uint8_t> (crc >> 16));
        put (b, o + 3, static_cast<uint8_t> (crc >> 24));
      }

      constexpr uint8_t ds4_flag0_motor {0x01};
      constexpr uint8_t ds4_flag0_led {0x02};
      constexpr uint8_t ds4_hwctl_crc32 {0x40};
      constexpr uint8_t ds4_hwctl_hid {0x80};

      constexpr uint8_t ds_flag0_compatible_vibration {0x01};
      constexpr uint8_t ds_flag0_haptics_select {0x02};
      constexpr uint8_t ds_flag1_lightbar_enable {0x04};
      constexpr uint8_t ds_flag1_player_leds_enable {0x10};

      constexpr uint8_t ds_flag0_right_trigger_effect {0x04};
      constexpr uint8_t ds_flag0_left_trigger_effect {0x08};

      constexpr size_t ds_right_trigger_effect_offset {10};
      constexpr size_t ds_left_trigger_effect_offset {21};
      constexpr size_t ds_trigger_effect_size {11};

      constexpr uint8_t ds_trigger_off {0x00};
      constexpr uint8_t ds_trigger_feedback {0x01};
      constexpr uint8_t ds_trigger_weapon {0x02};

      constexpr uint8_t ds_trigger_max_position {9};
      constexpr uint8_t ds_trigger_max_strength {8};

      uint8_t
      clamp_to (uint8_t v, uint8_t hi) noexcept
      {
        return v > hi ? hi : v;
      }

      bool
      encode_trigger_effect (const adaptive_trigger_request& t,
                             std::span<std::byte> e) noexcept
      {
        assert (e.size () >= ds_trigger_effect_size);

        std::fill_n (e.data (), ds_trigger_effect_size, std::byte {});

        const uint8_t start (clamp_to (t.start_position, ds_trigger_max_position));
        const uint8_t strength (clamp_to (t.strength, ds_trigger_max_strength));

        switch (t.effect)
        {
          case trigger_effect::off:
            {
              put (e, 0, ds_trigger_off);
              return true;
            }

          case trigger_effect::feedback:
            {
              put (e, 0, ds_trigger_feedback);
              put (e, 1, start);
              put (e, 2, strength);
              return true;
            }

          case trigger_effect::weapon:
            {
              const uint8_t end (
                clamp_to (t.end_position > start ? t.end_position
                                                 : static_cast<uint8_t> (start + 1),
                          ds_trigger_max_position));

              if (end <= start)
                return false;

              put (e, 0, ds_trigger_weapon);
              put (e, 1, start);
              put (e, 2, end);
              return true;
            }
        }

        return false;
      }

      constexpr uint8_t ds_output_tag {0x10};

      constexpr uint8_t ds_haptic_report_id {0x32};
      constexpr uint8_t ds_packet_sized {0x80};

      constexpr uint8_t ds_haptic_control_id {0x11};
      constexpr uint8_t ds_haptic_control_length {7};

      constexpr uint8_t ds_haptic_control[ds_haptic_control_length]
      {
        0xFE, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00
      };

      constexpr uint8_t ds_haptic_samples_id {0x12};
      constexpr uint8_t ds_haptic_samples_length {
        static_cast<uint8_t> (haptic_frames_per_report * 2)};

      constexpr size_t ds_haptic_control_offset {2};
      constexpr size_t ds_haptic_control_data {ds_haptic_control_offset + 2};
      constexpr size_t ds_haptic_counter {
        ds_haptic_control_data + ds_haptic_control_length - 1};
      constexpr size_t ds_haptic_samples_offset {
        ds_haptic_control_data + ds_haptic_control_length};
      constexpr size_t ds_haptic_samples_data {ds_haptic_samples_offset + 2};
      constexpr size_t ds_haptic_crc {ds_haptic_report_size - 4};

      static_assert (ds_haptic_samples_data + ds_haptic_samples_length <=
                       ds_haptic_crc,
                     "the packets and the checksum must fit inside one report");

      constexpr uint8_t ds_haptic_silence {0x80};

      uint8_t
      to_sample (float v) noexcept
      {
        const float c (std::clamp (v, -1.0f, 1.0f));

        return static_cast<uint8_t> (
          std::lround (c * 127.0f) + static_cast<int> (ds_haptic_silence));
      }
    }

    std::optional<size_t>
    encode_dualshock4_output (const output_request& r,
                              connection link,
                              std::span<std::byte> out) noexcept
    {
      const bool bt (link == connection::bluetooth);

      if (link != connection::usb && !bt)
        return std::nullopt;

      const size_t size (bt ? ds4_output_bt_size : ds4_output_usb_size);
      const size_t base (bt ? 3 : 1);

      if (out.size () < size)
        return std::nullopt;

      std::fill_n (out.data (), size, std::byte {});

      if (bt)
      {
        put (out, 0, 0x11);
        put (out, 1, ds4_hwctl_hid | ds4_hwctl_crc32);
      }
      else
      {
        put (out, 0, 0x05);
      }

      if (const auto* rr = std::get_if<rumble_request> (&r))
      {
        put (out, base + 0, ds4_flag0_motor);
        put (out, base + 3, to_byte (rr->high_frequency));
        put (out, base + 4, to_byte (rr->low_frequency));
      }
      else if (const auto* lb = std::get_if<light_bar_request> (&r))
      {
        put (out, base + 0, ds4_flag0_led);
        put (out, base + 5, lb->red);
        put (out, base + 6, lb->green);
        put (out, base + 7, lb->blue);
      }
      else
      {
        return std::nullopt;
      }

      if (bt)
        seal_bt_crc (out.subspan (0, size));

      return size;
    }

    std::optional<size_t>
    encode_dualsense_output (const output_request& r,
                             connection link,
                             uint8_t& bt_sequence,
                             std::span<std::byte> out) noexcept
    {
      const bool bt (link == connection::bluetooth);

      if (link != connection::usb && !bt)
        return std::nullopt;

      const size_t size (bt ? ds_output_bt_size : ds_output_usb_size);
      const size_t base (bt ? 3 : 1);

      if (out.size () < size)
        return std::nullopt;

      std::fill_n (out.data (), size, std::byte {});

      if (bt)
      {
        put (out, 0, 0x31);

        put (out, 1, static_cast<uint8_t> ((bt_sequence & 0x0F) << 4));
        put (out, 2, ds_output_tag);

        bt_sequence = static_cast<uint8_t> ((bt_sequence + 1) & 0x0F);
      }
      else
      {
        put (out, 0, 0x02);
      }

      if (const auto* rr = std::get_if<rumble_request> (&r))
      {
        put (out, base + 0, ds_flag0_haptics_select | ds_flag0_compatible_vibration);
        put (out, base + 2, to_byte (rr->high_frequency));
        put (out, base + 3, to_byte (rr->low_frequency));
      }
      else if (const auto* lb = std::get_if<light_bar_request> (&r))
      {
        put (out, base + 1, ds_flag1_lightbar_enable);
        put (out, base + 44, lb->red);
        put (out, base + 45, lb->green);
        put (out, base + 46, lb->blue);
      }
      else if (const auto* pl = std::get_if<player_led_request> (&r))
      {
        put (out, base + 1, ds_flag1_player_leds_enable);
        put (out, base + 43, pl->mask & 0x1F);
      }
      else if (const auto* at = std::get_if<adaptive_trigger_request> (&r))
      {
        const bool right (at->side == trigger_side::right);

        const size_t o (base + (right ? ds_right_trigger_effect_offset
                                      : ds_left_trigger_effect_offset));

        if (!encode_trigger_effect (*at, out.subspan (o, ds_trigger_effect_size)))
          return std::nullopt;

        put (out, base + 0, right ? ds_flag0_right_trigger_effect
                                  : ds_flag0_left_trigger_effect);
      }
      else
      {
        return std::nullopt;
      }

      if (bt)
        seal_bt_crc (out.subspan (0, size));

      return size;
    }

    std::optional<size_t>
    encode_dualsense_haptics (std::span<const haptic::frame> frames,
                              uint8_t& counter,
                              std::span<std::byte> out) noexcept
    {
      if (frames.size () != haptic_frames_per_report ||
          out.size () < ds_haptic_report_size)
        return std::nullopt;

      std::fill_n (out.data (), ds_haptic_report_size, std::byte {});

      put (out, 0, ds_haptic_report_id);
      put (out, 1, 0);

      put (out, ds_haptic_control_offset, ds_haptic_control_id | ds_packet_sized);
      put (out, ds_haptic_control_offset + 1, ds_haptic_control_length);

      for (size_t i (0); i != ds_haptic_control_length; ++i)
        put (out, ds_haptic_control_data + i, ds_haptic_control[i]);

      put (out, ds_haptic_counter, counter++);

      put (out, ds_haptic_samples_offset, ds_haptic_samples_id | ds_packet_sized);
      put (out, ds_haptic_samples_offset + 1, ds_haptic_samples_length);

      for (size_t i (0); i != frames.size (); ++i)
      {
        put (out, ds_haptic_samples_data + i * 2 + 0, to_sample (frames[i].left));
        put (out, ds_haptic_samples_data + i * 2 + 1, to_sample (frames[i].right));
      }

      seal_bt_crc (out.subspan (0, ds_haptic_report_size));

      return ds_haptic_report_size;
    }
  }
}
