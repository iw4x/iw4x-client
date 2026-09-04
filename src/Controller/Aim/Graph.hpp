#pragma once

#include "../Types.hpp"

#include "../Support/InplaceVector.hpp"

namespace Controller
{
  namespace aim
  {
    struct knot
    {
      float input {0.0f};
      float output {0.0f};
    };

    inline constexpr size_t max_graph_knots {32};

    class aim_graph
    {
    public:
      static std::optional<aim_graph>
      make (std::span<const knot>, bool require_monotonic, std::string& why);

      float
      evaluate (float input) const noexcept;

      std::span<const knot>
      knots () const noexcept {return {knots_.data (), knots_.size ()};}

      bool
      monotonic () const noexcept {return monotonic_;}

    private:
      aim_graph () = default;

      inplace_vector<knot, max_graph_knots> knots_;
      bool monotonic_ {false};
    };
  }
}
