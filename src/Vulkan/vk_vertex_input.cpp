#include <Vulkan/vk_vertex_input.hpp>

#include <format>
#include <iterator>   // size()
#include <algorithm>  // ranges::{find_if,equal}()
#include <stdexcept>  // invalid_argument
#include <functional> // ranges::equal_to
#include <string_view>

using namespace std;

namespace Vulkan
{
  optional<VkFormat>
  vk_vertex_format (d3d9::decl_type t)
  {
    // Note that Direct3D 9 passes the unnormalized integer types to the
    // shader as floats, thus the scaled formats.
    //
    // @@ Vulkan doesn't require the scaled formats for vertex buffers,
    //    does it? If so, then we may need to convert on devices that lack
    //    them.
    //
    switch (t)
    {
      case d3d9::decltype_float1:    return VK_FORMAT_R32_SFLOAT;
      case d3d9::decltype_float2:    return VK_FORMAT_R32G32_SFLOAT;
      case d3d9::decltype_float3:    return VK_FORMAT_R32G32B32_SFLOAT;
      case d3d9::decltype_float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;

      // D3DCOLOR is BGRA in memory and reaches the shader as RGBA.
      //
      case d3d9::decltype_d3dcolor:  return VK_FORMAT_B8G8R8A8_UNORM;

      case d3d9::decltype_ubyte4:    return VK_FORMAT_R8G8B8A8_USCALED;
      case d3d9::decltype_ubyte4n:   return VK_FORMAT_R8G8B8A8_UNORM;
      case d3d9::decltype_short2:    return VK_FORMAT_R16G16_SSCALED;
      case d3d9::decltype_short4:    return VK_FORMAT_R16G16B16A16_SSCALED;
      case d3d9::decltype_short2n:   return VK_FORMAT_R16G16_SNORM;
      case d3d9::decltype_short4n:   return VK_FORMAT_R16G16B16A16_SNORM;
      case d3d9::decltype_ushort2n:  return VK_FORMAT_R16G16_UNORM;
      case d3d9::decltype_ushort4n:  return VK_FORMAT_R16G16B16A16_UNORM;

      // Direct3D 9 packs x into the low bits, as does A2B10G10R10.
      //
      case d3d9::decltype_udec3:
        return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
      case d3d9::decltype_dec3n:
        return VK_FORMAT_A2B10G10R10_SNORM_PACK32;

      case d3d9::decltype_float16_2: return VK_FORMAT_R16G16_SFLOAT;
      case d3d9::decltype_float16_4: return VK_FORMAT_R16G16B16A16_SFLOAT;

      default:                       return nullopt;
    }
  }

  VkFormat
  vk_vertex_element_format (const d3d9::vertex_element& e)
  {
    optional<VkFormat> r (
      vk_vertex_format (static_cast<d3d9::decl_type> (e.type)));

    if (!r)
      throw invalid_argument (
        format ("unknown vertex element type {}", e.type));

    if (e.stream >= vk_vertex_streams)
      throw invalid_argument (
        format ("vertex element stream {} out of range", e.stream));

    return *r;
  }

  namespace
  {
    // Semantic names, indexed by the Direct3D 9 usage.
    //
    const char* const usages[] =
    {
      "POSITION",     // d3d9::declusage_position
      "BLENDWEIGHT",  // d3d9::declusage_blendweight
      "BLENDINDICES", // d3d9::declusage_blendindices
      "NORMAL",       // d3d9::declusage_normal
      "PSIZE",        // d3d9::declusage_psize
      "TEXCOORD",     // d3d9::declusage_texcoord
      "TANGENT",      // d3d9::declusage_tangent
      "BINORMAL",     // d3d9::declusage_binormal
      "TESSFACTOR",   // d3d9::declusage_tessfactor
      "POSITIONT",    // d3d9::declusage_positiont
      "COLOR",        // d3d9::declusage_color
      "FOG",          // d3d9::declusage_fog
      "DEPTH",        // d3d9::declusage_depth
      "SAMPLE"        // d3d9::declusage_sample
    };

    char
    upper (char c) noexcept
    {
      return c >= 'a' && c <= 'z' ? static_cast<char> (c - 'a' + 'A') : c;
    }

    // Return true if the semantic name is for the usage. Note that the
    // comparison is case-insensitive since HLSL semantics are.
    //
    bool
    usage_of (string_view name, uint32_t usage)
    {
      return usage < size (usages) &&
             ranges::equal (name, string_view (usages[usage]),
                            ranges::equal_to {}, &upper);
    }
  }

  vk_vertex_input
  vk_match_vertex_input (span<const d3d9::vertex_element> elements,
                         span<const vkd3d_shader_signature_element> inputs)
  {
    span<const d3d9::vertex_element> es (elements.begin (),
                                         ranges::find_if (elements,
                                                          &vk_end_element));

    vk_vertex_input r;

    // Match each shader input with the element that has the same semantic.
    // Direct3D 9 drivers tolerate a declaration that lacks an input, so we
    // bind such an input to the default binding, which has a zero stride
    // and so the same value for every vertex (see vk_context).
    //
    for (const vkd3d_shader_signature_element& i: inputs)
    {
      auto input_element = [&i] (const d3d9::vertex_element& e)
      {
        return e.usage_index == i.semantic_index &&
               usage_of (i.semantic_name, e.usage);
      };

      auto e (ranges::find_if (es, input_element));

      VkVertexInputAttributeDescription a {};
      a.location = i.register_index;

      if (e == es.end ())
      {
        a.binding = vk_default_binding;
        a.format = vk_default_format;
        a.offset = 0;
      }
      else
      {
        a.binding = e->stream;
        a.format = vk_vertex_element_format (*e);
        a.offset = e->offset;
      }

      r.attributes.push_back (a);
      r.bindings |= 1u << a.binding;
    }

    return r;
  }
}
