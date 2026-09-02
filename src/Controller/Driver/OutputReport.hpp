#pragma once

#include "../Types.hpp"

#include "../Device/Id.hpp"
#include "../Haptic/Effect.hpp"
#include "Output.hpp"

namespace Controller
{
  namespace driver
  {
    inline constexpr size_t ds4_output_usb_size {32};
    inline constexpr size_t ds4_output_bt_size {78};
    inline constexpr size_t ds_output_usb_size {63};
    inline constexpr size_t ds_output_bt_size {78};

    std::optional<size_t>
    encode_dualshock4_output (const output_request&,
                              connection,
                              std::span<std::byte> out) noexcept;

    std::optional<size_t>
    encode_dualsense_output (const output_request&,
                             connection,
                             uint8_t& bt_sequence,
                             std::span<std::byte> out) noexcept;

    inline constexpr uint32_t haptic_sample_rate {3000};
    inline constexpr size_t haptic_frames_per_report {32};
    inline constexpr size_t ds_haptic_report_size {142};

    std::optional<size_t>
    encode_dualsense_haptics (std::span<const haptic::frame>,
                              uint8_t& counter,
                              std::span<std::byte> out) noexcept;
  }
}
