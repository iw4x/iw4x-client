#pragma once

#include <map>
#include <cstdint>
#include <optional>

namespace Vulkan
{
  // Allocator of ranges in a Vulkan memory block.
  //
  // vk_device allocates device memory in 64MB blocks and uses one of these
  // per block to hand out ranges (requests over 32MB get a dedicated
  // allocation). The allocator only deals with offsets, the memory itself
  // stays with vk_device, which also serializes the calls with its memory
  // mutex.
  //
  // @@ Allocation is first fit and vk_buffer reallocates dynamic buffers
  //    when renaming them, which with IW4 happens every frame. Could this
  //    fragment the blocks over a long session? Probably not much since the
  //    sizes repeat from frame to frame, but we haven't measured it.
  //
  // @@ An empty block is only freed with the device. Should we return it to
  //    Vulkan, maybe once it stays empty for a while? Feels like it would
  //    help after a map change.
  //
  class vk_block_allocator
  {
  public:
    explicit
    vk_block_allocator (std::uint64_t size);

    // Allocate a range of size bytes and return its offset, nullopt if there
    // is no room left. The alignment is expected to be a power of two.
    //
    std::optional<std::uint64_t>
    allocate (std::uint64_t size, std::uint64_t alignment);

    // Free the range at offset, which must have been allocated with the
    // same size.
    //
    void
    free (std::uint64_t offset, std::uint64_t size);

    // Statistics. Currently unused.
    //
    std::uint64_t
    size () const noexcept {return size_;}

    std::uint64_t
    used () const noexcept {return used_;}

    bool
    empty () const noexcept {return used_ == 0;}

    using ranges = std::map<std::uint64_t, std::uint64_t>;

    const ranges&
    free_ranges () const noexcept {return free_;}

  private:
    std::uint64_t size_;
    std::uint64_t used_ = 0;
    ranges free_;
  };
}
