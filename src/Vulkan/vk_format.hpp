#pragma once

#include <cstdint>
#include <optional>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>

namespace Vulkan
{
  // Direct3D 9 format's Vulkan description as returned by vk_format().
  //
  struct vk_format_info
  {
    VkFormat format = VK_FORMAT_UNDEFINED;

    // Format to use if the device doesn't support the first one, undefined
    // if there is none.
    //
    VkFormat alternative = VK_FORMAT_UNDEFINED;

    // Image view swizzle that makes the format read as in Direct3D 9.
    //
    VkComponentMapping swizzle {};

    std::uint32_t block_size = 1;  // Block width and height in texels.
    std::uint32_t block_bytes = 0; // Block size in bytes.

    bool depth = false;
    bool stencil = false;

    // Return the image aspects that the format has.
    //
    VkImageAspectFlags
    aspect () const noexcept
    {
      return depth ? (VK_IMAGE_ASPECT_DEPTH_BIT |
                      (stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0))
                   : VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // Return the byte size of one block row for the width in texels.
    //
    std::uint32_t
    pitch (std::uint32_t width) const noexcept
    {
      return (width + block_size - 1) / block_size * block_bytes;
    }

    // Return the block row count for the height in texels.
    //
    std::uint32_t
    rows (std::uint32_t height) const noexcept
    {
      return (height + block_size - 1) / block_size;
    }
  };

  // INTZ, the FOURCC format that Direct3D 9 drivers provide for depth
  // textures that shaders can sample.
  //
  inline constexpr d3d9::format d3dfmt_intz (
    static_cast<d3d9::format> ('I' | 'N' << 8 | 'T' << 16 | 'Z' << 24));

  // Return the Direct3D 9 format's Vulkan description, nullopt if we don't
  // support the format.
  //
  std::optional<vk_format_info>
  vk_format (d3d9::format);

  // Return the format's sRGB variant, VK_FORMAT_UNDEFINED if it has none.
  //
  VkFormat
  vk_srgb_format (VkFormat);
}
