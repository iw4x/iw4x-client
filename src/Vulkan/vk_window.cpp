#include <Vulkan/vk_window.hpp>

#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <windows.h>

#include <Vulkan/vk_error.hpp>

using namespace std;

namespace Vulkan
{
  vk_window::
  vk_window (HWND w) noexcept
    : native_ (w)
  {
  }

  vector<const char*> vk_window::
  instance_extensions () const
  {
    return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
  }

  VkSurfaceKHR vk_window::
  create_surface (VkInstance i) const
  {
    VkWin32SurfaceCreateInfoKHR si {};
    si.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    si.hinstance = GetModuleHandleW (nullptr); // IW4's executable.
    si.hwnd = native_;

    VkSurfaceKHR r (VK_NULL_HANDLE);
    vk_check (vkCreateWin32SurfaceKHR (i, &si, nullptr, &r),
              "unable to create window surface");
    return r;
  }

  optional<pair<uint32_t, uint32_t>> vk_window::
  client_size () const
  {
    // Note that a minimized window has an empty client area, which we also
    // treat as no size.
    //
    RECT c;
    if (!GetClientRect (native_, &c) ||
        c.right <= 0                 ||
        c.bottom <= 0)
      return nullopt;

    return pair (static_cast<uint32_t> (c.right),
                 static_cast<uint32_t> (c.bottom));
  }
}
