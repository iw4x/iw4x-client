#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Vulkan
{
  // Physical device description as used by vk_select_device().
  //
  // The caller fills it from the Vulkan physical device queries. Each
  // feature and extension member is true if the device supports it.
  //
  struct vk_device_candidate
  {
    // Device type (see VkPhysicalDeviceType).
    //
    enum class kind {other, integrated, discrete, virtual_gpu, cpu};

    std::string name; // Device name, used in diagnostics.
    kind type = kind::other;

    std::uint32_t api_version = 0; // Encoded with VK_MAKE_API_VERSION().

    // Vulkan 1.3 and 1.2 features.
    //
    bool dynamic_rendering = false;
    bool synchronization2 = false;
    bool timeline_semaphore = false;
    bool host_query_reset = false;

    // Vulkan 1.0 features.
    //
    bool texture_compression_bc = false;
    bool sampler_anisotropy = false;
    bool independent_blend = false;

    bool swapchain = false;       // VK_KHR_swapchain
    bool push_descriptor = false; // VK_KHR_push_descriptor

    // True if the device has a graphics queue family that can present to
    // the window surface.
    //
    bool present_queue = false;
  };

  // Return the first requirement that the device doesn't meet as a name
  // suitable for diagnostics (for example, "VK_KHR_swapchain"), nullopt if
  // the device meets all of them.
  //
  // The requirements are Vulkan 1.3, every feature and extension member,
  // and present_queue.
  //
  std::optional<std::string>
  vk_missing_requirement (const vk_device_candidate&);

  // Select the device to use and return its position in the list, nullopt
  // if no device meets the requirements (see vk_missing_requirement()).
  //
  // Discrete GPUs are preferred to integrated ones, which in turn are
  // preferred to the rest. Among equally ranked devices the first one in
  // the list is selected.
  //
  std::optional<std::size_t>
  vk_select_device (const std::vector<vk_device_candidate>&);
}
