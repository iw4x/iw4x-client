#include <Vulkan/vk_query.hpp>

#include <cassert>

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device.hpp>

using namespace std;

namespace Vulkan
{
  vk_query::
  vk_query (vk_device& d, d3d9::query_type t)
    : vk_object (d), type_ (t)
  {
    assert (t == d3d9::querytype_occlusion || t == d3d9::querytype_event);

    // An event query is signaled once its frame completes (see
    // vk_context::issue()), so only an occlusion query needs a Vulkan query.
    //
    // @@ A pool per query feels wasteful. Maybe we should share one pool
    //    among all the occlusion queries?
    //
    if (t == d3d9::querytype_occlusion)
    {
      VkQueryPoolCreateInfo ci {};
      ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
      ci.queryType = VK_QUERY_TYPE_OCCLUSION;
      ci.queryCount = 1;

      vk_check (vkCreateQueryPool (d.handle (), &ci, nullptr, &pool_),
                "unable to create occlusion query pool");
    }
  }

  vk_query::
  ~vk_query ()
  {
    if (pool_ != VK_NULL_HANDLE)
      vkDestroyQueryPool (device ().handle (), pool_, nullptr);
  }
}
