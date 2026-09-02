#include "DualSenseEdge.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace driver
  {
    bool
    decode_dualsense_edge (std::span<const std::byte> report,
                           connection link,
                           raw_sample& raw,
                           canonical_sample& canonical) noexcept
    {
      return decode_dualsense (report, link, raw, canonical, true);
    }

    bool
    dualsense_edge_driver::
    poll (raw_sample& raw, canonical_sample& canonical) noexcept
    {
      return read_and_decode (raw, canonical, true);
    }
  }
}
