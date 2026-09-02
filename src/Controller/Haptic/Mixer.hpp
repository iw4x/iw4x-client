#pragma once

#include "../Types.hpp"

#include "Effect.hpp"

namespace Controller
{
  namespace haptic
  {
    class mixer
    {
    public:
      static constexpr size_t voices {12};

      mixer () = default;

      mixer (const mixer&) = delete;
      mixer& operator= (const mixer&) = delete;

      bool
      play (const effect&) noexcept;

      void
      stop (uint32_t tag) noexcept;

      void
      stop_all () noexcept;

      void
      set_rumble (float low_frequency, float high_frequency) noexcept;

      void
      render (std::span<frame>, uint32_t rate) noexcept;

      bool
      has_effects () const noexcept
      {
        return started_.load (std::memory_order_relaxed) != 0;
      }

      uint64_t
      dropped () const noexcept {return dropped_.load (std::memory_order_relaxed);}

    private:
      enum class state : uint8_t
      {
        free,
        filling,
        ready,
        playing,
      };

      struct voice
      {
        std::atomic<state> phase {state::free};

        std::atomic<bool> stopping {false};

        effect what {};

        float elapsed {0.0f};
        float deep_phase {0.0f};
        float crisp_phase {0.0f};
      };

      bool
      render_voice (voice&, std::span<frame>, float step) noexcept;

      void
      render_rumble (std::span<frame>, float step) noexcept;

      std::array<voice, voices> voices_ {};

      std::atomic<float> rumble_low_ {0.0f};
      std::atomic<float> rumble_high_ {0.0f};

      float rumble_low_level_ {0.0f};
      float rumble_high_level_ {0.0f};
      float rumble_low_phase_ {0.0f};
      float rumble_high_phase_ {0.0f};

      std::atomic<uint64_t> started_ {0};
      std::atomic<uint64_t> dropped_ {0};
    };
  }
}
