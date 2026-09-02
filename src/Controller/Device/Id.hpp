#pragma once

#include "../Types.hpp"

namespace Controller
{
  class device_id
  {
  public:
    constexpr device_id () = default;

    constexpr explicit
    device_id (uint32_t v) noexcept: value_ (v) {}

    constexpr uint32_t
    value () const noexcept {return value_;}

    constexpr explicit
    operator bool () const noexcept {return value_ != 0;}

    friend constexpr bool
    operator== (device_id, device_id) noexcept = default;

    friend constexpr std::strong_ordering
    operator<=> (device_id, device_id) noexcept = default;

  private:
    uint32_t value_ {0};
  };

  inline constexpr device_id no_device {};

  std::ostream&
  operator<< (std::ostream&, device_id);

  enum class transport_kind : uint8_t
  {
    unknown,
    xinput,
    raw_input,
    hid,
  };

  const char*
  to_string (transport_kind) noexcept;

  std::ostream&
  operator<< (std::ostream&, transport_kind);

  enum class connection : uint8_t
  {
    unknown,
    usb,
    bluetooth,
    virtualized,
  };

  const char*
  to_string (connection) noexcept;

  std::ostream&
  operator<< (std::ostream&, connection);

  class user_index
  {
  public:
    static constexpr uint8_t count {4};

    constexpr explicit
    user_index (uint8_t v) noexcept: value_ (v) {}

    static constexpr std::optional<user_index>
    try_from (int v) noexcept
    {
      if (v < 0 || v >= count)
        return std::nullopt;

      return user_index (static_cast<uint8_t> (v));
    }

    constexpr uint8_t
    value () const noexcept {return value_;}

    friend constexpr bool
    operator== (user_index, user_index) noexcept = default;

    friend constexpr std::strong_ordering
    operator<=> (user_index, user_index) noexcept = default;

  private:
    uint8_t value_;
  };

  class report_id
  {
  public:
    constexpr report_id () = default;

    constexpr explicit
    report_id (uint8_t v) noexcept: value_ (v) {}

    constexpr uint8_t
    value () const noexcept {return value_;}

    friend constexpr bool
    operator== (report_id, report_id) noexcept = default;

    friend constexpr std::strong_ordering
    operator<=> (report_id, report_id) noexcept = default;

  private:
    uint8_t value_ {0};
  };

  static_assert (sizeof (report_id) == 1);
  static_assert (static_cast<uint8_t> (transport_kind::hid) < 4);
  static_assert (static_cast<uint8_t> (connection::virtualized) < 4);
}
