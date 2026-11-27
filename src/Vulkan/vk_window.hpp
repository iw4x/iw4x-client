#pragma once

#include <vector>
#include <cstdint>
#include <utility>  // pair
#include <optional>

#include <Vulkan/vk.hpp>

namespace Vulkan
{
  // IW4's window, which we render into. Note that the window is not owned:
  // IW4 creates and destroys it.
  //
  class vk_window
  {
  public:
    explicit
    vk_window (HWND) noexcept;

    vk_window (const vk_window&) = delete;
    vk_window& operator= (const vk_window&) = delete;

    // Return the instance extensions that create_surface() needs.
    //
    std::vector<const char*>
    instance_extensions () const;

    // Throw vk_error on failure.
    //
    VkSurfaceKHR
    create_surface (VkInstance) const;

    // Return the client area width and height, nullopt if the area is empty
    // (for example, the window is minimized).
    //
    std::optional<std::pair<std::uint32_t, std::uint32_t>>
    client_size () const;

    HWND
    native () const noexcept {return native_;}

  private:
    HWND native_;
  };
}
