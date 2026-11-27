#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <tuple>
#include <utility> // pair

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3dbc.hpp>
#include <Vulkan/vk_object.hpp>
#include <Vulkan/vk_shader_translate.hpp>

namespace Vulkan
{
  // Direct3D 9 shader (IDirect3DVertexShader9 and IDirect3DPixelShader9).
  //
  // In Vulkan the translated vertex shader depends on how its outputs link
  // to the pixel shader. The translated pixel shader depends on the alpha
  // test and the shadow samplers. So we keep the bytecode and translate it
  // on first use for each such combination (see module()).
  //
  class vk_shader: public vk_object
  {
  public:
    // Throw vk_shader_error if we can't translate the bytecode (see
    // vk_d3dbc_scan()) and if it is for the other stage.
    //
    vk_shader (vk_device&, vk_shader_stage, std::span<const std::uint32_t>);

    virtual
    ~vk_shader () override;

    vk_shader_stage
    stage () const noexcept {return stage_;}

    // Up to and including the end token.
    //
    std::span<const std::uint32_t>
    code () const noexcept {return code_;}

    const vk_d3dbc_info&
    info () const noexcept {return info_;}

    const vk_shader_signatures&
    signatures () const noexcept {return signatures_;}

    // Return the vertex shader module for the varying map (see
    // vk_link_varyings()), translating the shader if necessary. Throw
    // vk_shader_error and vk_error on failure.
    //
    VkShaderModule
    module (const vk_varying_map&);

    // As above but return the pixel shader module for the alpha test and the
    // shadow samplers (one bit per sampler).
    //
    VkShaderModule
    module (const vk_alpha_test&, std::uint32_t shadow_samplers);

  private:
    VkShaderModule
    create (const std::vector<std::uint32_t>& spirv);

  private:
    vk_shader_stage stage_;
    std::vector<std::uint32_t> code_;
    vk_d3dbc_info info_;
    vk_shader_signatures signatures_;

    std::vector<std::pair<vk_varying_map, VkShaderModule>> vertex_modules_;
    std::vector<std::tuple<vk_alpha_test, std::uint32_t, VkShaderModule>>
      pixel_modules_;
  };
}
