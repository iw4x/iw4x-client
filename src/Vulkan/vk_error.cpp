#include <Vulkan/vk_error.hpp>

#include <string>

using namespace std;

namespace Vulkan
{
  const char*
  to_string (VkResult r) noexcept
  {
    // We only name the core results that the renderer is likely to get back,
    // together with the surface and swapchain ones. Anything else comes out
    // as unknown.
    //
    switch (r)
    {
      case VK_SUCCESS:
        return "VK_SUCCESS";
      case VK_NOT_READY:
        return "VK_NOT_READY";
      case VK_TIMEOUT:
        return "VK_TIMEOUT";
      case VK_EVENT_SET:
        return "VK_EVENT_SET";
      case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
      case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
      case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
      case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
      case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
      case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
      case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
      default:
        return "unknown VkResult";
    }
  }

  namespace
  {
    class vk_category_impl: public error_category
    {
    public:
      virtual const char*
      name () const noexcept override
      {
        return "vulkan";
      }

      // Note that system_error normally appends this to the description, as
      // in:
      //
      // unable to create Vulkan instance: VK_ERROR_INCOMPATIBLE_DRIVER
      //
      virtual string
      message (int v) const override
      {
        return to_string (static_cast<VkResult> (v));
      }
    };
  }

  const error_category&
  vk_category () noexcept
  {
    static const vk_category_impl c;
    return c;
  }
}
