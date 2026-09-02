#include "Identity.hpp"

#include "../Types.hpp"

namespace Controller
{
  const char*
  to_string (family f) noexcept
  {
    switch (f)
    {
      case family::unknown:        return "unknown";
      case family::xbox:           return "xbox";
      case family::dualshock4:     return "dualshock4";
      case family::dualsense:      return "dualsense";
      case family::dualsense_edge: return "dualsense-edge";
    }

    return "unknown";
  }

  std::ostream&
  operator<< (std::ostream& os, family f)
  {
    return os << to_string (f);
  }

  family
  classify (vendor_id v, product_id p) noexcept
  {
    if (v == vendor_sony)
    {
      if (p == product_dualsense_edge)
        return family::dualsense_edge;

      if (p == product_dualsense)
        return family::dualsense;

      if (p == product_ds4_gen1 ||
          p == product_ds4_gen2 ||
          p == product_ds4_dongle)
        return family::dualshock4;
    }

    return family::unknown;
  }
}
