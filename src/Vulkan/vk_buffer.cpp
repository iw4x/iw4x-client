#include <Vulkan/vk_buffer.hpp>

#include <cstring> // memcpy()
#include <utility> // exchange()

using namespace std;

namespace Vulkan
{
  namespace
  {
    // @@ Nothing copies to vertex/index buffers currently. Can't we drop
    //    the transfer usage? Probably harmless to keep, though.
    //
    const VkBufferUsageFlags usage (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  }

  // Static buffers prefer device-local memory while dynamic buffers prefer
  // cached system memory. IW4 rewrites its dynamic buffers every frame and
  // CPU writes into device-local memory (through the BAR window) seem to be
  // much slower than the GPU reading system memory, which it only does about
  // once per write anyway.
  //
  vk_buffer::
  vk_buffer (vk_device& d, uint32_t size, uint32_t u, kind k)
    : vk_object (d),
      size_ (size),
      kind_ (k),
      preferred_ ((u & d3d9::usage_dynamic) != 0
                  ? VK_MEMORY_PROPERTY_HOST_CACHED_BIT
                  : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
  {
    auto [b, m] = d.create_buffer (size_,
                                   usage,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   preferred_);
    buffer_ = b;
    memory_ = m;
  }

  vk_buffer::
  ~vk_buffer ()
  {
    device ().destroy_buffer (buffer_, memory_);
  }

  void vk_buffer::
  rename (bool preserve)
  {
    vk_device& d (device ());

    auto [b, m] = d.create_buffer (size_,
                                   usage,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   preferred_);
    if (preserve)
      memcpy (m.mapped, memory_.mapped, size_);

    VkBuffer ob (exchange (buffer_, b));
    vk_allocation om (exchange (memory_, m));

    // The GPU may still be reading the old memory, so let the device destroy
    // it once the frames that use it complete (see vk_device::retire()).
    //
    d.retire ([&d, ob, om] () {d.destroy_buffer (ob, om);});

    frame_ = 0; // Not used by any frame yet.
  }

  void* vk_buffer::
  lock (uint32_t offset, uint32_t, uint32_t flags)
  {
    // See if the GPU may still be reading the buffer. Note that this includes
    // the frame being recorded, which is not submitted yet.
    //
    bool used (frame_ > device ().completed_frame ());

    // If so, then writing in place is only safe with d3d9::lock_nooverwrite,
    // where the caller promises to leave the data in use alone. Otherwise we
    // rename the buffer and, since the lock may only cover part of it, copy
    // the current contents over (there is nothing to copy if the caller
    // discards them with d3d9::lock_discard).
    //
    // Note that this copies the whole buffer for what may be a small write.
    // IW4 doesn't seem to hit this case since it locks its dynamic buffers
    // with discard once per frame and then appends with no-overwrite.
    //
    if (used && !(flags & d3d9::lock_nooverwrite))
      rename ((flags & d3d9::lock_discard) == 0);

    return static_cast<unsigned char*> (memory_.mapped) + offset;
  }
}
