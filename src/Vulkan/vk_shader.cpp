#include <Vulkan/vk_shader.hpp>

#include <cassert>
#include <optional>
#include <algorithm> // ranges::{find,find_if,equal}()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    // Return true if the two varying map entries are the same.
    //
    bool
    same (const vkd3d_shader_varying_map& x, const vkd3d_shader_varying_map& y)
    {
      return x.output_signature_index == y.output_signature_index &&
             x.input_register_index   == y.input_register_index   &&
             x.input_mask             == y.input_mask;
    }
  }

  vk_shader::
  vk_shader (vk_device& d, vk_shader_stage s, span<const uint32_t> ts)
    : vk_object (d), stage_ (s)
  {
    // Make sure the bytecode is something that we can translate and that it
    // is for this stage.
    //
    optional<vk_d3dbc_info> i (vk_d3dbc_scan (ts.data (), ts.size ()));

    if (!i)
      throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                             "{} shader is not shader model 2 or 3 bytecode",
                             to_string (s));

    if (i->pixel != (s == vk_shader_stage::pixel))
      throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                             "{} shader bytecode is of a {} shader",
                             to_string (s),
                             i->pixel ? "pixel" : "vertex");

    // Direct3D 9 doesn't require the caller to keep the bytecode after the
    // shader is created, so we make a copy. We need it later since we only
    // translate the shader once we know the pipeline that uses it (see
    // module()).
    //
    info_ = *i;
    code_.assign (ts.begin (), ts.begin () + info_.size / sizeof (uint32_t));
    signatures_ = vk_scan_signatures (code_);
  }

  vk_shader::
  ~vk_shader ()
  {
    VkDevice d (device ().handle ());

    for (const auto& m: vertex_modules_)
      vkDestroyShaderModule (d, m.second, nullptr);

    for (const auto& [a, s, m]: pixel_modules_)
      vkDestroyShaderModule (d, m, nullptr);
  }

  VkShaderModule vk_shader::
  module (const vk_varying_map& vm)
  {
    assert (stage_ == vk_shader_stage::vertex);

    // The translation depends on how the outputs are linked to the pixel
    // shader inputs, so we cache a module for each varying map. Normally
    // there are only a few per shader, so a linear search will do.
    //
    auto equal_map = [&vm] (const vk_varying_map& x)
    {
      return ranges::equal (x, vm, &same);
    };

    auto i (ranges::find_if (vertex_modules_,
                             equal_map,
                             &pair<vk_varying_map, VkShaderModule>::first));

    if (i != vertex_modules_.end ())
      return i->second;

    VkShaderModule r (create (vk_translate_vertex (code_, info_, vm)));
    vertex_modules_.emplace_back (vm, r);
    return r;
  }

  VkShaderModule vk_shader::
  module (const vk_alpha_test& at, uint32_t shadow_samplers)
  {
    assert (stage_ == vk_shader_stage::pixel);

    // As above but here the translation depends on the alpha test and the
    // shadow samplers.
    //
    for (const auto& [a, s, m]: pixel_modules_)
    {
      if (a == at && s == shadow_samplers)
        return m;
    }

    VkShaderModule r (
      create (vk_translate_pixel (code_, info_, at, shadow_samplers)));

    pixel_modules_.emplace_back (at, shadow_samplers, r);
    return r;
  }

  VkShaderModule vk_shader::
  create (const vector<uint32_t>& spirv)
  {
    VkShaderModuleCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = spirv.size () * sizeof (uint32_t);
    ci.pCode = spirv.data ();

    VkShaderModule r;
    vk_check (vkCreateShaderModule (device ().handle (), &ci, nullptr, &r),
              "unable to create {} shader module", to_string (stage_));
    return r;
  }
}
