#include <Vulkan/vk_object.hpp>

#include <cassert>

#include <Vulkan/vk_device.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    // We identify objects in the pipeline cache keys by id (see
    // vk_context.cpp). A pointer won't do there since a deleted object's
    // address can be reused by a new object, which would then match the old
    // object's pipelines. With 64 bits we will never run out of ids, so
    // there is no reuse.
    //
    atomic<uint64_t> next_id (1);
  }

  vk_object::
  vk_object (vk_device& d) noexcept
    : device_ (d), id_ (next_id.fetch_add (1, memory_order_relaxed))
  {
  }

  vk_object::
  ~vk_object ()
  {
  }

  uint32_t vk_object::
  add_ref () noexcept
  {
    return refs_.fetch_add (1, memory_order_relaxed) + 1;
  }

  uint32_t vk_object::
  release () noexcept
  {
    uint32_t n (refs_.fetch_sub (1, memory_order_acq_rel) - 1);
    assert (n != ~0u);

    // Releasing the last reference doesn't mean that the GPU is done with
    // the object. Commands that use it may still be recorded in the current
    // frame and the previous frames may still be in flight. So we defer the
    // deletion until all of them complete (see vk_device::retire()).
    //
    if (n == 0)
      device_.retire ([this] () {delete this;});

    return n;
  }
}
