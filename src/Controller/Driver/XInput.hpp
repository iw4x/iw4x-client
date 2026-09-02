#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Driver.hpp"
#include "../Transport/XInputModule.hpp"

namespace Controller
{
  namespace driver
  {
    void
    decode_xinput (const XINPUT_GAMEPAD&,
                   bool has_guide,
                   raw_sample&,
                   canonical_sample&) noexcept;

    class xinput_driver: public driver
    {
    public:
      xinput_driver (const context&,
                     const transport::xinput_module&,
                     device_id,
                     user_index);

      Controller::family
      family () const noexcept override {return Controller::family::xbox;}

      device_id
      device () const noexcept override {return device_;}

      bool
      poll (raw_sample&, canonical_sample&) noexcept override;

      void
      submit (const output_request&) noexcept override;

    private:
      const context& ctx_;
      const transport::xinput_module& module_;
      device_id device_;
      user_index index_;

      uint32_t last_packet_ {0};
      bool have_packet_ {false};
    };
  }
}
