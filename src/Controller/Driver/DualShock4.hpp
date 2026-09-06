#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Driver.hpp"
#include "../Transport/Hid.hpp"

namespace Controller
{
  namespace driver
  {
    bool
    decode_dualshock4 (std::span<const std::byte> report,
                       connection link,
                       raw_sample& raw,
                       canonical_sample& canonical) noexcept;

    class dualshock4_driver: public driver
    {
    public:
      dualshock4_driver (const context&, transport::hid_device&, device_id);

      Controller::family
      family () const noexcept override {return Controller::family::dualshock4;}

      device_id
      device () const noexcept override {return device_;}

      bool
      poll (raw_sample&, canonical_sample&) noexcept override;

      void
      submit (const output_request&) noexcept override;

      void
      configure (const output_policy&) noexcept override;

    private:
      void
      submit_report (const output_request&) noexcept;

      void
      queue_rumble (const rumble_request&) noexcept;

      void
      flush_rumble () noexcept;

      const context& ctx_;
      transport::hid_device& hid_;
      device_id device_;
      connection link_;

      bool minimal_reported_ {false};
      bool unencodable_reported_ {false};

      output_policy policy_ {};

      rumble_request pending_rumble_ {};
      bool rumble_pending_ {false};
      bool rumble_sent_ {false};
      timestamp last_rumble_ {};
    };
  }
}
