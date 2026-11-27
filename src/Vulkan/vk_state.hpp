#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_shader_translate.hpp> // vk_alpha_test

namespace Vulkan
{
  // Direct3D 9 render and sampler states and their translation to Vulkan
  // pipeline and sampler state.
  //
  // The states are kept as Direct3D 9 sets them, that is, as DWORDs
  // indexed by the state type, and only translated when we build a
  // pipeline (sampler).
  //

  class vk_render_states
  {
  public:
    // Initialize the states to the Direct3D 9 defaults. Depth testing is
    // only enabled if there is a depth buffer.
    //
    explicit
    vk_render_states (bool depth_buffer);

    std::uint32_t
    operator[] (d3d9::render_state_type s) const noexcept {return values_[s];}

    std::uint32_t&
    operator[] (d3d9::render_state_type s) noexcept {return values_[s];}

    // Return the state as a float, for the states that Direct3D 9 passes as
    // float bits (depth bias, point size, and so on).
    //
    float
    real (d3d9::render_state_type) const noexcept;

    static const std::size_t size = 256;

  private:
    std::array<std::uint32_t, size> values_;
  };

  class vk_sampler_states
  {
  public:
    // Initialize the states to the Direct3D 9 defaults.
    //
    vk_sampler_states ();

    std::uint32_t
    operator[] (d3d9::sampler_state_type s) const noexcept {return values_[s];}

    std::uint32_t&
    operator[] (d3d9::sampler_state_type s) noexcept {return values_[s];}

    static const std::size_t size = 14;

  private:
    std::array<std::uint32_t, size> values_;
  };

  // Return the Vulkan equivalent. If the target has no alpha channel, then
  // the destination alpha factors use 1, as in Direct3D 9.
  //
  VkBlendFactor
  vk_blend_factor (d3d9::blend, bool alpha_target);

  VkBlendOp
  vk_blend_op (d3d9::blend_op);

  // Return the blend state for the render target with the index (0-3),
  // which selects the color write mask.
  //
  VkPipelineColorBlendAttachmentState
  vk_blend_attachment (const vk_render_states&,
                       std::uint32_t target,
                       bool alpha_target);

  // Return the blend factor state as RGBA.
  //
  std::array<float, 4>
  vk_blend_constants (const vk_render_states&);

  VkCompareOp
  vk_compare_op (d3d9::cmp_func);

  VkStencilOp
  vk_stencil_op (d3d9::stencil_op);

  VkCullModeFlags
  vk_cull_mode (d3d9::cull);

  // Direct3D 9 treats clockwise triangles as front-facing.
  //
  const VkFrontFace vk_front_face (VK_FRONT_FACE_CLOCKWISE);

  struct vk_stencil
  {
    bool enable;
    VkStencilOpState front;
    VkStencilOpState back;
  };

  // Return the stencil state. Without the two-sided stencil mode the back
  // faces get the front face state.
  //
  vk_stencil
  vk_stencil_state (const vk_render_states&);

  // Return the constant and slope-scaled depth bias for the depth buffer
  // format, the constant one converted to Vulkan units.
  //
  std::array<float, 2>
  vk_depth_bias (const vk_render_states&, VkFormat depth);

  // Return the alpha test for vk_translate_pixel(). If the test is
  // disabled, then the function is always.
  //
  vk_alpha_test
  vk_alpha_test_state (const vk_render_states&);

  // The topology is dynamic state, but Vulkan only lets it change within a
  // class. So the pipeline key includes the class (see vk_pipeline_key).
  //
  enum class vk_topology_class
  {
    point,
    line,
    triangle
  };

  VkPrimitiveTopology
  vk_topology (d3d9::primitive_type);

  vk_topology_class
  vk_topology_class_of (d3d9::primitive_type);

  // Return the number of vertices that the primitive count takes.
  //
  std::uint32_t
  vk_vertex_count (d3d9::primitive_type, std::uint32_t primitives);

  // Sampler states that affect the Vulkan sampler, for the sampler cache
  // (see vk_sampler_cache). The byte states saturate.
  //
  struct vk_sampler_key
  {
    std::uint8_t address_u;
    std::uint8_t address_v;
    std::uint8_t address_w;
    std::uint8_t mag_filter;
    std::uint8_t min_filter;
    std::uint8_t mip_filter;
    std::uint8_t max_anisotropy;
    std::uint8_t max_mip_level;
    std::uint32_t border_color;
    float lod_bias;

    bool compare; // Shadow sampler (see vk_translate_pixel()).

    friend bool
    operator== (const vk_sampler_key&, const vk_sampler_key&) = default;
  };

  // Return the key without compare, which is not a sampler state.
  //
  vk_sampler_key
  vk_sampler_key_of (const vk_sampler_states&);

  // Return the sampler create info. The anisotropy is clamped to the
  // device's maximum, with 1 meaning no anisotropic filtering.
  //
  VkSamplerCreateInfo
  vk_sampler_info (const vk_sampler_key&, float max_anisotropy);
}
