#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Haptic/Stream.hpp"
#include "Driver.hpp"
#include "../Transport/Hid.hpp"

namespace Controller
{
  namespace driver
  {
    bool
    decode_dualsense (std::span<const std::byte> report,
                      connection link,
                      raw_sample& raw,
                      canonical_sample& canonical,
                      bool edge = false) noexcept;

    class dualsense_driver: public driver
    {
    public:
      dualsense_driver (const context&, transport::hid_device&, device_id);

      Controller::family
      family () const noexcept override {return Controller::family::dualsense;}

      device_id
      device () const noexcept override {return device_;}

      bool
      poll (raw_sample&, canonical_sample&) noexcept override;

      void
      submit (const output_request&) noexcept override;

      void
      configure (const output_policy&) noexcept override;

      void
      stop_haptic (uint32_t) noexcept override;

      std::string
      diagnostics () const override;

    protected:
      void
      submit_report (const output_request&) noexcept;

      void
      queue_rumble (const rumble_request&) noexcept;

      void
      flush_rumble () noexcept;

      bool
      ensure_haptics () noexcept;

      bool
      play_waveform (const rumble_request&) noexcept;

      bool
      play_effect (const haptic::effect&) noexcept;

      bool
      read_and_decode (raw_sample&, canonical_sample&, bool edge) noexcept;

      const context& ctx_;
      transport::hid_device& hid_;
      device_id device_;
      connection link_;

      uint8_t bt_output_sequence_ {0};

      bool minimal_reported_ {false};

      output_policy policy_ {};

      rumble_request pending_rumble_ {};
      bool rumble_pending_ {false};
      bool rumble_sent_ {false};
      timestamp last_rumble_ {};

      std::unique_ptr<haptic::stream> haptics_;

      bool haptics_failed_ {false};

      bool reported_fallback_ {false};
      bool reported_effect_drop_ {false};

      bool emulating_ {false};
    };
  }
}
