#pragma once

#include <span>
#include <vector>
#include <format>
#include <string>
#include <string_view>
#include <cstdint>
#include <utility>      // forward()
#include <system_error>

#include <vkd3d_shader.h>

#include <Vulkan/vk_d3dbc.hpp>

namespace Vulkan
{
  // Direct3D 9 shader bytecode translation to SPIR-V with vkd3d-shader.
  //

  enum class vk_shader_stage
  {
    vertex,
    pixel
  };

  const char*
  to_string (vk_shader_stage) noexcept;

  // Direct3D 9 constant register sets. The values are the constant buffer
  // indexes that vkd3d-shader uses for the sets.
  //
  enum class vk_constant_set: std::uint32_t
  {
    floats = VKD3D_SHADER_D3DBC_FLOAT_CONSTANT_REGISTER,
    ints   = VKD3D_SHADER_D3DBC_INT_CONSTANT_REGISTER,
    bools  = VKD3D_SHADER_D3DBC_BOOL_CONSTANT_REGISTER
  };

  const std::uint32_t vk_pixel_samplers (16); // As in Direct3D 9.
  const std::uint32_t vk_vertex_samplers (4);

  // Return the descriptor binding in the push descriptor set that all the
  // pipelines share (see vk_pipeline_cache). The layout is as follows:
  //
  // vertex shader constants   0-2
  // pixel shader constants    3-5
  // pixel shader samplers    16-31
  // vertex shader samplers   32-35
  //
  constexpr std::uint32_t
  vk_constant_binding (vk_shader_stage s, vk_constant_set c)
  {
    return (s == vk_shader_stage::pixel ? 3 : 0) +
           static_cast<std::uint32_t> (c);
  }

  constexpr std::uint32_t
  vk_sampler_binding (vk_shader_stage s, std::uint32_t sampler)
  {
    return (s == vk_shader_stage::pixel ? 16 : 32) + sampler;
  }

  // Return the error category for the vkd3d-shader result codes.
  //
  const std::error_category&
  vkd3d_category () noexcept;

  // Shader translation failure with the vkd3d-shader result as the error
  // code.
  //
  class vk_shader_error: public std::system_error
  {
  public:
    template <std::formattable<char>... A>
    vk_shader_error (int r, std::format_string<A...> f, A&&... a)
      : std::system_error (r,
                           vkd3d_category (),
                           std::format (f, std::forward<A> (a)...)) {}
  };

  // vkd3d-shader diagnostics, which a call returns through out() and the
  // destructor frees.
  //
  class vk_shader_messages
  {
  public:
    vk_shader_messages () = default;

    vk_shader_messages (const vk_shader_messages&) = delete;
    vk_shader_messages& operator= (const vk_shader_messages&) = delete;

    ~vk_shader_messages () {vkd3d_shader_free_messages (p_);}

    char**
    out () noexcept {return &p_;}

    const char*
    get () const noexcept {return p_;}

    // Return the diagnostics without the trailing newlines, "no
    // diagnostics" if there are none.
    //
    std::string_view
    text () const noexcept;

  private:
    char* p_ = nullptr;
  };

  // Owning wrapper for a vkd3d-shader signature, that is, the declared
  // shader inputs (outputs).
  //
  class vk_signature
  {
  public:
    vk_signature () noexcept: s_ {} {}

    explicit
    vk_signature (const vkd3d_shader_signature& s) noexcept: s_ (s) {}

    vk_signature (vk_signature&&) noexcept;
    vk_signature& operator= (vk_signature&&) noexcept;

    vk_signature (const vk_signature&) = delete;
    vk_signature& operator= (const vk_signature&) = delete;

    ~vk_signature ();

    const vkd3d_shader_signature&
    get () const noexcept {return s_;}

    std::span<const vkd3d_shader_signature_element>
    elements () const noexcept {return {s_.elements, s_.element_count};}

  private:
    vkd3d_shader_signature s_;
  };

  struct vk_shader_signatures
  {
    vk_signature input;
    vk_signature output;
  };

  // Return the shader's input and output signatures. Throw vk_shader_error
  // on failure.
  //
  vk_shader_signatures
  vk_scan_signatures (std::span<const std::uint32_t> code);

  using vk_varying_map = std::vector<vkd3d_shader_varying_map>;

  // Return the map that links the vertex shader's outputs to the pixel
  // shader's inputs (see vk_translate_vertex()).
  //
  vk_varying_map
  vk_link_varyings (const vk_signature& output, const vk_signature& input);

  // Alpha test for vk_translate_pixel(), which builds it into the shader
  // since Vulkan has none. The always function means no test.
  //
  struct vk_alpha_test
  {
    vkd3d_shader_comparison_func func = VKD3D_SHADER_COMPARISON_FUNC_ALWAYS;
    float ref = 0.0f;

    friend bool
    operator== (const vk_alpha_test&, const vk_alpha_test&) = default;
  };

  // Compile the HLSL source for the profile (for example, vs_3_0) to
  // Direct3D 9 bytecode. Throw vk_shader_error on failure.
  //
  std::vector<std::uint32_t>
  vk_compile_hlsl (std::string_view source, const char* profile);

  // Translate the vertex shader, placing its outputs at the locations that
  // the varying map assigns. Throw vk_shader_error on failure.
  //
  std::vector<std::uint32_t>
  vk_translate_vertex (std::span<const std::uint32_t> code,
                       const vk_d3dbc_info&,
                       const vk_varying_map&);

  // Translate the pixel shader with the alpha test built in. The shadow
  // samplers (one bit per sampler) compare with the reference value, as a
  // depth texture does in Direct3D 9. Throw vk_shader_error on failure.
  //
  std::vector<std::uint32_t>
  vk_translate_pixel (std::span<const std::uint32_t> code,
                      const vk_d3dbc_info&,
                      const vk_alpha_test&,
                      std::uint32_t shadow_samplers);
}
