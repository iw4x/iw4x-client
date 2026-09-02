#include "Mixer.hpp"

#include "../Types.hpp"

#include <cmath>
#include <algorithm>

namespace Controller
{
  namespace haptic
  {
    namespace
    {
      constexpr float two_pi {6.28318530717958647692f};

      constexpr float rumble_low_hertz {60.0f};
      constexpr float rumble_high_hertz {180.0f};

      constexpr float rumble_ramp_seconds {0.004f};

      void
      advance (float& phase, float increment) noexcept
      {
        phase += increment;

        while (phase >= 1.0f)
          phase -= 1.0f;
      }

      float
      approach (float current, float target, float coefficient) noexcept
      {
        return current + (target - current) * coefficient;
      }
    }

    bool
    mixer::
    play (const effect& e) noexcept
    {
      for (voice& v: voices_)
      {
        state expected (state::free);

        if (!v.phase.compare_exchange_strong (expected,
                                              state::filling,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed))
          continue;

        v.what = e;
        v.stopping.store (false, std::memory_order_relaxed);

        v.phase.store (state::ready, std::memory_order_release);

        started_.fetch_add (1, std::memory_order_relaxed);
        return true;
      }

      dropped_.fetch_add (1, std::memory_order_relaxed);
      return false;
    }

    void
    mixer::
    stop (uint32_t tag) noexcept
    {
      if (tag == 0)
        return;

      for (voice& v: voices_)
      {
        const state phase (v.phase.load (std::memory_order_acquire));

        if (phase != state::ready && phase != state::playing)
          continue;

        if (v.what.tag == tag)
          v.stopping.store (true, std::memory_order_relaxed);
      }
    }

    void
    mixer::
    stop_all () noexcept
    {
      for (voice& v: voices_)
        v.stopping.store (true, std::memory_order_relaxed);

      rumble_low_.store (0.0f, std::memory_order_relaxed);
      rumble_high_.store (0.0f, std::memory_order_relaxed);
    }

    void
    mixer::
    set_rumble (float low_frequency, float high_frequency) noexcept
    {
      rumble_low_.store (std::clamp (low_frequency, 0.0f, 1.0f),
                         std::memory_order_relaxed);
      rumble_high_.store (std::clamp (high_frequency, 0.0f, 1.0f),
                          std::memory_order_relaxed);
    }

    void
    mixer::
    render_rumble (std::span<frame> out, float step) noexcept
    {
      const float target_low (rumble_low_.load (std::memory_order_relaxed));
      const float target_high (rumble_high_.load (std::memory_order_relaxed));

      const float coefficient (std::min (step / rumble_ramp_seconds, 1.0f));

      const float low_increment (rumble_low_hertz * step);
      const float high_increment (rumble_high_hertz * step);

      for (frame& f: out)
      {
        rumble_low_level_ = approach (rumble_low_level_, target_low, coefficient);
        rumble_high_level_ = approach (rumble_high_level_, target_high, coefficient);

        f.left += rumble_low_level_ * std::sin (rumble_low_phase_ * two_pi);
        f.right += rumble_high_level_ * std::sin (rumble_high_phase_ * two_pi);

        advance (rumble_low_phase_, low_increment);
        advance (rumble_high_phase_, high_increment);
      }
    }

    bool
    mixer::
    render_voice (voice& v, std::span<frame> out, float step) noexcept
    {
      const effect& e (v.what);

      const float duration (e.duration.count ());

      if (!(duration > 0.0f))
        return true;

      const bool looping (e.loop && !v.stopping.load (std::memory_order_relaxed));

      const float deep_increment (hertz_for (e.deep_sharpness) * step);
      const float crisp_increment (hertz_for (e.crisp_sharpness) * step);

      for (frame& f: out)
      {
        if (v.elapsed >= duration)
        {
          if (!looping)
            return true;

          v.elapsed -= duration;
        }

        const float t (v.elapsed / duration);

        const float deep (e.deep.evaluate (t) * e.intensity *
                          std::sin (v.deep_phase * two_pi));
        const float crisp (e.crisp.evaluate (t) * e.intensity *
                           std::sin (v.crisp_phase * two_pi));

        switch (e.where)
        {
          case actuator::both:
            {
              f.left += deep;
              f.right += crisp;
              break;
            }

          case actuator::left:
            {
              f.left += deep + crisp;
              break;
            }

          case actuator::right:
            {
              f.right += deep + crisp;
              break;
            }
        }

        advance (v.deep_phase, deep_increment);
        advance (v.crisp_phase, crisp_increment);

        v.elapsed += step;
      }

      return false;
    }

    void
    mixer::
    render (std::span<frame> out, uint32_t rate) noexcept
    {
      if (rate == 0)
        return;

      const float step (1.0f / static_cast<float> (rate));

      std::fill (out.begin (), out.end (), frame {});

      render_rumble (out, step);

      for (voice& v: voices_)
      {
        state phase (v.phase.load (std::memory_order_acquire));

        if (phase == state::ready)
        {
          v.elapsed = 0.0f;
          v.deep_phase = 0.0f;
          v.crisp_phase = 0.0f;

          v.phase.store (state::playing, std::memory_order_relaxed);
          phase = state::playing;
        }

        if (phase != state::playing)
          continue;

        if (render_voice (v, out, step))
          v.phase.store (state::free, std::memory_order_release);
      }

      for (frame& f: out)
      {
        f.left = std::clamp (f.left, -1.0f, 1.0f);
        f.right = std::clamp (f.right, -1.0f, 1.0f);
      }
    }
  }
}
