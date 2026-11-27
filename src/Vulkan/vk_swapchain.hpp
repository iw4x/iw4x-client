#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <utility> // pair

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_texture.hpp>

namespace Vulkan
{
  class vk_device;
  class vk_context;

  // Vulkan swapchain with a Direct3D 9 back buffer.
  //
  // IW4 renders into the back buffer, which is an ordinary render target
  // that it can also read back as in Direct3D 9. On present we copy it into
  // the swapchain image, scaling it if the window size differs. So the back
  // buffer size is what IW4 asks for and the swapchain follows the window.
  //
  // If the window is minimized, then there is no swapchain and present()
  // only submits the frame.
  //
  class vk_swapchain
  {
  public:
    // The back buffer size and format. Note that the window size can still
    // differ (see above).
    //
    struct options
    {
      std::uint32_t width;
      std::uint32_t height;
      d3d9::format format = d3d9::fmt_x8r8g8b8;
      bool vsync = true;
    };

    // Throw vk_error on failure.
    //
    vk_swapchain (vk_context&, const options&);

    ~vk_swapchain ();

    vk_swapchain (const vk_swapchain&) = delete;
    vk_swapchain& operator= (const vk_swapchain&) = delete;

    const options&
    settings () const noexcept {return options_;}

    vk_surface&
    back_buffer () const noexcept {return *back_buffer_;}

    // Submit the frame and present the back buffer. Throw vk_error on
    // failure, except for an out of date swapchain, which is recreated on
    // the next call.
    //
    void
    present ();

    // Recreate the swapchain and the back buffer with the new options, as
    // IDirect3DDevice9::Reset() does. Note that this invalidates the
    // reference that back_buffer() returned.
    //
    void
    reset (const options&);

  private:
    bool
    create_swapchain ();

    void
    destroy_swapchain () noexcept;

    void
    create_back_buffer ();

    std::size_t
    acquire_semaphore ();

  private:
    vk_context& context_;
    vk_device& device_;
    options options_;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_ {};
    bool outdated_ = false;

    std::vector<VkImage> images_;
    std::vector<VkSemaphore> rendered_;

    std::vector<std::pair<VkSemaphore, std::uint64_t>> acquired_;
    std::size_t next_acquire_ = 0;

    vk_surface* back_buffer_ = nullptr;
  };
}
