#include "Calibration.hpp"

#include "../Types.hpp"

#include <cmath>

namespace Controller
{
  namespace aim
  {
    float
    fov_scale (degrees fov, degrees reference_fov) noexcept
    {
      const float f (std::tan (to_radians (fov).value * 0.5f));
      const float r (std::tan (to_radians (reference_fov).value * 0.5f));

      if (!(r > 0.0f) || !std::isfinite (f))
        return 1.0f;

      return f / r;
    }

    std::optional<aim_calibration>
    aim_calibration::
    make (const aim_settings& s, std::string& why)
    {
      std::string w;

      if (!validate (s.hip.deadzone, w))
      {
        why = "hip deadzone: " + w;
        return std::nullopt;
      }

      if (!validate (s.ads.deadzone, w))
      {
        why = "ADS deadzone: " + w;
        return std::nullopt;
      }

      std::optional<aim_graph> graph;
      if (s.graph_knots)
      {
        std::optional<aim_graph> built (
          aim_graph::make (std::span<const knot> (s.graph_knots->data (),
                                             s.graph_knots->size ()),
                           s.graph_monotonic, w));
        if (!built)
        {
          why = "aim graph: " + w;
          return std::nullopt;
        }

        graph = std::move (built);
      }

      aim_calibration c;
      c.hip_ = s.hip;
      c.ads_ = s.ads;
      c.accel_ = s.accel;
      c.graph_ = std::move (graph);
      return c;
    }

    aim_processor::config
    aim_calibration::
    processor_config () const noexcept
    {
      return {hip_, ads_, accel_, graph_ ? &*graph_ : nullptr};
    }
  }
}
