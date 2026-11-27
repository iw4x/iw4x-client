#pragma once

#include <cstdint>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_object.hpp>

namespace Vulkan
{
  // Direct3D 9 vertex/index buffer.
  //
  // The buffer memory is host-visible and lock() returns a pointer into its
  // mapping. If the GPU may still be reading the buffer, then writing to it
  // in place is only safe if the caller promised not to overwrite anything
  // in use (d3d9::lock_nooverwrite). Otherwise we rename the buffer, that
  // is, give it new memory and retire the old one (see vk_object for
  // details). This is what Direct3D 9 drivers do and what IW4 expects.
  //
  // Note that we know the buffer may still be in use because the context
  // marks it with the frame being recorded whenever it binds the buffer
  // (see use()).
  //
  class vk_buffer: public vk_object
  {
  public:
    enum class kind {vertex, index16, index32};

    // Allocate size bytes of buffer memory. Only d3d9::usage_dynamic in
    // usage has an effect (see the implementation for details).
    //
    // Throw vk_error on failure.
    //
    vk_buffer (vk_device&, std::uint32_t size, std::uint32_t usage, kind);

    virtual
    ~vk_buffer () override;

    std::uint32_t
    size () const noexcept {return size_;}

    kind
    type () const noexcept {return kind_;}

    VkIndexType
    index_type () const noexcept
    {
      return kind_ == kind::index32 ? VK_INDEX_TYPE_UINT32
                                    : VK_INDEX_TYPE_UINT16;
    }

    // Lock the buffer for writing and return the pointer to its memory at
    // the specified offset. The flags are d3d9::lock_*. We ignore the size
    // since a rename that keeps the contents copies the whole buffer anyway
    // (see the implementation for details).
    //
    // Throw vk_error on failure.
    //
    void*
    lock (std::uint32_t offset, std::uint32_t size, std::uint32_t flags);

    void
    unlock () noexcept {} // Nothing to do since the memory is coherent.

    // Context interface.
    //
    VkBuffer
    handle () const noexcept {return buffer_;}

    // Mark the buffer as used by the frame.
    //
    void
    use (std::uint64_t frame) noexcept {frame_ = frame;}

  private:
    // Switch to new memory and retire the current one. If preserve is true,
    // then copy the contents over.
    //
    void
    rename (bool preserve);

  private:
    std::uint32_t size_;
    kind kind_;
    VkMemoryPropertyFlags preferred_; // See the constructor.

    VkBuffer buffer_ = VK_NULL_HANDLE;
    vk_allocation memory_;

    std::uint64_t frame_ = 0; // Last frame that used the buffer.
  };
}
