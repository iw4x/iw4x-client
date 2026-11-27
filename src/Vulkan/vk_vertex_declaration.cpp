#include <Vulkan/vk_vertex_declaration.hpp>

using namespace std;

namespace Vulkan
{
  vk_vertex_declaration::
  vk_vertex_declaration (vk_device& d, const d3d9::vertex_element* es)
    : vk_object (d)
  {
    // Copy the elements, including the end marker (D3DDECL_END), and
    // validate each. This way a bad declaration fails here, as it does in
    // Direct3D 9, and not later when we build the pipeline.
    //
    for (;; ++es)
    {
      const d3d9::vertex_element& e (*es);
      elements_.push_back (e);

      if (vk_end_element (e))
        break;

      vk_vertex_element_format (e); // Validate.
    }
  }
}
