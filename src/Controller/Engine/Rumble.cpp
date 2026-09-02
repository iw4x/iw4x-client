#include "Rumble.hpp"

#include "../Types.hpp"

#include <algorithm>

namespace Controller
{
  namespace engine
  {
    namespace
    {
      constexpr float milliseconds_per_second {1000.0f};

      constexpr float deep_sharpness {0.0f};
      constexpr float crisp_sharpness {0.7f};

      haptic::envelope
      envelope_from (const Game::RumbleGraph* graph) noexcept
      {
        if (graph == nullptr || graph->knotCount == 0)
          return {};

        const size_t count (std::min (static_cast<size_t> (graph->knotCount),
                                      haptic::envelope::max_knots));

        std::array<haptic::envelope::knot, haptic::envelope::max_knots> knots {};

        for (size_t i (0); i != count; ++i)
          knots[i] = {graph->knots[i][0], graph->knots[i][1]};

        return haptic::envelope::from ({knots.data (), count});
      }
    }

    bool
    effect_from_rumble (const Game::RumbleInfo& info,
                        float scale,
                        bool loop,
                        haptic::effect& out) noexcept
    {
      if (!(info.duration > 0.0f))
        return false;

      haptic::effect e;
      e.deep = envelope_from (info.lowRumbleGraph);
      e.crisp = envelope_from (info.highRumbleGraph);

      if (e.deep.empty () && e.crisp.empty ())
        return false;

      e.deep_sharpness = deep_sharpness;
      e.crisp_sharpness = crisp_sharpness;
      e.intensity = std::clamp (scale, 0.0f, 1.0f);
      e.duration = seconds {info.duration / milliseconds_per_second};
      e.loop = loop;

      e.tag = static_cast<uint32_t> (info.rumbleNameIndex + 1);

      out = e;
      return true;
    }
  }
}
