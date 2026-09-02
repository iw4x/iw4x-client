#include "Graph.hpp"

#include "../Types.hpp"

#include <cmath>
#include <cassert>

namespace Controller
{
  namespace aim
  {
    std::optional<aim_graph>
    aim_graph::
    make (std::span<const knot> ks, bool require_monotonic, std::string& why)
    {
      if (ks.size () < 2)
      {
        why = "an aim graph requires at least two knots";
        return std::nullopt;
      }

      if (ks.size () > max_graph_knots)
      {
        why = "an aim graph has too many knots";
        return std::nullopt;
      }

      bool non_decreasing (true);

      for (size_t i (0); i < ks.size (); ++i)
      {
        if (!std::isfinite (ks[i].input) || !std::isfinite (ks[i].output))
        {
          why = "aim graph knot values must be finite";
          return std::nullopt;
        }

        if (ks[i].input < 0.0f || ks[i].input > 1.0f)
        {
          why = "aim graph knot inputs must lie in [0, 1]";
          return std::nullopt;
        }

        if (i > 0)
        {
          if (ks[i].input <= ks[i - 1].input)
          {
            why = "aim graph knot inputs must be strictly increasing";
            return std::nullopt;
          }

          if (ks[i].output < ks[i - 1].output)
            non_decreasing = false;
        }
      }

      if (require_monotonic && !non_decreasing)
      {
        why = "a monotonic aim graph requires non-decreasing outputs";
        return std::nullopt;
      }

      aim_graph g;
      for (const knot& k: ks)
        g.knots_.push_back (k);
      g.monotonic_ = non_decreasing;

      return g;
    }

    float
    aim_graph::
    evaluate (float input) const noexcept
    {
      assert (knots_.size () >= 2);

      const knot& first (knots_.front ());
      const knot& last (knots_.back ());

      if (input <= first.input)
        return first.output;
      if (input >= last.input)
        return last.output;

      for (size_t i (1); i < knots_.size (); ++i)
      {
        if (input <= knots_[i].input)
        {
          const knot& a (knots_[i - 1]);
          const knot& b (knots_[i]);

          const float span (b.input - a.input);
          assert (span > 0.0f);

          const float t ((input - a.input) / span);
          return a.output + (b.output - a.output) * t;
        }
      }

      return last.output;
    }
  }
}
