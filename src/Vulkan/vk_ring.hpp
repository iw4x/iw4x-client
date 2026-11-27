#pragma once

#include <deque>
#include <cstdint>
#include <optional>

namespace Vulkan
{
  // Ring allocator for memory that the GPU only needs until the frame that
  // uses it completes, such as the shader constants and the vertices that
  // IW4 draws from memory. The ring only hands out offsets and the caller
  // owns the actual buffer.
  //
  // Each allocation is tagged with a serial (normally the frame number).
  // Once the serial completes, reclaim() returns its memory to the ring.
  // Note that this only works if the serials don't decrease since the
  // memory is reclaimed in the allocation order.
  //
  class vk_ring
  {
  public:
    explicit
    vk_ring (std::uint64_t size);

    // Return the offset of size bytes aligned to alignment (a power of two),
    // nullopt if the ring doesn't have that much free memory in one piece.
    //
    std::optional<std::uint64_t>
    allocate (std::uint64_t size,
              std::uint64_t alignment,
              std::uint64_t serial);

    // Free the memory of every serial up to and including completed.
    //
    void
    reclaim (std::uint64_t completed);

    std::uint64_t
    size () const noexcept {return size_;}

    bool
    empty () const noexcept {return spans_.empty ();}

  private:
    struct span
    {
      std::uint64_t serial;
      std::uint64_t end;
    };

    std::uint64_t size_;
    std::uint64_t head_ = 0;
    std::uint64_t tail_ = 0;
    std::deque<span> spans_;
  };
}
