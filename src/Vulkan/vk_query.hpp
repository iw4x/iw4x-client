#pragma once

#include <cstdint>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_object.hpp>

namespace Vulkan
{
  // Direct3D 9 query (IDirect3DQuery9). Only the occlusion and event types
  // are supported.
  //
  // Note that an event query is considered done once its frame completes,
  // which is coarser than in Direct3D 9 (see vk_context::issue()).
  //
  class vk_query: public vk_object
  {
  public:
    vk_query (vk_device&, d3d9::query_type);

    virtual
    ~vk_query () override;

    d3d9::query_type
    type () const noexcept {return type_;}

    // Null for an event query.
    //
    VkQueryPool
    pool () const noexcept {return pool_;}

    // Query states as in Direct3D 9 except that idle (never issued) stands
    // in for signaled.
    //
    enum class state {idle, building, issued};

    // Note that the query remains issued after its result is read.
    //
    state status = state::idle;

    // If the query is issued, then its result is available once this frame
    // completes.
    //
    std::uint64_t frame = 0;

    // Sample count from the last occlusion query that completed.
    //
    std::uint32_t result = 0;

  private:
    d3d9::query_type type_;
    VkQueryPool pool_ = VK_NULL_HANDLE;
  };
}
