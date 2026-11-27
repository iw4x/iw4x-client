#include <Vulkan/vk_material_state.hpp>

#include <bit>       // bit_cast()
#include <array>
#include <algorithm> // ranges::clamp()

using namespace std;

namespace Vulkan
{
  namespace
  {
    // IW4's material state bits (GfxStateBits::loadBits). The first word
    // holds the blending and rasterization state while the second holds the
    // depth and stencil state.
    //
    // Note that most fields store the Direct3D 9 value as is (the blend
    // factors and operations, for example) and some store it off by one
    // (the stencil operations and functions). The rest are IW4 enumerations
    // that we map with the tables below.
    //
    const uint32_t blend_rgb_shift       = 0;
    const uint32_t blend_alpha_shift     = 16;
    const uint32_t blend_op_mask         = 0x700;
    const uint32_t alpha_test_disable    = 0x800;
    const uint32_t alpha_test_mask       = 0x3000;
    const uint32_t alpha_test_gt_0       = 0x1000;
    const uint32_t alpha_test_lt_128     = 0x2000;
    const uint32_t cull_shift            = 14;
    const uint32_t color_write_rgb       = 0x8000000;
    const uint32_t color_write_alpha     = 0x10000000;
    const uint32_t srgb_write            = 0x40000000;
    const uint32_t polygon_mode_line     = 0x80000000;

    // Second word.
    //
    const uint32_t depth_write           = 0x1;
    const uint32_t depth_test_disable    = 0x2;
    const uint32_t depth_test_shift      = 2;
    const uint32_t polygon_offset_shift  = 4;
    const uint32_t polygon_offset_shadow = 3;
    const uint32_t stencil_front         = 0x40;
    const uint32_t stencil_back          = 0x80;
    const uint32_t stencil_front_shift   = 8;
    const uint32_t stencil_back_shift    = 20;

    // The cull field is 2 for back faces and 3 for front faces, anything
    // else is none. Direct3D 9 front faces are clockwise, so culling back
    // faces is D3DCULL_CCW.
    //
    const array<uint32_t, 4> cull_modes  {d3d9::cull_none,
                                          d3d9::cull_none,
                                          d3d9::cull_ccw,
                                          d3d9::cull_cw};

    const array<uint32_t, 4> depth_tests {d3d9::cmp_always,
                                          d3d9::cmp_less,
                                          d3d9::cmp_equal,
                                          d3d9::cmp_lessequal};

    // Render states that vk_apply_state_bits() sets. The caller forwards
    // exactly these to the context.
    //
    const d3d9::render_state_type states[]
    {
      d3d9::rs_colorwriteenable,
      d3d9::rs_srgbwriteenable,
      d3d9::rs_alphatestenable,
      d3d9::rs_alphafunc,
      d3d9::rs_alpharef,
      d3d9::rs_cullmode,
      d3d9::rs_fillmode,
      d3d9::rs_alphablendenable,
      d3d9::rs_separatealphablendenable,
      d3d9::rs_blendop,
      d3d9::rs_srcblend,
      d3d9::rs_destblend,
      d3d9::rs_blendopalpha,
      d3d9::rs_srcblendalpha,
      d3d9::rs_destblendalpha,
      d3d9::rs_zwriteenable,
      d3d9::rs_zenable,
      d3d9::rs_zfunc,
      d3d9::rs_depthbias,
      d3d9::rs_slopescaledepthbias,
      d3d9::rs_stencilenable,
      d3d9::rs_twosidedstencilmode,
      d3d9::rs_stencilpass,
      d3d9::rs_stencilfail,
      d3d9::rs_stencilzfail,
      d3d9::rs_stencilfunc,
      d3d9::rs_ccw_stencilpass,
      d3d9::rs_ccw_stencilfail,
      d3d9::rs_ccw_stencilzfail,
      d3d9::rs_ccw_stencilfunc
    };

    uint32_t
    field (uint32_t bits, uint32_t shift, uint32_t mask) noexcept
    {
      return (bits >> shift) & mask;
    }
  }

  span<const d3d9::render_state_type>
  vk_state_bits_states () noexcept
  {
    return states;
  }

  void
  vk_apply_state_bits (vk_render_states& r,
                       uint32_t b0,
                       uint32_t b1,
                       const vk_polygon_offsets& o)
  {
    r[d3d9::rs_colorwriteenable] =
      ((b0 & color_write_rgb) != 0
       ? d3d9::colorwriteenable_red |
         d3d9::colorwriteenable_green |
         d3d9::colorwriteenable_blue
       : 0) |
      ((b0 & color_write_alpha) != 0 ? d3d9::colorwriteenable_alpha : 0);

    r[d3d9::rs_srgbwriteenable] = (b0 & srgb_write) != 0;

    // The alpha test field's 0 value means alpha >= 128.
    //
    {
      uint32_t t (b0 & alpha_test_mask);

      r[d3d9::rs_alphatestenable] = (b0 & alpha_test_disable) == 0;
      r[d3d9::rs_alphafunc] = t == alpha_test_gt_0   ? d3d9::cmp_greater :
                              t == alpha_test_lt_128 ? d3d9::cmp_less
                                                     : d3d9::cmp_greaterequal;
      r[d3d9::rs_alpharef] = t == alpha_test_gt_0 ? 0 : 128;
    }

    r[d3d9::rs_cullmode] = cull_modes[field (b0, cull_shift, 3)];
    r[d3d9::rs_fillmode] = (b0 & polygon_mode_line) != 0 ? d3d9::fill_wireframe
                                                      : d3d9::fill_solid;

    // Blending is on if the color operation is not 0. A 0 alpha operation
    // means that alpha blends with the color factors.
    //
    // When blending is off, we reset the factors to the Direct3D 9 defaults.
    // They have no effect then but this way equivalent states probably end
    // up with the same pipeline.
    //
    {
      uint32_t op (field (b0, blend_rgb_shift + 8, 7));
      uint32_t op_alpha (field (b0, blend_alpha_shift + 8, 7));

      r[d3d9::rs_alphablendenable] = (b0 & blend_op_mask) != 0;
      r[d3d9::rs_separatealphablendenable] = op != 0 && op_alpha != 0;

      if (op != 0)
      {
        r[d3d9::rs_blendop]   = op;
        r[d3d9::rs_srcblend]  = field (b0, blend_rgb_shift, 0xf);
        r[d3d9::rs_destblend] = field (b0, blend_rgb_shift + 4, 0xf);

        bool separate (op_alpha != 0);
        uint32_t s (separate ? blend_alpha_shift : blend_rgb_shift);

        r[d3d9::rs_blendopalpha]   = field (b0, s + 8, 7);
        r[d3d9::rs_srcblendalpha]  = field (b0, s, 0xf);
        r[d3d9::rs_destblendalpha] = field (b0, s + 4, 0xf);
      }
      else
      {
        r[d3d9::rs_blendop]        = d3d9::blendop_add;
        r[d3d9::rs_srcblend]       = d3d9::blend_one;
        r[d3d9::rs_destblend]      = d3d9::blend_zero;
        r[d3d9::rs_blendopalpha]   = d3d9::blendop_add;
        r[d3d9::rs_srcblendalpha]  = d3d9::blend_one;
        r[d3d9::rs_destblendalpha] = d3d9::blend_zero;
      }
    }

    r[d3d9::rs_zwriteenable] = (b1 & depth_write) != 0;
    r[d3d9::rs_zenable] = (b1 & depth_test_disable) != 0 ? d3d9::zb_false
                                                     : d3d9::zb_true;
    r[d3d9::rs_zfunc] = depth_tests[field (b1, depth_test_shift, 3)];

    // The polygon offset field is a level (0 to 2) that scales the offset
    // dvars, except for 3, which selects the shadow map offsets.
    //
    {
      uint32_t l (field (b1, polygon_offset_shift, 3));
      float n (static_cast<float> (l));
      bool shadow (l == polygon_offset_shadow);

      float bias (shadow ? o.shadow_bias : o.bias * n);
      float scale (shadow ? o.shadow_scale : o.scale * n);

      r[d3d9::rs_depthbias] = bit_cast<uint32_t> (bias);
      r[d3d9::rs_slopescaledepthbias] = bit_cast<uint32_t> (scale);
    }

    // The back stencil state only applies with the front one enabled, in
    // which case it turns on two-sided stencil. Without it the back faces
    // use the front state, so we copy it over (as above, for the sake of the
    // pipeline).
    //
    {
      bool front ((b1 & stencil_front) != 0);
      bool back ((b1 & stencil_back) != 0);

      r[d3d9::rs_stencilenable] = front;
      r[d3d9::rs_twosidedstencilmode] = front && back;

      // Each operation and the function take 3 bits and are stored one off
      // the Direct3D 9 value.
      //
      auto set = [&r, b1] (uint32_t shift,
                           d3d9::render_state_type pass,
                           d3d9::render_state_type fail,
                           d3d9::render_state_type zfail,
                           d3d9::render_state_type func)
      {
        r[pass]  = field (b1, shift + 0, 7) + 1;
        r[fail]  = field (b1, shift + 3, 7) + 1;
        r[zfail] = field (b1, shift + 6, 7) + 1;
        r[func]  = field (b1, shift + 9, 7) + 1;
      };

      set (stencil_front_shift,
           d3d9::rs_stencilpass,
           d3d9::rs_stencilfail,
           d3d9::rs_stencilzfail,
           d3d9::rs_stencilfunc);

      set (back ? stencil_back_shift : stencil_front_shift,
           d3d9::rs_ccw_stencilpass,
           d3d9::rs_ccw_stencilfail,
           d3d9::rs_ccw_stencilzfail,
           d3d9::rs_ccw_stencilfunc);
    }
  }

  void
  vk_apply_sampler_state (vk_sampler_states& r,
                          uint8_t s,
                          const vk_texture_filtering& f)
  {
    // IW4's sampler state byte (see MaterialTextureDef::samplerState). The
    // low 3 bits are the filter and the next 2 bits are the mipmapping. The
    // top 3 bits clamp the texture coordinates, one bit per coordinate.
    //
    uint8_t filter (s & 7);
    uint8_t mipmap ((s >> 3) & 3);

    uint32_t min (d3d9::texf_point);
    uint32_t mag (d3d9::texf_point);
    uint32_t anisotropy (1);

    // Use anisotropic filtering with the material's degree clamped to the
    // configured range. If that leaves 1, then it is just linear filtering.
    //
    auto anisotropic = [&min, &mag, &anisotropy, &f] (uint32_t n)
    {
      anisotropy = ranges::clamp (n, f.anisotropy_min, f.anisotropy_max);

      if (anisotropy > 1)
      {
        min = d3d9::texf_anisotropic;
        mag = d3d9::texf_anisotropic;
      }
      else
      {
        min = d3d9::texf_linear;
        mag = d3d9::texf_linear;
      }
    };

    switch (filter)
    {
      case 2:
      {
        // Mipmapped linear filtering gets upgraded to anisotropic if the
        // configured minimum calls for it.
        //
        if (mipmap != 0 && f.anisotropy_min > 1)
          anisotropic (f.anisotropy_min);
        else
        {
          min = d3d9::texf_linear;
          mag = d3d9::texf_linear;
        }
        break;
      }
      case 3: anisotropic (2); break;
      case 4: anisotropic (4); break;
      default:                 break;
    }

    // The configured mip mode can override the material's (see
    // vk_mip_mode). Note that the mode never turns mipmapping on for a
    // texture that doesn't use it.
    //
    uint32_t mip (d3d9::texf_none);
    switch (f.mip)
    {
      case vk_mip_mode::material:
        mip = mipmap == 0 ? d3d9::texf_none :
              mipmap == 1 ? d3d9::texf_point : d3d9::texf_linear;
        break;
      case vk_mip_mode::linear:
        mip = mipmap == 0 ? d3d9::texf_none : d3d9::texf_linear;
        break;
      case vk_mip_mode::nearest:
        mip = mipmap == 0 ? d3d9::texf_none : d3d9::texf_point;
        break;
      case vk_mip_mode::none:
        mip = d3d9::texf_none;
        break;
    }

    r[d3d9::samp_minfilter] = min;
    r[d3d9::samp_magfilter] = mag;
    r[d3d9::samp_mipfilter] = mip;
    r[d3d9::samp_maxanisotropy] = anisotropy;

    auto address = [s] (uint8_t clamp) -> uint32_t
    {
      return (s & clamp) != 0 ? d3d9::taddress_clamp : d3d9::taddress_wrap;
    };

    r[d3d9::samp_addressu] = address (0x20);
    r[d3d9::samp_addressv] = address (0x40);
    r[d3d9::samp_addressw] = address (0x80);
  }
}
