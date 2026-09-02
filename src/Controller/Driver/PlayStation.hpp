#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Device/Id.hpp"
#include "../Transport/Hid.hpp"

namespace Controller
{
  namespace driver
  {
    bool
    enable_extended_reports (const context&,
                             transport::hid_device&,
                             device_id) noexcept;

    bool
    minimal_bluetooth_report (std::span<const std::byte>, connection link) noexcept;
  }
}
