#pragma once

#include "../Types.hpp"

#include "../Device/Id.hpp"
#include "../Device/Identity.hpp"
#include "../Sample/Sample.hpp"
#include "Output.hpp"

namespace Controller
{
  namespace driver
  {
    inline constexpr size_t max_reports_per_poll {32};

    class driver
    {
    public:
      virtual
      ~driver () = default;

      virtual Controller::family
      family () const noexcept = 0;

      virtual device_id
      device () const noexcept = 0;

      virtual bool
      poll (raw_sample& raw, canonical_sample& canonical) noexcept = 0;

      virtual void
      submit (const output_request&) noexcept = 0;

      virtual void
      configure (const output_policy&) noexcept {}

      virtual void
      stop_haptic (uint32_t) noexcept {}

      virtual std::string
      diagnostics () const {return {};}
    };
  }
}
