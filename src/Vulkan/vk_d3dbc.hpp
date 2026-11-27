#pragma once

#include <span>
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Vulkan
{
  // Here and below d3dbc refers to Direct3D 9 shader bytecode. We scan it
  // for what the pipeline needs to know and rewrite it before passing it to
  // vkd3d-shader. See the implementation for the bytecode format and for
  // why each rewrite is needed.
  //

  // Sampler texture type as declared by the shader, none if not declared.
  //
  enum class vk_sampler_type: std::uint8_t
  {
    none,
    plane,
    cube,
    volume
  };

  // Shader information as returned by vk_d3dbc_scan().
  //
  struct vk_d3dbc_info
  {
    bool pixel = false;      // Vertex shader if false.
    std::uint32_t major = 0; // Shader model version.
    std::uint32_t minor = 0;

    std::size_t size = 0; // In bytes, end token included.

    // Constant register counts (highest register used plus one). If
    // relative is true, then floats is the whole set since the shader may
    // read any float constant.
    //
    std::uint32_t floats = 0;
    std::uint32_t ints = 0;
    std::uint32_t bools = 0;
    bool relative = false;

    std::uint32_t samplers = 0; // Bitmask of declared samplers.

    bool position = false; // True if vPos is read.
    bool face = false;     // True if vFace is read.

    std::array<vk_sampler_type, 16> sampler_types {}; // By sampler number.
  };

  // Scan the bytecode reading at most limit tokens. Return nullopt if the
  // bytecode is invalid, which includes not ending within the limit.
  //
  // Note that only shader models 2 and 3 are supported.
  //
  std::optional<vk_d3dbc_info>
  vk_d3dbc_scan (const std::uint32_t* tokens, std::size_t limit);

  // Return the bytecode up to and including the end token, nullopt if it is
  // invalid.
  //
  // Note that the scan is unbounded, so use this only for bytecode that
  // comes without its size (as in CreateVertexShader()).
  //
  std::optional<std::span<const std::uint32_t>>
  vk_d3dbc_code (const std::uint32_t* tokens);

  // Result of vk_d3dbc_lower_misc().
  //
  struct vk_d3dbc_lowered
  {
    std::vector<std::uint32_t> code;
    std::optional<std::uint32_t> position; // vPos input register, if read.
    std::optional<std::uint32_t> face;     // vFace input register, if read.
  };

  // Move the pixel shader's vPos and vFace to free input registers declared
  // as texcoords. Return nullopt if there are not enough free registers and
  // usage indexes.
  //
  // Note that the translated SPIR-V still has to be patched to read the
  // actual built-ins (see vk_spirv::lower_position() and lower_face()).
  //
  std::optional<vk_d3dbc_lowered>
  vk_d3dbc_lower_misc (std::span<const std::uint32_t> code);

  // Expand the Direct3D 9 macro instructions (m4x4, lit, etc) into simpler
  // ones and make log, rsq, and pow take the source's absolute value.
  // Return the bytecode unchanged if there is nothing to expand and nullopt
  // if there are not enough temporary registers for the expansions.
  //
  std::optional<std::vector<std::uint32_t>>
  vk_d3dbc_expand (std::span<const std::uint32_t> code);

  // Rewrite the source swizzles of componentwise instructions with a partial
  // write mask so that the masked out components are not read.
  //
  std::vector<std::uint32_t>
  vk_d3dbc_mask_inputs (std::span<const std::uint32_t> code);

  // Return the bytecode with the comment blocks (constant table, etc)
  // removed.
  //
  std::vector<std::uint32_t>
  vk_d3dbc_strip_comments (std::span<const std::uint32_t> code);
}
