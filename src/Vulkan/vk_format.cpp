#include <Vulkan/vk_format.hpp>

using namespace std;

namespace Vulkan
{
  optional<vk_format_info>
  vk_format (d3d9::format f)
  {
    // Shorthands for the swizzle column below.
    //
    const VkComponentSwizzle r (VK_COMPONENT_SWIZZLE_R);
    const VkComponentSwizzle g (VK_COMPONENT_SWIZZLE_G);
    const VkComponentSwizzle i (VK_COMPONENT_SWIZZLE_IDENTITY);
    const VkComponentSwizzle o (VK_COMPONENT_SWIZZLE_ONE);
    const VkComponentSwizzle z (VK_COMPONENT_SWIZZLE_ZERO);

    struct entry
    {
      d3d9::format d3d;
      VkFormat format;
      VkFormat alternative;
      VkComponentMapping swizzle;
      uint32_t block_size;
      uint32_t block_bytes;
      bool depth;
      bool stencil;
    };

    const VkFormat none (VK_FORMAT_UNDEFINED);

    // The Direct3D 9 formats that IW4 uses and their Vulkan equivalents.
    //
    // Note that Direct3D 9 lists the components from a little-endian word's
    // most significant bit down while Vulkan lists the 8-bit components in
    // their memory order. So A8R8G8B8 is B8G8R8A8 in Vulkan. The packed
    // formats are named from the most significant bit in both APIs and so
    // map as is.
    //
    // Vulkan has no luminance and alpha-only formats. We store them in the
    // red and green channels and swizzle them into place. The swizzle also
    // makes the unused X8 component read as 1. Components that a format
    // doesn't have read as 1 as well, which is what Direct3D 9 returns for
    // them.
    //
    // For the depth formats there is a second Vulkan format that we use if
    // the device doesn't support the first one (see vk_texture.cpp). Vulkan
    // only guarantees that a device supports one of D24 with stencil and
    // D32F with stencil, so the two fall back to each other. INTZ is the
    // Direct3D 9 driver extension for sampling depth (see d3dfmt_intz) and
    // is just D24S8 for Vulkan.
    //
    static const entry table[]
    {
      {d3d9::fmt_a8r8g8b8,      VK_FORMAT_B8G8R8A8_UNORM,
       none,                         {i, i, i, i}, 1,  4, false, false},
      {d3d9::fmt_x8r8g8b8,      VK_FORMAT_B8G8R8A8_UNORM,
       none,                         {i, i, i, o}, 1,  4, false, false},
      {d3d9::fmt_a8b8g8r8,      VK_FORMAT_R8G8B8A8_UNORM,
       none,                         {i, i, i, i}, 1,  4, false, false},
      {d3d9::fmt_r5g6b5,        VK_FORMAT_R5G6B5_UNORM_PACK16,
       none,                         {i, i, i, o}, 1,  2, false, false},
      {d3d9::fmt_a1r5g5b5,      VK_FORMAT_A1R5G5B5_UNORM_PACK16,
       none,                         {i, i, i, i}, 1,  2, false, false},
      {d3d9::fmt_l8,            VK_FORMAT_R8_UNORM,
       none,                         {r, r, r, o}, 1,  1, false, false},
      {d3d9::fmt_a8l8,          VK_FORMAT_R8G8_UNORM,
       none,                         {r, r, r, g}, 1,  2, false, false},
      {d3d9::fmt_a8,            VK_FORMAT_R8_UNORM,
       none,                         {z, z, z, r}, 1,  1, false, false},
      {d3d9::fmt_g16r16,        VK_FORMAT_R16G16_UNORM,
       none,                         {r, g, o, o}, 1,  4, false, false},
      {d3d9::fmt_g16r16f,       VK_FORMAT_R16G16_SFLOAT,
       none,                         {r, g, o, o}, 1,  4, false, false},
      {d3d9::fmt_r32f,          VK_FORMAT_R32_SFLOAT,
       none,                         {r, o, o, o}, 1,  4, false, false},
      {d3d9::fmt_a16b16g16r16,  VK_FORMAT_R16G16B16A16_UNORM,
       none,                         {i, i, i, i}, 1,  8, false, false},
      {d3d9::fmt_a16b16g16r16f, VK_FORMAT_R16G16B16A16_SFLOAT,
       none,                         {i, i, i, i}, 1,  8, false, false},
      {d3d9::fmt_dxt1,          VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
       none,                         {i, i, i, i}, 4,  8, false, false},
      {d3d9::fmt_dxt3,          VK_FORMAT_BC2_UNORM_BLOCK,
       none,                         {i, i, i, i}, 4, 16, false, false},
      {d3d9::fmt_dxt5,          VK_FORMAT_BC3_UNORM_BLOCK,
       none,                         {i, i, i, i}, 4, 16, false, false},

      {d3d9::fmt_d24s8,         VK_FORMAT_D24_UNORM_S8_UINT,
       VK_FORMAT_D32_SFLOAT_S8_UINT, {i, i, i, i}, 1,  4, true,  true},
      {d3dfmt_intz,          VK_FORMAT_D24_UNORM_S8_UINT,
       VK_FORMAT_D32_SFLOAT_S8_UINT, {i, i, i, i}, 1,  4, true,  true},
      {d3d9::fmt_d24fs8,        VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT,  {i, i, i, i}, 1,  4, true,  true},
      {d3d9::fmt_d24x8,         VK_FORMAT_X8_D24_UNORM_PACK32,
       VK_FORMAT_D32_SFLOAT,         {i, i, i, i}, 1,  4, true,  false},
      {d3d9::fmt_d16,           VK_FORMAT_D16_UNORM,
       none,                         {i, i, i, i}, 1,  2, true,  false},
      {d3d9::fmt_d16_lockable,  VK_FORMAT_D16_UNORM,
       none,                         {i, i, i, i}, 1,  2, true,  false},
      {d3d9::fmt_d15s1,         VK_FORMAT_D16_UNORM_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT,  {i, i, i, i}, 1,  2, true,  true}
    };

    for (const entry& e: table)
    {
      if (e.d3d == f)
        return vk_format_info {e.format, e.alternative, e.swizzle,
                               e.block_size, e.block_bytes,
                               e.depth, e.stencil};
    }

    return nullopt;
  }

  VkFormat
  vk_srgb_format (VkFormat f)
  {
    // Direct3D 9 turns sRGB decoding on with a sampler state
    // (D3DSAMP_SRGBTEXTURE) for any texture with a suitable format. In
    // Vulkan the format itself says whether to decode, so a texture gets a
    // second view with the sRGB variant (see vk_texture.cpp).
    //
    switch (f)
    {
      case VK_FORMAT_B8G8R8A8_UNORM:      return VK_FORMAT_B8G8R8A8_SRGB;
      case VK_FORMAT_R8G8B8A8_UNORM:      return VK_FORMAT_R8G8B8A8_SRGB;
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
      case VK_FORMAT_BC2_UNORM_BLOCK:     return VK_FORMAT_BC2_SRGB_BLOCK;
      case VK_FORMAT_BC3_UNORM_BLOCK:     return VK_FORMAT_BC3_SRGB_BLOCK;
      default:                            return VK_FORMAT_UNDEFINED; // None.
    }
  }
}
