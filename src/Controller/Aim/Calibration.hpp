#pragma once

#include "../Types.hpp"

#include "Types.hpp"
#include "Graph.hpp"
#include "Assist.hpp"

namespace Controller
{
  namespace aim
  {
    float
    fov_scale (degrees fov, degrees reference_fov) noexcept;

    struct aim_settings
    {
      aim_profile hip;
      aim_profile ads;
      turn_integrator::limits accel;
      std::optional<std::vector<knot>> graph_knots;
      bool graph_monotonic {true};
    };

    class aim_calibration
    {
    public:
      static std::optional<aim_calibration>
      make (const aim_settings&, std::string& why);

      aim_processor::config
      processor_config () const noexcept;

    private:
      aim_calibration () = default;

      aim_profile hip_;
      aim_profile ads_;
      turn_integrator::limits accel_;
      std::optional<aim_graph> graph_;
    };
  }
}
