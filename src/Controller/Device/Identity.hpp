#pragma once

#include "../Types.hpp"

namespace Controller
{
  class vendor_id
  {
  public:
    constexpr explicit
    vendor_id (uint16_t v) noexcept: value_ (v) {}

    constexpr uint16_t
    value () const noexcept {return value_;}

    friend constexpr bool
    operator== (vendor_id, vendor_id) noexcept = default;

  private:
    uint16_t value_;
  };

  class product_id
  {
  public:
    constexpr explicit
    product_id (uint16_t v) noexcept: value_ (v) {}

    constexpr uint16_t
    value () const noexcept {return value_;}

    friend constexpr bool
    operator== (product_id, product_id) noexcept = default;

  private:
    uint16_t value_;
  };

  enum class family : uint8_t
  {
    unknown,
    xbox,
    dualshock4,
    dualsense,
    dualsense_edge,
  };

  const char*
  to_string (family) noexcept;

  std::ostream&
  operator<< (std::ostream&, family);

  inline constexpr vendor_id vendor_sony {0x054C};
  inline constexpr vendor_id vendor_microsoft {0x045E};

  inline constexpr product_id product_ds4_gen1 {0x05C4};
  inline constexpr product_id product_ds4_gen2 {0x09CC};
  inline constexpr product_id product_ds4_dongle {0x0BA0};
  inline constexpr product_id product_dualsense {0x0CE6};
  inline constexpr product_id product_dualsense_edge {0x0DF2};

  struct device_identity
  {
    Controller::family family {family::unknown};
    std::optional<vendor_id> vendor;
    std::optional<product_id> product;

    std::optional<uint16_t> release;
  };

  family
  classify (vendor_id, product_id) noexcept;
}
