#include <Vulkan/vk_sampler_cache.hpp>

#include <algorithm> // ranges::find()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device.hpp>

using namespace std;

namespace Vulkan
{
  vk_sampler_cache::
  vk_sampler_cache (vk_device& d)
    : device_ (d), max_anisotropy_ (d.limits ().maxSamplerAnisotropy)
  {
  }

  vk_sampler_cache::
  ~vk_sampler_cache ()
  {
    for (const auto& s: samplers_)
      vkDestroySampler (device_.handle (), s.second, nullptr);
  }

  VkSampler vk_sampler_cache::
  get (const vk_sampler_key& k)
  {
    // IW4 only uses a handful of distinct sampler states, so a linear search
    // should be good enough.
    //
    auto i (ranges::find (samplers_,
                          k,
                          &pair<vk_sampler_key, VkSampler>::first));

    if (i != samplers_.end ())
      return i->second;

    // Note that we never destroy a sampler before the cache. The key has a
    // limited number of values and Vulkan guarantees at least 4000 samplers
    // (maxSamplerAllocationCount).
    //
    VkSamplerCreateInfo ci (vk_sampler_info (k, max_anisotropy_));

    VkSampler r;
    vk_check (vkCreateSampler (device_.handle (), &ci, nullptr, &r),
              "unable to create sampler");

    samplers_.emplace_back (k, r);
    return r;
  }
}
