#include "Connection.hpp"

#include "../Types.hpp"

#include <variant>

namespace Controller
{
  bool
  same_binding (const transport_binding& a, const transport_binding& b) noexcept
  {
    if (a.index () != b.index ())
      return false;

    if (const auto* xa = std::get_if<xinput_binding> (&a))
      return xa->index == std::get<xinput_binding> (b).index;

    if (const auto* ha = std::get_if<hid_binding> (&a))
      return ha->path == std::get<hid_binding> (b).path;

    return false;
  }
}
