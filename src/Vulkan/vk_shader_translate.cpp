#include <Vulkan/vk_shader_translate.hpp>

#include <bit>       // countr_zero()
#include <array>
#include <cstring>   // strlen()
#include <vector>
#include <optional>
#include <utility>   // exchange()
#include <string_view>

#include <Vulkan/vk_spirv.hpp>

using namespace std;

namespace Vulkan
{
  const char*
  to_string (vk_shader_stage s) noexcept
  {
    switch (s)
    {
      case vk_shader_stage::vertex: return "vertex";
      case vk_shader_stage::pixel:  return "pixel";
    }

    return "unknown";
  }

  namespace
  {
    class vkd3d_category_impl: public error_category
    {
    public:
      virtual const char*
      name () const noexcept override
      {
        return "vkd3d-shader";
      }

      virtual string
      message (int v) const override
      {
        switch (v)
        {
          case VKD3D_OK:                     return "success";
          case VKD3D_ERROR_OUT_OF_MEMORY:    return "out of memory";
          case VKD3D_ERROR_INVALID_ARGUMENT: return "invalid argument";
          case VKD3D_ERROR_INVALID_SHADER:   return "invalid shader";
          case VKD3D_ERROR_NOT_IMPLEMENTED:  return "not implemented";
          default:                           return "error";
        }
      }
    };

    // Return the compile information for translating the bytecode to
    // SPIR-V. The next argument is the stage-specific chain.
    //
    // We pin the API version so that a newer vkd3d-shader doesn't change the
    // translation underneath us.
    //
    vkd3d_shader_compile_info
    compile_info (span<const uint32_t> code, const void* next)
    {
      static const vkd3d_shader_compile_option options[] =
      {
        {VKD3D_SHADER_COMPILE_OPTION_API_VERSION, VKD3D_SHADER_API_VERSION_1_17}
      };

      vkd3d_shader_compile_info r {};
      r.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
      r.next = next;
      r.source.code = code.data ();
      r.source.size = code.size_bytes ();
      r.source_type = VKD3D_SHADER_SOURCE_D3D_BYTECODE;
      r.target_type = VKD3D_SHADER_TARGET_SPIRV_BINARY;
      r.options = options;
      r.option_count = 1;
      r.log_level = VKD3D_SHADER_LOG_ERROR;
      return r;
    }

    // Translate the shader to SPIR-V with our descriptor layout (see
    // vk_constant_binding() and vk_sampler_binding()). The next argument is
    // chained after the binding information.
    //
    vector<uint32_t>
    translate (span<const uint32_t> code,
               const vk_d3dbc_info& info,
               vk_shader_stage stage,
               const void* next)
    {
      vkd3d_shader_visibility visibility (stage == vk_shader_stage::pixel
                                          ? VKD3D_SHADER_VISIBILITY_PIXEL
                                          : VKD3D_SHADER_VISIBILITY_VERTEX);

      // vkd3d-shader turns each constant register type into a constant
      // buffer, indexed as in vk_constant_set, which we map to our bindings
      // for the stage.
      //
      array<vkd3d_shader_resource_binding, 3> bindings;
      {
        const vk_constant_set sets[] = {vk_constant_set::floats,
                                        vk_constant_set::ints,
                                        vk_constant_set::bools};
        size_t i (0);
        for (vk_constant_set s: sets)
        {
          vkd3d_shader_resource_binding& b (bindings[i++]);
          b = {};
          b.type = VKD3D_SHADER_DESCRIPTOR_TYPE_CBV;
          b.register_space = 0;
          b.register_index = static_cast<uint32_t> (s);
          b.shader_visibility = visibility;
          b.flags = VKD3D_SHADER_BINDING_FLAG_BUFFER;
          b.binding = {0, vk_constant_binding (stage, s), 1};
        }
      }

      // A Direct3D 9 sampler register stands for the texture together with
      // its sampling state, so in Vulkan it becomes a combined image
      // sampler. We map each declared sampler to its binding.
      //
      array<vkd3d_shader_combined_resource_sampler, vk_pixel_samplers> samplers;
      size_t sampler_count (0);
      {
        uint32_t limit (stage == vk_shader_stage::pixel
                        ? vk_pixel_samplers
                        : vk_vertex_samplers);

        for (uint32_t m (info.samplers); m != 0; m &= m - 1)
        {
          uint32_t s (static_cast<uint32_t> (countr_zero (m)));

          if (s >= limit)
            throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                                   "{} shader sampler s{} out of range",
                                   to_string (stage), s);

          vkd3d_shader_combined_resource_sampler& c (samplers[sampler_count++]);
          c = {};
          c.resource_space = 0;
          c.resource_index = s;
          c.sampler_space = 0;
          c.sampler_index = s;
          c.shader_visibility = visibility;
          c.flags = VKD3D_SHADER_BINDING_FLAG_IMAGE;
          c.binding = {0, vk_sampler_binding (stage, s), 1};
        }
      }

      vkd3d_shader_interface_info interface {};
      interface.type = VKD3D_SHADER_STRUCTURE_TYPE_INTERFACE_INFO;
      interface.next = next;
      interface.bindings = bindings.data ();
      interface.binding_count = static_cast<unsigned int> (bindings.size ());
      interface.combined_samplers = samplers.data ();
      interface.combined_sampler_count =
        static_cast<unsigned int> (sampler_count);

      vkd3d_shader_spirv_target_info target {};
      target.type = VKD3D_SHADER_STRUCTURE_TYPE_SPIRV_TARGET_INFO;
      target.next = &interface;
      target.environment = VKD3D_SHADER_SPIRV_ENVIRONMENT_VULKAN_1_1;

      // Rewrite the bytecode so that vkd3d-shader translates it as Direct3D 9
      // defines it (see vk_d3dbc.cpp for details).
      //
      optional<vector<uint32_t>> x (
        vk_d3dbc_expand (
          vk_d3dbc_mask_inputs (vk_d3dbc_strip_comments (code))));

      if (!x)
        throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                               "shader has no temporary registers to spare "
                               "for expanding its instructions");

      vkd3d_shader_compile_info ci (compile_info (*x, &target));
      vkd3d_shader_code spv {};
      vk_shader_messages ms;

      if (int r = vkd3d_shader_compile (&ci, &spv, ms.out ()); r < 0)
        throw vk_shader_error (r,
                               "unable to translate {} shader: {}",
                               to_string (stage), ms.text ());

      vector<uint32_t> ws (spv.size / sizeof (uint32_t));
      memcpy (ws.data (), spv.code, ws.size () * sizeof (uint32_t));
      vkd3d_shader_free_shader_code (&spv);

      // vkd3d-shader sizes the float constant block after the highest
      // register that the shader names. With relative addressing the shader
      // may read past it, so we grow the block to the whole set (see
      // vk_d3dbc_scan()).
      //
      if (info.relative)
      {
        vk_spirv m (move (ws));
        m.resize_block (0,
                        vk_constant_binding (stage, vk_constant_set::floats),
                        info.floats);
        ws = m.release ();
      }

      return ws;
    }
  }

  const error_category&
  vkd3d_category () noexcept
  {
    static const vkd3d_category_impl c;
    return c;
  }

  vk_signature::
  vk_signature (vk_signature&& x) noexcept
    : s_ (exchange (x.s_, vkd3d_shader_signature {}))
  {
  }

  vk_signature& vk_signature::
  operator= (vk_signature&& x) noexcept
  {
    if (this != &x)
    {
      vkd3d_shader_free_shader_signature (&s_);
      s_ = exchange (x.s_, vkd3d_shader_signature {});
    }

    return *this;
  }

  string_view vk_shader_messages::
  text () const noexcept
  {
    // The diagnostics go into our error messages, so drop the trailing
    // newlines that vkd3d-shader adds.
    //
    if (p_ == nullptr || *p_ == '\0')
      return "no diagnostics";

    string_view r (p_, strlen (p_));
    while (!r.empty () && (r.back () == '\n' || r.back () == '\r'))
      r.remove_suffix (1);

    return r;
  }

  vk_signature::
  ~vk_signature ()
  {
    vkd3d_shader_free_shader_signature (&s_);
  }

  vk_shader_signatures
  vk_scan_signatures (span<const uint32_t> code)
  {
    vkd3d_shader_scan_signature_info si {};
    si.type = VKD3D_SHADER_STRUCTURE_TYPE_SCAN_SIGNATURE_INFO;

    // Strip the comments as for the translation (see translate()), so that
    // the scan sees the same bytecode.
    //
    const vector<uint32_t> c (vk_d3dbc_strip_comments (code));

    vkd3d_shader_compile_info ci (compile_info (c, &si));
    vk_shader_messages ms;

    if (int r = vkd3d_shader_scan (&ci, ms.out ()); r < 0)
      throw vk_shader_error (r, "unable to scan shader: {}", ms.text ());

    vk_shader_signatures r {vk_signature (si.input), vk_signature (si.output)};
    si.input = {};
    si.output = {};
    vkd3d_shader_free_scan_signature_info (&si);

    return r;
  }

  vk_varying_map
  vk_link_varyings (const vk_signature& output, const vk_signature& input)
  {
    // The map gives each pixel shader input the location that its matching
    // vertex shader output has. Direct3D 9 matches them by semantic while
    // Vulkan matches them by location.
    //
    vk_varying_map r (input.get ().element_count);
    unsigned int n (0);

    vkd3d_shader_build_varying_map (&output.get (), &input.get (),
                                    &n, r.data ());

    r.resize (n);
    return r;
  }

  vector<uint32_t>
  vk_compile_hlsl (string_view source, const char* profile)
  {
    static const vkd3d_shader_compile_option options[] =
    {
      {VKD3D_SHADER_COMPILE_OPTION_API_VERSION, VKD3D_SHADER_API_VERSION_1_17}
    };

    // Compile to Direct3D 9 bytecode, which then goes through the same
    // translation as IW4's shaders.
    //
    vkd3d_shader_hlsl_source_info hi {};
    hi.type = VKD3D_SHADER_STRUCTURE_TYPE_HLSL_SOURCE_INFO;
    hi.entry_point = "main";
    hi.profile = profile;

    vkd3d_shader_compile_info ci {};
    ci.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
    ci.next = &hi;
    ci.source.code = source.data ();
    ci.source.size = source.size ();
    ci.source_type = VKD3D_SHADER_SOURCE_HLSL;
    ci.target_type = VKD3D_SHADER_TARGET_D3D_BYTECODE;
    ci.options = options;
    ci.option_count = 1;
    ci.log_level = VKD3D_SHADER_LOG_ERROR;

    vkd3d_shader_code c {};
    vk_shader_messages ms;

    if (int r = vkd3d_shader_compile (&ci, &c, ms.out ()); r < 0)
      throw vk_shader_error (r,
                             "unable to compile {} shader: {}",
                             profile,
                             ms.text ());

    vector<uint32_t> r ((c.size + 3) / 4); // Round up to whole tokens.
    memcpy (r.data (), c.code, c.size);
    vkd3d_shader_free_shader_code (&c);
    return r;
  }

  vector<uint32_t>
  vk_translate_vertex (span<const uint32_t> code,
                       const vk_d3dbc_info& info,
                       const vk_varying_map& varyings)
  {
    vkd3d_shader_varying_map_info vi {};
    vi.type = VKD3D_SHADER_STRUCTURE_TYPE_VARYING_MAP_INFO;
    vi.varying_map = varyings.data ();
    vi.varying_count = static_cast<unsigned int> (varyings.size ());

    return translate (code, info, vk_shader_stage::vertex, &vi);
  }

  vector<uint32_t>
  vk_translate_pixel (span<const uint32_t> code,
                      const vk_d3dbc_info& info,
                      const vk_alpha_test& alpha,
                      uint32_t shadow_samplers)
  {
    // In Direct3D 9 sampling a depth texture compares with the reference
    // value, which is how IW4 does shadow mapping. vkd3d-shader has to know
    // which samplers these are to emit comparison samples.
    //
    vkd3d_shader_d3dbc_source_info di {};
    di.type = VKD3D_SHADER_STRUCTURE_TYPE_D3DBC_SOURCE_INFO;
    di.shadow_samplers = shadow_samplers;

    array<vkd3d_shader_parameter1, 2> ps {};
    vkd3d_shader_parameter_info pi {};
    pi.type = VKD3D_SHADER_STRUCTURE_TYPE_PARAMETER_INFO;
    pi.parameters = ps.data ();
    pi.next = &di;

    // Vulkan has no alpha test, so vkd3d-shader emits it into the shader
    // (as a discard) from these parameters.
    //
    if (alpha.func != VKD3D_SHADER_COMPARISON_FUNC_ALWAYS)
    {
      ps[0].name = VKD3D_SHADER_PARAMETER_NAME_ALPHA_TEST_FUNC;
      ps[0].type = VKD3D_SHADER_PARAMETER_TYPE_IMMEDIATE_CONSTANT;
      ps[0].data_type = VKD3D_SHADER_PARAMETER_DATA_TYPE_UINT32;
      ps[0].u.immediate_constant.u.u32 = static_cast<uint32_t> (alpha.func);

      ps[1].name = VKD3D_SHADER_PARAMETER_NAME_ALPHA_TEST_REF;
      ps[1].type = VKD3D_SHADER_PARAMETER_TYPE_IMMEDIATE_CONSTANT;
      ps[1].data_type = VKD3D_SHADER_PARAMETER_DATA_TYPE_FLOAT32;
      ps[1].u.immediate_constant.u.f32 = alpha.ref;

      pi.parameter_count = 2;
    }

    // vPos and vFace go through input registers that we patch to read the
    // built-ins after the translation (see vk_d3dbc_lower_misc()).
    //
    if (!info.position && !info.face)
      return translate (code, info, vk_shader_stage::pixel, &pi);

    optional<vk_d3dbc_lowered> l (vk_d3dbc_lower_misc (code));

    if (!l)
      throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                             "pixel shader has no input registers to spare "
                             "for vPos or vFace");

    vk_spirv m (translate (l->code, info, vk_shader_stage::pixel, &pi));

    if ((l->position && !m.lower_position (*l->position)) ||
        (l->face && !m.lower_face (*l->face)))
      throw vk_shader_error (VKD3D_ERROR_INVALID_SHADER,
                             "unable to lower vPos or vFace in pixel shader");

    return m.release ();
  }
}
