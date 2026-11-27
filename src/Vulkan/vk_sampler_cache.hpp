#pragma once

#include <cstddef>
#include <utility> // pair
#include <vector>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_state.hpp>

namespace Vulkan
{
  class vk_device;

  // Vulkan samplers by key. A sampler is created on first use and lives as
  // long as the cache. Note that the anisotropy is clamped to the device's
  // limit (see vk_sampler_info()).
  //
  class vk_sampler_cache
  {
  public:
    explicit
    vk_sampler_cache (vk_device&);

    ~vk_sampler_cache ();

    vk_sampler_cache (const vk_sampler_cache&) = delete;
    vk_sampler_cache& operator= (const vk_sampler_cache&) = delete;

    // Return the key's sampler, creating it if necessary. Throw vk_error on
    // failure.
    //
    VkSampler
    get (const vk_sampler_key&);

    std::size_t
    size () const noexcept {return samplers_.size ();}

  private:
    vk_device& device_;
    float max_anisotropy_;
    std::vector<std::pair<vk_sampler_key, VkSampler>> samplers_;
  };
}
