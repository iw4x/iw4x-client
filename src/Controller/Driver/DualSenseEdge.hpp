#pragma once

#include "../Types.hpp"

#include "DualSense.hpp"

namespace Controller
{
  namespace driver
  {
    bool
    decode_dualsense_edge (std::span<const std::byte> report,
                           connection link,
                           raw_sample& raw,
                           canonical_sample& canonical) noexcept;

    class dualsense_edge_driver: public dualsense_driver
    {
    public:
      using dualsense_driver::dualsense_driver;

      Controller::family
      family () const noexcept override
      {
        return Controller::family::dualsense_edge;
      }

      bool
      poll (raw_sample&, canonical_sample&) noexcept override;
    };
  }
}
