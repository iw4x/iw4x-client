#pragma once

#include "../Types.hpp"

#include "Frame.hpp"

#include <cassert>

namespace Controller
{
  template <size_t Capacity>
  class frame_ring
  {
  public:
    static constexpr size_t capacity {Capacity};

    static_assert (Capacity >= 2,
                   "a ring needs at least two slots to distinguish states");
    static_assert ((Capacity & (Capacity - 1)) == 0,
                   "ring capacity must be a power of two");

    bool
    push (const input_frame& f) noexcept
    {
      const size_t h (head_.load (std::memory_order_relaxed));
      const size_t t (tail_.load (std::memory_order_acquire));

      if (h - t >= Capacity)
      {
        dropped_.fetch_add (1, std::memory_order_relaxed);
        return false;
      }

      assert (f.sequence > last_sequence_ || h == 0);
      last_sequence_ = f.sequence;

      buf_[h & mask] = f;
      head_.store (h + 1, std::memory_order_release);
      return true;
    }

    bool
    try_pop (input_frame& out) noexcept
    {
      const size_t t (tail_.load (std::memory_order_relaxed));
      const size_t h (head_.load (std::memory_order_acquire));

      if (t == h)
        return false;

      out = buf_[t & mask];
      tail_.store (t + 1, std::memory_order_release);
      return true;
    }

    size_t
    occupancy () const noexcept
    {
      return head_.load (std::memory_order_acquire) -
             tail_.load (std::memory_order_acquire);
    }

    uint64_t
    dropped () const noexcept
    {
      return dropped_.load (std::memory_order_relaxed);
    }

  private:
    static constexpr size_t mask {Capacity - 1};

    std::array<input_frame, Capacity> buf_ {};

    std::atomic<size_t> head_ {0};
    std::atomic<size_t> tail_ {0};
    std::atomic<uint64_t> dropped_ {0};

    uint64_t last_sequence_ {0};
  };
}
