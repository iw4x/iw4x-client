#include "Effect.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace haptic
  {
    namespace
    {
      envelope
      struck () noexcept
      {
        constexpr envelope::knot knots[]
        {
          {0.00f, 0.0f},
          {0.04f, 1.0f},
          {0.35f, 0.45f},
          {1.00f, 0.0f},
        };

        return envelope::from (knots);
      }

      seconds
      transient_duration (float sharpness) noexcept
      {
        return seconds {std::lerp (0.09f, 0.02f, std::clamp (sharpness, 0.0f, 1.0f))};
      }

      constexpr float body_offset {0.35f};

      bool
      in_unit_range (float v) noexcept
      {
        return v >= 0.0f && v <= 1.0f;
      }
    }

    const char*
    to_string (actuator a) noexcept
    {
      switch (a)
      {
        case actuator::left:  return "left";
        case actuator::right: return "right";
        case actuator::both:  return "both";
      }

      return "both";
    }

    envelope
    envelope::
    level (float amplitude) noexcept
    {
      const float a (std::clamp (amplitude, 0.0f, 1.0f));

      const knot knots[] {{0.0f, a}, {1.0f, a}};
      return from (knots);
    }

    envelope
    envelope::
    from (std::span<const knot> ks) noexcept
    {
      envelope e;

      float last (-1.0f);

      for (const knot& k: ks)
      {
        if (e.knots_.size () == max_knots)
          break;

        if (!in_unit_range (k.at) || !in_unit_range (k.amplitude) || k.at <= last)
          continue;

        e.knots_.push_back (k);
        last = k.at;
      }

      if (e.knots_.size () < 2)
        e.knots_.clear ();

      return e;
    }

    float
    envelope::
    evaluate (float t) const noexcept
    {
      if (knots_.empty ())
        return 0.0f;

      if (t <= knots_.front ().at)
        return knots_.front ().amplitude;

      if (t >= knots_.back ().at)
        return knots_.back ().amplitude;

      for (size_t i (1); i != knots_.size (); ++i)
      {
        if (t > knots_[i].at)
          continue;

        const knot& a (knots_[i - 1]);
        const knot& b (knots_[i]);

        return std::lerp (a.amplitude, b.amplitude, (t - a.at) / (b.at - a.at));
      }

      return knots_.back ().amplitude;
    }

    float
    hertz_for (float sharpness) noexcept
    {
      const float t (std::clamp (sharpness, 0.0f, 1.0f));
      return min_hertz * std::pow (max_hertz / min_hertz, t);
    }

    effect
    transient (float intensity, float sharpness) noexcept
    {
      const float s (std::clamp (sharpness, 0.0f, 1.0f));

      effect e;
      e.deep = struck ();
      e.crisp = struck ();
      e.deep_sharpness = std::max (0.0f, s - body_offset);
      e.crisp_sharpness = s;
      e.intensity = std::clamp (intensity, 0.0f, 1.0f);
      e.duration = transient_duration (s);
      return e;
    }

    effect
    continuous (float intensity, float sharpness, seconds duration) noexcept
    {
      const float s (std::clamp (sharpness, 0.0f, 1.0f));

      effect e;

      e.deep = envelope::level (1.0f);
      e.crisp = envelope::level (1.0f);
      e.deep_sharpness = std::max (0.0f, s - body_offset);
      e.crisp_sharpness = s;
      e.intensity = std::clamp (intensity, 0.0f, 1.0f);
      e.duration = duration;
      return e;
    }
  }
}
