#pragma once

#include "../Types.hpp"

#include "../Sample/Axis.hpp"
#include "../Sample/Button.hpp"

namespace Controller
{
  namespace driver
  {
    uint8_t
    rd_u8 (std::span<const std::byte>, size_t offset) noexcept;

    uint16_t
    rd_le16 (std::span<const std::byte>, size_t offset) noexcept;

    int16_t
    rd_le16s (std::span<const std::byte>, size_t offset) noexcept;

    uint32_t
    rd_le32 (std::span<const std::byte>, size_t offset) noexcept;

    uint32_t
    crc32_le (uint32_t init, std::span<const std::byte>) noexcept;

    inline constexpr uint8_t ps_input_crc_seed {0xA1};
    inline constexpr uint8_t ps_output_crc_seed {0xA2};
    inline constexpr uint8_t ps_feature_crc_seed {0xA3};

    bool
    verify_ps_crc32 (uint8_t seed, std::span<const std::byte> data, uint32_t expected) noexcept;

    struct ps_touch_point
    {
      bool active;
      uint8_t id;
      uint16_t x;
      uint16_t y;
    };

    ps_touch_point
    decode_touch_point (std::span<const std::byte> p) noexcept;

    void
    apply_hat (button_set&, uint8_t hat) noexcept;

    stick_vector
    normalize_ps_stick (uint8_t x, uint8_t y) noexcept;
  }
}
