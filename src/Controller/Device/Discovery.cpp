#include "Discovery.hpp"

#include "../Types.hpp"

#include <cassert>
#include <algorithm>

#include "../Transport/Hid.hpp"

namespace Controller
{
  namespace
  {
    constexpr uint8_t subtype_wheel {0x02};
    constexpr uint8_t subtype_arcade_stick {0x03};
    constexpr uint8_t subtype_flight_stick {0x04};
    constexpr uint8_t subtype_dance_pad {0x05};
    constexpr uint8_t subtype_guitar {0x06};
    constexpr uint8_t subtype_guitar_alternate {0x07};
    constexpr uint8_t subtype_drum_kit {0x08};
    constexpr uint8_t subtype_guitar_bass {0x0B};

    bool
    maps_onto_a_gamepad (uint8_t subtype) noexcept
    {
      switch (subtype)
      {
        case subtype_wheel:
        case subtype_arcade_stick:
        case subtype_flight_stick:
        case subtype_dance_pad:
        case subtype_guitar:
        case subtype_guitar_alternate:
        case subtype_guitar_bass:
        case subtype_drum_kit:
          return false;

        default:
          return true;
      }
    }
  }

  namespace
  {
    constexpr capabilities dualsense_capabilities {
      capability::gyroscope |
      capability::accelerometer |
      capability::touchpad |
      capability::battery |
      capability::microphone_button |
      capability::rumble |
      capability::haptics |
      capability::adaptive_triggers |
      capability::light_bar |
      capability::player_leds};

    capabilities
    capabilities_for (family f) noexcept
    {
      switch (f)
      {
        case family::xbox:
          return capabilities (capability::rumble);

        case family::dualshock4:
          return capability::gyroscope |
                 capability::accelerometer |
                 capability::touchpad |
                 capability::battery |
                 capability::rumble |
                 capability::light_bar;

        case family::dualsense:
          return dualsense_capabilities;

        case family::dualsense_edge:
          return dualsense_capabilities | capability::back_buttons;

        case family::unknown:
          break;
      }

      return capabilities ();
    }
  }

  discovery::
  discovery (const context& ctx,
             registry& r,
             const transport::xinput_module& x)
    : ctx_ (ctx), registry_ (r), xinput_ (x), notifier_ (ctx)
  {
  }

  void
  discovery::
  scan ()
  {
    const bool changed (notifier_.consume ());
    const timestamp now (clock::now ());

    if (!changed && scanned_ && now - last_scan_ < interval)
      return;

    last_scan_ = now;
    scanned_ = true;

    scan_now ();
  }

  void
  discovery::
  scan_now ()
  {
    std::vector<transport_binding> seen;
    seen.reserve (user_index::count + 4);

    scan_xinput (seen);
    scan_hid (seen);

    retire_unseen (seen);
  }

  void
  discovery::
  scan_xinput (std::vector<transport_binding>& seen)
  {
    if (!xinput_.loaded ())
      return;

    for (uint8_t i (0); i < user_index::count; ++i)
    {
      XINPUT_STATE state {};

      if (xinput_.get_state (i, state) != ERROR_SUCCESS)
        continue;

      XINPUT_CAPABILITIES caps {};

      if (xinput_.get_capabilities (i, 0, caps) == ERROR_SUCCESS &&
          !maps_onto_a_gamepad (caps.SubType))
        continue;

      const user_index slot (i);

      registry_.add (device_identity {family::xbox, std::nullopt, std::nullopt, std::nullopt},
                     transport_kind::xinput,
                     connection::unknown,
                     capabilities_for (family::xbox),
                     xinput_binding {slot});

      seen.push_back (xinput_binding {slot});
    }
  }

  void
  discovery::
  scan_hid (std::vector<transport_binding>& seen)
  {
    std::vector<std::wstring> unbound;

    for (transport::hid_enumeration_entry& e: transport::enumerate (ctx_))
    {
      const family f (classify (e.attributes.vendor, e.attributes.product));

      assert (f != family::unknown);

      if (e.link == connection::unknown)
      {
        if (std::find (unbound_.begin (), unbound_.end (), e.path) == unbound_.end ())
          ctx_.report (severity::warning, facility::discovery,
                       errc::ambiguous_identity,
                       std::string ("HID device ") +
                       to_string (f) +
                       " reports an input report of " +
                       std::to_string (e.input_report_length) +
                       " bytes; the drivers decode 64-byte (USB) and 78-byte "
                       "(Bluetooth) framing, so no driver is bound");

        unbound.push_back (std::move (e.path));
        continue;
      }

      registry_.add (device_identity {f,
                                      e.attributes.vendor,
                                      e.attributes.product,
                                      e.attributes.version},
                     transport_kind::hid,
                     e.link,
                     capabilities_for (f),
                     hid_binding {e.path});

      seen.push_back (hid_binding {std::move (e.path)});
    }

    unbound_ = std::move (unbound);
  }

  void
  discovery::
  retire_unseen (const std::vector<transport_binding>& seen)
  {
    std::vector<device_id> departed;

    registry_.for_each ([&seen, &departed] (const device_connection& d)
    {
      const bool present (
        std::any_of (seen.begin (), seen.end (),
                     [&d] (const transport_binding& b)
                     {
                       return same_binding (d.binding, b);
                     }));

      if (!present)
        departed.push_back (d.id);
    });

    for (device_id id: departed)
      registry_.remove (id);
  }
}
