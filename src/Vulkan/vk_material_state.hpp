#pragma once

#include <span>
#include <cstdint>

#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_state.hpp>

namespace Vulkan
{
  // IW4 keeps a material's render state as packed bits (GfxStateBits) and
  // its sampler state as a byte per texture (MaterialTextureDef). Here we
  // translate both into the Direct3D 9 states that the context understands,
  // which should be what IW4 itself would set on a Direct3D 9 device.
  //

  // Depth bias for the polygon offset levels in the state bits. IW4 takes
  // it from the polygon offset dvars, with a separate pair for the shadow
  // map level (see Vulkan.cpp).
  //
  struct vk_polygon_offsets
  {
    float bias = 0.0f;
    float scale = 0.0f;
    float shadow_bias = 0.0f;
    float shadow_scale = 0.0f;
  };

  // Return the render states that vk_apply_state_bits() sets, which are the
  // ones that the caller should forward to the context.
  //
  std::span<const d3d9::render_state_type>
  vk_state_bits_states () noexcept;

  // Translate the state bits into render states. Note that only the states
  // returned by vk_state_bits_states() are set.
  //
  void
  vk_apply_state_bits (vk_render_states&,
                       std::uint32_t bits0,
                       std::uint32_t bits1,
                       const vk_polygon_offsets&);

  // Mipmapping override that the user can select. With material we use
  // what the sampler state says while the others force the mipmap filter.
  // Note that a mode never enables mipmapping for a texture that doesn't
  // use it.
  //
  enum class vk_mip_mode
  {
    material,
    linear,
    nearest,
    none
  };

  // Texture filtering settings that override the sampler state.
  //
  struct vk_texture_filtering
  {
    std::uint32_t anisotropy_min = 1; // Above 1 also upgrades linear filtering.
    std::uint32_t anisotropy_max = 1;
    vk_mip_mode mip = vk_mip_mode::material;
  };

  // Translate the sampler state byte into the filter and address sampler
  // states, adjusted according to the settings.
  //
  void
  vk_apply_sampler_state (vk_sampler_states&,
                          std::uint8_t state,
                          const vk_texture_filtering&);
}
