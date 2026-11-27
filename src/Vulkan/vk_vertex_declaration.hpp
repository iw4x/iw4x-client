#pragma once

#include <span>
#include <vector>

#include <Vulkan/vk_object.hpp>
#include <Vulkan/vk_vertex_input.hpp>
#include <Vulkan/vk_d3d9.hpp>

namespace Vulkan
{
  // Direct3D 9 vertex declaration (IDirect3DVertexDeclaration9).
  //
  // The declaration only keeps the elements. The Vulkan vertex attributes
  // also depend on the vertex shader inputs, so they are matched when we
  // build the pipeline (see vk_match_vertex_input()).
  //
  class vk_vertex_declaration: public vk_object
  {
  public:
    // The elements end with D3DDECL_END. Throw invalid_argument if an
    // element is not supported.
    //
    vk_vertex_declaration (vk_device&, const d3d9::vertex_element*);

    // Including the end marker.
    //
    std::span<const d3d9::vertex_element>
    elements () const noexcept {return elements_;}

  private:
    std::vector<d3d9::vertex_element> elements_;
  };
}
