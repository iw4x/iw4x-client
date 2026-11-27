#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <optional>

#include <vkd3d_shader.h>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>

namespace Vulkan
{
  // Return the Vulkan format for the vertex element type, nullopt if there
  // is none.
  //
  std::optional<VkFormat>
  vk_vertex_format (d3d9::decl_type);

  // Return true if this is the declaration's end marker (D3DDECL_END).
  //
  inline bool
  vk_end_element (const d3d9::vertex_element& e) noexcept
  {
    return e.stream == 0xff && e.type == d3d9::decltype_unused;
  }

  // Return the element's format. Throw invalid_argument if the type is not
  // supported and if the stream is out of range.
  //
  VkFormat
  vk_vertex_element_format (const d3d9::vertex_element&);

  // Bindings 0-15 are the Direct3D 9 streams. The default binding reads
  // the same value for every vertex and feeds the shader inputs that the
  // declaration lacks.
  //
  const std::uint32_t vk_vertex_streams (16);
  const std::uint32_t vk_default_binding (vk_vertex_streams);
  const VkFormat vk_default_format (VK_FORMAT_R32G32B32A32_SFLOAT);

  struct vk_vertex_input
  {
    std::vector<VkVertexInputAttributeDescription> attributes;

    std::uint32_t bindings = 0; // One bit per binding that is used.
  };

  // Return the vertex attributes that feed the shader inputs from the
  // declaration elements, matched by semantic. The elements may include the
  // end marker. Throw invalid_argument if a matched element is not
  // supported.
  //
  vk_vertex_input
  vk_match_vertex_input (
    std::span<const d3d9::vertex_element> elements,
    std::span<const vkd3d_shader_signature_element> inputs);
}
