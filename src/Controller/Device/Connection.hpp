#pragma once

#include "../Types.hpp"

#include "Id.hpp"
#include "Identity.hpp"
#include "Capability.hpp"

#include <variant>

namespace Controller
{
  struct xinput_binding
  {
    user_index index;
  };

  struct hid_binding
  {
    std::wstring path;
  };

  using transport_binding = std::variant<std::monostate,
                                    xinput_binding,
                                    hid_binding>;

  bool
  same_binding (const transport_binding&, const transport_binding&) noexcept;

  struct device_connection
  {
    device_id id {};
    device_identity identity {};
    Controller::transport_kind transport {transport_kind::unknown};
    connection link {connection::unknown};
    capabilities caps {};
    transport_binding binding {};
  };
}
