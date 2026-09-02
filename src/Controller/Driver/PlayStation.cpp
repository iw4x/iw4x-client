#include "PlayStation.hpp"

#include "../Types.hpp"

#include <algorithm>

#include "Decode.hpp"

namespace Controller
{
  namespace driver
  {
    namespace
    {
      constexpr uint8_t ps_feature_calibration {0x05};
      constexpr size_t ps_feature_calibration_size {41};

      constexpr uint8_t ps_report_bt_minimal {0x01};

      constexpr size_t max_feature_size {128};
    }

    bool
    enable_extended_reports (const context& ctx,
                             transport::hid_device& hid,
                             device_id device) noexcept
    {
      const size_t n (std::clamp (hid.feature_report_length (),
                                  ps_feature_calibration_size,
                                  max_feature_size));

      std::array<std::byte, max_feature_size> buf {};
      buf[0] = static_cast<std::byte> (ps_feature_calibration);

      if (!hid.get_feature (std::span<std::byte> (buf.data (), n)))
      {
        ctx.report (severity::warning, facility::transport, errc::transport_failure,
                    device,
                    "unable to read the calibration feature report over Bluetooth; "
                    "the controller may keep sending minimal reports and produce no "
                    "input");
        return false;
      }

      return true;
    }

    bool
    minimal_bluetooth_report (std::span<const std::byte> r, connection link) noexcept
    {
      return link == connection::bluetooth &&
             !r.empty () &&
             rd_u8 (r, 0) == ps_report_bt_minimal;
    }
  }
}
