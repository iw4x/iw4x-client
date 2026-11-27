#include <Vulkan/vk_state.hpp>

#include <bit>       // bit_cast()
#include <utility>   // pair
#include <algorithm> // clamp(), min(), ranges::find()

using namespace std;

namespace Vulkan
{
  namespace
  {
    // Direct3D 9 passes float states as their bits in a DWORD.
    //
    uint32_t
    float_bits (float f) noexcept
    {
      return bit_cast<uint32_t> (f);
    }
  }

  vk_render_states::
  vk_render_states (bool depth_buffer)
  {
    // Start with the defaults that Direct3D 9 documents for a new device.
    // Note that depth testing is only on by default if the device was
    // created with a depth buffer.
    //
    values_.fill (0);

    vk_render_states& s (*this);

    s[d3d9::rs_zenable]                  = depth_buffer ? d3d9::zb_true
                                                        : d3d9::zb_false;
    s[d3d9::rs_fillmode]                 = d3d9::fill_solid;
    s[d3d9::rs_zwriteenable]             = true;
    s[d3d9::rs_alphatestenable]          = false;
    s[d3d9::rs_srcblend]                 = d3d9::blend_one;
    s[d3d9::rs_destblend]                = d3d9::blend_zero;
    s[d3d9::rs_cullmode]                 = d3d9::cull_ccw;
    s[d3d9::rs_zfunc]                    = d3d9::cmp_lessequal;
    s[d3d9::rs_alpharef]                 = 0;
    s[d3d9::rs_alphafunc]                = d3d9::cmp_always;
    s[d3d9::rs_alphablendenable]         = false;
    s[d3d9::rs_stencilenable]            = false;
    s[d3d9::rs_stencilfail]              = d3d9::stencilop_keep;
    s[d3d9::rs_stencilzfail]             = d3d9::stencilop_keep;
    s[d3d9::rs_stencilpass]              = d3d9::stencilop_keep;
    s[d3d9::rs_stencilfunc]              = d3d9::cmp_always;
    s[d3d9::rs_stencilref]               = 0;
    s[d3d9::rs_stencilmask]              = 0xffffffff;
    s[d3d9::rs_stencilwritemask]         = 0xffffffff;
    s[d3d9::rs_pointsize]                = float_bits (1.0f);
    s[d3d9::rs_colorwriteenable]         = 0xf;
    s[d3d9::rs_blendop]                  = d3d9::blendop_add;
    s[d3d9::rs_scissortestenable]        = false;
    s[d3d9::rs_slopescaledepthbias]      = 0;
    s[d3d9::rs_twosidedstencilmode]      = false;
    s[d3d9::rs_ccw_stencilfail]          = d3d9::stencilop_keep;
    s[d3d9::rs_ccw_stencilzfail]         = d3d9::stencilop_keep;
    s[d3d9::rs_ccw_stencilpass]          = d3d9::stencilop_keep;
    s[d3d9::rs_ccw_stencilfunc]          = d3d9::cmp_always;
    s[d3d9::rs_colorwriteenable1]        = 0xf;
    s[d3d9::rs_colorwriteenable2]        = 0xf;
    s[d3d9::rs_colorwriteenable3]        = 0xf;
    s[d3d9::rs_blendfactor]              = 0xffffffff;
    s[d3d9::rs_depthbias]                = 0;
    s[d3d9::rs_separatealphablendenable] = false;
    s[d3d9::rs_srcblendalpha]            = d3d9::blend_one;
    s[d3d9::rs_destblendalpha]           = d3d9::blend_zero;
    s[d3d9::rs_blendopalpha]             = d3d9::blendop_add;
  }

  float vk_render_states::
  real (d3d9::render_state_type s) const noexcept
  {
    return bit_cast<float> (values_[s]);
  }

  vk_sampler_states::
  vk_sampler_states ()
  {
    // As above.
    //
    values_.fill (0);

    vk_sampler_states& s (*this);

    s[d3d9::samp_addressu]      = d3d9::taddress_wrap;
    s[d3d9::samp_addressv]      = d3d9::taddress_wrap;
    s[d3d9::samp_addressw]      = d3d9::taddress_wrap;
    s[d3d9::samp_bordercolor]   = 0;
    s[d3d9::samp_magfilter]     = d3d9::texf_point;
    s[d3d9::samp_minfilter]     = d3d9::texf_point;
    s[d3d9::samp_mipfilter]     = d3d9::texf_none;
    s[d3d9::samp_mipmaplodbias] = 0;
    s[d3d9::samp_maxmiplevel]   = 0;
    s[d3d9::samp_maxanisotropy] = 1;
  }

  VkBlendFactor
  vk_blend_factor (d3d9::blend b, bool alpha_target)
  {
    switch (b)
    {
      case d3d9::blend_zero:
        return VK_BLEND_FACTOR_ZERO;
      case d3d9::blend_one:
        return VK_BLEND_FACTOR_ONE;
      case d3d9::blend_srccolor:
        return VK_BLEND_FACTOR_SRC_COLOR;
      case d3d9::blend_invsrccolor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
      case d3d9::blend_srcalpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
      case d3d9::blend_invsrcalpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      // A target without alpha reads as opaque in Direct3D 9, so the
      // destination alpha is 1 there.
      //
      case d3d9::blend_destalpha:
        return alpha_target ? VK_BLEND_FACTOR_DST_ALPHA : VK_BLEND_FACTOR_ONE;
      case d3d9::blend_invdestalpha:
        return alpha_target ? VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
                            : VK_BLEND_FACTOR_ZERO;
      case d3d9::blend_destcolor:
        return VK_BLEND_FACTOR_DST_COLOR;
      case d3d9::blend_invdestcolor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
      case d3d9::blend_srcalphasat:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
      case d3d9::blend_blendfactor:
        return VK_BLEND_FACTOR_CONSTANT_COLOR;
      case d3d9::blend_invblendfactor:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

      default:
        return VK_BLEND_FACTOR_ONE;
    }
  }

  VkBlendOp
  vk_blend_op (d3d9::blend_op o)
  {
    switch (o)
    {
      case d3d9::blendop_subtract:    return VK_BLEND_OP_SUBTRACT;
      case d3d9::blendop_revsubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
      case d3d9::blendop_min:         return VK_BLEND_OP_MIN;
      case d3d9::blendop_max:         return VK_BLEND_OP_MAX;
      default:                        return VK_BLEND_OP_ADD;
    }
  }

  VkPipelineColorBlendAttachmentState
  vk_blend_attachment (const vk_render_states& s,
                       uint32_t target,
                       bool alpha_target)
  {
    const d3d9::render_state_type masks[] = {d3d9::rs_colorwriteenable,
                                             d3d9::rs_colorwriteenable1,
                                             d3d9::rs_colorwriteenable2,
                                             d3d9::rs_colorwriteenable3};

    VkPipelineColorBlendAttachmentState r {};

    r.colorWriteMask = s[masks[target]] & 0xf;

    if (!s[d3d9::rs_alphablendenable])
      return r;

    r.blendEnable = VK_TRUE;

    // The both* source modes are an old Direct3D shorthand that sets the
    // destination factor as well, which is then ignored.
    //
    auto factors = [] (uint32_t src,
                       uint32_t dst) -> pair<d3d9::blend, d3d9::blend>
    {
      if (src == d3d9::blend_bothsrcalpha)
        return {d3d9::blend_srcalpha, d3d9::blend_invsrcalpha};

      if (src == d3d9::blend_bothinvsrcalpha)
        return {d3d9::blend_invsrcalpha, d3d9::blend_srcalpha};

      return {static_cast<d3d9::blend> (src), static_cast<d3d9::blend> (dst)};
    };

    pair<d3d9::blend, d3d9::blend> c (
      factors (s[d3d9::rs_srcblend], s[d3d9::rs_destblend]));

    r.srcColorBlendFactor = vk_blend_factor (c.first, alpha_target);
    r.dstColorBlendFactor = vk_blend_factor (c.second, alpha_target);
    r.colorBlendOp =
      vk_blend_op (static_cast<d3d9::blend_op> (s[d3d9::rs_blendop]));

    if (s[d3d9::rs_separatealphablendenable])
    {
      pair<d3d9::blend, d3d9::blend> a (
        factors (s[d3d9::rs_srcblendalpha], s[d3d9::rs_destblendalpha]));

      r.srcAlphaBlendFactor = vk_blend_factor (a.first, alpha_target);
      r.dstAlphaBlendFactor = vk_blend_factor (a.second, alpha_target);
      r.alphaBlendOp =
        vk_blend_op (static_cast<d3d9::blend_op> (s[d3d9::rs_blendopalpha]));
    }
    else
    {
      // Direct3D 9 uses the color factors and operation for alpha unless
      // separate alpha blending is enabled.
      //
      r.srcAlphaBlendFactor = r.srcColorBlendFactor;
      r.dstAlphaBlendFactor = r.dstColorBlendFactor;
      r.alphaBlendOp = r.colorBlendOp;
    }

    // Spell the alpha factors with their alpha variants. For the alpha
    // channel Vulkan already takes the color factors' alpha component, so
    // this should not change the result.
    //
    // @@ Is there a driver that needs this, though? If not, then we could
    //    drop it.
    //
    using factor_pair = pair<VkBlendFactor, VkBlendFactor>;

    static const factor_pair alphas[] =
    {
      {VK_BLEND_FACTOR_SRC_COLOR,
       VK_BLEND_FACTOR_SRC_ALPHA},
      {VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
       VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
      {VK_BLEND_FACTOR_DST_COLOR,
       VK_BLEND_FACTOR_DST_ALPHA},
      {VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
       VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA}
    };

    auto alpha = [] (VkBlendFactor f)
    {
      auto i (ranges::find (alphas, f, &factor_pair::first));
      return i != ranges::end (alphas) ? i->second : f;
    };

    r.srcAlphaBlendFactor = alpha (r.srcAlphaBlendFactor);
    r.dstAlphaBlendFactor = alpha (r.dstAlphaBlendFactor);

    return r;
  }

  array<float, 4>
  vk_blend_constants (const vk_render_states& s)
  {
    // The blend factor is a D3DCOLOR, that is, ARGB with 8 bits per channel.
    //
    uint32_t c (s[d3d9::rs_blendfactor]);

    auto channel = [c] (uint32_t shift)
    {
      return static_cast<float> ((c >> shift) & 0xff) / 255.0f;
    };

    return {channel (16), channel (8), channel (0), channel (24)};
  }

  VkCompareOp
  vk_compare_op (d3d9::cmp_func f)
  {
    switch (f)
    {
      case d3d9::cmp_never:        return VK_COMPARE_OP_NEVER;
      case d3d9::cmp_less:         return VK_COMPARE_OP_LESS;
      case d3d9::cmp_equal:        return VK_COMPARE_OP_EQUAL;
      case d3d9::cmp_lessequal:    return VK_COMPARE_OP_LESS_OR_EQUAL;
      case d3d9::cmp_greater:      return VK_COMPARE_OP_GREATER;
      case d3d9::cmp_notequal:     return VK_COMPARE_OP_NOT_EQUAL;
      case d3d9::cmp_greaterequal: return VK_COMPARE_OP_GREATER_OR_EQUAL;
      default:                     return VK_COMPARE_OP_ALWAYS;
    }
  }

  VkStencilOp
  vk_stencil_op (d3d9::stencil_op o)
  {
    switch (o)
    {
      case d3d9::stencilop_zero:    return VK_STENCIL_OP_ZERO;
      case d3d9::stencilop_replace: return VK_STENCIL_OP_REPLACE;
      case d3d9::stencilop_incrsat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
      case d3d9::stencilop_decrsat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
      case d3d9::stencilop_invert:  return VK_STENCIL_OP_INVERT;
      case d3d9::stencilop_incr:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
      case d3d9::stencilop_decr:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
      default:                      return VK_STENCIL_OP_KEEP;
    }
  }

  VkCullModeFlags
  vk_cull_mode (d3d9::cull c)
  {
    switch (c)
    {
      case d3d9::cull_cw:  return VK_CULL_MODE_FRONT_BIT;
      case d3d9::cull_ccw: return VK_CULL_MODE_BACK_BIT;
      default:             return VK_CULL_MODE_NONE;
    }
  }

  vk_stencil
  vk_stencil_state (const vk_render_states& s)
  {
    auto face = [&s] (d3d9::render_state_type fail,
                      d3d9::render_state_type depth_fail,
                      d3d9::render_state_type pass,
                      d3d9::render_state_type func)
    {
      VkStencilOpState r {};
      r.failOp = vk_stencil_op (static_cast<d3d9::stencil_op> (s[fail]));
      r.passOp = vk_stencil_op (static_cast<d3d9::stencil_op> (s[pass]));
      r.depthFailOp =
        vk_stencil_op (static_cast<d3d9::stencil_op> (s[depth_fail]));
      r.compareOp = vk_compare_op (static_cast<d3d9::cmp_func> (s[func]));
      r.compareMask = s[d3d9::rs_stencilmask];
      r.writeMask = s[d3d9::rs_stencilwritemask];
      r.reference = s[d3d9::rs_stencilref];
      return r;
    };

    vk_stencil r;
    r.enable = s[d3d9::rs_stencilenable] != 0;
    r.front = face (d3d9::rs_stencilfail,
                    d3d9::rs_stencilzfail,
                    d3d9::rs_stencilpass,
                    d3d9::rs_stencilfunc);

    // Unless two-sided stencil is on, Direct3D 9 applies the clockwise
    // states to both faces.
    //
    r.back = s[d3d9::rs_twosidedstencilmode]
      ? face (d3d9::rs_ccw_stencilfail,
              d3d9::rs_ccw_stencilzfail,
              d3d9::rs_ccw_stencilpass,
              d3d9::rs_ccw_stencilfunc)
      : r.front;

    return r;
  }

  array<float, 2>
  vk_depth_bias (const vk_render_states& s, VkFormat depth)
  {
    // Direct3D 9 gives the constant bias in depth units while Vulkan counts
    // it in the smallest resolvable depth differences, which is 2^-n for an
    // n-bit integer format. For a float format the difference depends on
    // the depth value, so we use 2^-23, which is right near 1 where most
    // depth values end up. The slope-scaled bias needs no conversion.
    //
    float r;
    switch (depth)
    {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT:  r = 65536.0f;    break;
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT: r = 8388608.0f;  break;
      default:                           r = 16777216.0f; break;
    }

    return {s.real (d3d9::rs_depthbias) * r,
            s.real (d3d9::rs_slopescaledepthbias)};
  }

  vk_alpha_test
  vk_alpha_test_state (const vk_render_states& s)
  {
    vk_alpha_test r;

    if (!s[d3d9::rs_alphatestenable])
      return r;

    // The vkd3d-shader comparison values are the same as in Direct3D 9. An
    // invalid function means no test, which feels like the safer choice.
    // The reference value is a byte.
    //
    uint32_t f (s[d3d9::rs_alphafunc]);
    r.func = static_cast<vkd3d_shader_comparison_func> (
      f >= d3d9::cmp_never && f <= d3d9::cmp_always ? f : d3d9::cmp_always);

    if (r.func != VKD3D_SHADER_COMPARISON_FUNC_ALWAYS)
      r.ref = static_cast<float> (s[d3d9::rs_alpharef] & 0xff) / 255.0f;

    return r;
  }

  VkPrimitiveTopology
  vk_topology (d3d9::primitive_type t)
  {
    switch (t)
    {
      case d3d9::pt_pointlist:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case d3d9::pt_linelist:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      case d3d9::pt_linestrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      case d3d9::pt_trianglestrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
      case d3d9::pt_trianglefan:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
      default:                     return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
  }

  vk_topology_class
  vk_topology_class_of (d3d9::primitive_type t)
  {
    switch (t)
    {
      case d3d9::pt_pointlist: return vk_topology_class::point;
      case d3d9::pt_linelist:
      case d3d9::pt_linestrip: return vk_topology_class::line;
      default:                 return vk_topology_class::triangle;
    }
  }

  uint32_t
  vk_vertex_count (d3d9::primitive_type t, uint32_t n)
  {
    switch (t)
    {
      case d3d9::pt_pointlist:     return n;
      case d3d9::pt_linelist:      return n * 2;
      case d3d9::pt_linestrip:     return n + 1;
      case d3d9::pt_trianglestrip:
      case d3d9::pt_trianglefan:   return n + 2;
      default:                     return n * 3;
    }
  }

  vk_sampler_key
  vk_sampler_key_of (const vk_sampler_states& s)
  {
    // Saturate the states that we keep in a byte so that a bogus value
    // cannot alias a valid one.
    //
    auto byte = [] (uint32_t v)
    {
      return static_cast<uint8_t> (min<uint32_t> (v, 0xff));
    };

    vk_sampler_key r {};
    r.address_u = byte (s[d3d9::samp_addressu]);
    r.address_v = byte (s[d3d9::samp_addressv]);
    r.address_w = byte (s[d3d9::samp_addressw]);
    r.mag_filter = byte (s[d3d9::samp_magfilter]);
    r.min_filter = byte (s[d3d9::samp_minfilter]);
    r.mip_filter = byte (s[d3d9::samp_mipfilter]);
    r.max_anisotropy = byte (s[d3d9::samp_maxanisotropy]);
    r.max_mip_level = byte (s[d3d9::samp_maxmiplevel]);
    r.border_color = s[d3d9::samp_bordercolor];
    r.lod_bias = bit_cast<float> (s[d3d9::samp_mipmaplodbias]);
    return r;
  }

  VkSamplerCreateInfo
  vk_sampler_info (const vk_sampler_key& k, float max_anisotropy)
  {
    auto filter = [] (uint8_t f)
    {
      return f == d3d9::texf_point || f == d3d9::texf_none ? VK_FILTER_NEAREST
                                                           : VK_FILTER_LINEAR;
    };

    auto address = [] (uint8_t a)
    {
      switch (a)
      {
        case d3d9::taddress_mirror:
        case d3d9::taddress_mirroronce:
          return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case d3d9::taddress_clamp:
          return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case d3d9::taddress_border:
          return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      }
    };

    VkSamplerCreateInfo r {};
    r.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    r.magFilter = filter (k.mag_filter);
    r.minFilter = filter (k.min_filter);
    r.addressModeU = address (k.address_u);
    r.addressModeV = address (k.address_v);
    r.addressModeW = address (k.address_w);
    r.mipLodBias = k.lod_bias;

    // In Direct3D 9 anisotropic filtering is a minification filter, so we
    // only enable it if the minification filter asks for it and the device
    // supports it.
    //
    if (k.min_filter == d3d9::texf_anisotropic && max_anisotropy > 1.0f)
    {
      r.anisotropyEnable = VK_TRUE;
      r.maxAnisotropy = clamp (static_cast<float> (k.max_anisotropy),
                               1.0f,
                               max_anisotropy);
    }
    else
      r.maxAnisotropy = 1.0f;

    // A shadow sampler compares as a Direct3D 9 depth texture does: the
    // result is 1 if the depth is not less than the reference value.
    //
    if (k.compare)
    {
      r.compareEnable = VK_TRUE;
      r.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    }

    // Despite its name, the max mip level is the most detailed level to
    // sample, that is, the minimum LOD. Without mip filtering only that
    // level is sampled.
    //
    r.minLod = static_cast<float> (k.max_mip_level);

    if (k.mip_filter == d3d9::texf_none)
    {
      r.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      r.maxLod = r.minLod;
    }
    else
    {
      r.mipmapMode = k.mip_filter == d3d9::texf_point
        ? VK_SAMPLER_MIPMAP_MODE_NEAREST
        : VK_SAMPLER_MIPMAP_MODE_LINEAR;
      r.maxLod = VK_LOD_CLAMP_NONE;
    }

    // Vulkan only has a few fixed border colors without the custom border
    // color extension, so we pick the closest one. Note that there is no
    // transparent white.
    //
    {
      uint32_t c (k.border_color);
      bool opaque ((c >> 24) >= 0x80);
      uint32_t sum (((c >> 16) & 0xff) + ((c >> 8) & 0xff) + (c & 0xff));
      bool white (sum >= 3 * 0x80);

      r.borderColor = white
        ? (opaque ? VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
                  : VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
        : (opaque ? VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK
                  : VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);
    }

    return r;
  }
}
