#include "States.hpp"

#include <algorithm> // clamp(), ranges::fill()
#include <bit>       // bit_cast()
#include <cmath>     // lround()
#include <cstring>   // memset()

using namespace std;

namespace Components
{
  namespace d3d11
  {
    namespace
    {
      DWORD
      float_state (float f) noexcept
      {
        return bit_cast<DWORD> (f);
      }

      // D3D11 numbers its blend factors, blend operations, comparisons,
      // and stencil operations the same as D3D9, so these mostly pass the
      // value through. What D3D11 lacks is the BOTH* shortcuts (which set
      // the destination factor as well, see blend_desc()) and the color
      // factors for alpha.
      //
      D3D11_BLEND
      color_blend (DWORD b) noexcept
      {
        return b >= D3DBLEND_ZERO && b <= D3DBLEND_INVBLENDFACTOR &&
               b != D3DBLEND_BOTHSRCALPHA && b != D3DBLEND_BOTHINVSRCALPHA
               ? static_cast<D3D11_BLEND> (b)
               : D3D11_BLEND_ONE;
      }

      D3D11_BLEND
      alpha_blend (DWORD b) noexcept
      {
        switch (b)
        {
          case D3DBLEND_SRCCOLOR:     return D3D11_BLEND_SRC_ALPHA;
          case D3DBLEND_INVSRCCOLOR:  return D3D11_BLEND_INV_SRC_ALPHA;
          case D3DBLEND_DESTCOLOR:    return D3D11_BLEND_DEST_ALPHA;
          case D3DBLEND_INVDESTCOLOR: return D3D11_BLEND_INV_DEST_ALPHA;
          default:                    return color_blend (b);
        }
      }

      D3D11_BLEND_OP
      blend_op (DWORD o) noexcept
      {
        return o >= D3DBLENDOP_ADD && o <= D3DBLENDOP_MAX
               ? static_cast<D3D11_BLEND_OP> (o)
               : D3D11_BLEND_OP_ADD;
      }

      D3D11_COMPARISON_FUNC
      comparison (DWORD c) noexcept
      {
        return c >= D3DCMP_NEVER && c <= D3DCMP_ALWAYS
               ? static_cast<D3D11_COMPARISON_FUNC> (c)
               : D3D11_COMPARISON_ALWAYS;
      }

      D3D11_STENCIL_OP
      stencil_op (DWORD o) noexcept
      {
        return o >= D3DSTENCILOP_KEEP && o <= D3DSTENCILOP_DECR
               ? static_cast<D3D11_STENCIL_OP> (o)
               : D3D11_STENCIL_OP_KEEP;
      }

      D3D11_TEXTURE_ADDRESS_MODE
      address (DWORD a) noexcept
      {
        return a >= D3DTADDRESS_WRAP && a <= D3DTADDRESS_MIRRORONCE
               ? static_cast<D3D11_TEXTURE_ADDRESS_MODE> (a)
               : D3D11_TEXTURE_ADDRESS_WRAP;
      }

      // Return the D3D11 filter type of a D3D9 one, point for none (which
      // only makes sense for mipmaps, see sampler_desc()).
      //
      D3D11_FILTER_TYPE
      filter_type (DWORD f) noexcept
      {
        return f == D3DTEXF_NONE || f == D3DTEXF_POINT
               ? D3D11_FILTER_TYPE_POINT
               : D3D11_FILTER_TYPE_LINEAR;
      }
    }

    void
    default_render_states (render_states& s, bool depth)
    {
      ranges::fill (s, 0);

      s[D3DRS_ZENABLE]                  = depth ? D3DZB_TRUE : D3DZB_FALSE;
      s[D3DRS_FILLMODE]                 = D3DFILL_SOLID;
      s[D3DRS_SHADEMODE]                = D3DSHADE_GOURAUD;
      s[D3DRS_ZWRITEENABLE]             = TRUE;
      s[D3DRS_LASTPIXEL]                = TRUE;
      s[D3DRS_SRCBLEND]                 = D3DBLEND_ONE;
      s[D3DRS_DESTBLEND]                = D3DBLEND_ZERO;
      s[D3DRS_CULLMODE]                 = D3DCULL_CCW;
      s[D3DRS_ZFUNC]                    = D3DCMP_LESSEQUAL;
      s[D3DRS_ALPHAFUNC]                = D3DCMP_ALWAYS;
      s[D3DRS_FOGEND]                   = float_state (1.0f);
      s[D3DRS_FOGDENSITY]               = float_state (1.0f);
      s[D3DRS_STENCILFAIL]              = D3DSTENCILOP_KEEP;
      s[D3DRS_STENCILZFAIL]             = D3DSTENCILOP_KEEP;
      s[D3DRS_STENCILPASS]              = D3DSTENCILOP_KEEP;
      s[D3DRS_STENCILFUNC]              = D3DCMP_ALWAYS;
      s[D3DRS_STENCILMASK]              = 0xFFFFFFFF;
      s[D3DRS_STENCILWRITEMASK]         = 0xFFFFFFFF;
      s[D3DRS_TEXTUREFACTOR]            = 0xFFFFFFFF;
      s[D3DRS_CLIPPING]                 = TRUE;
      s[D3DRS_LIGHTING]                 = TRUE;
      s[D3DRS_COLORVERTEX]              = TRUE;
      s[D3DRS_LOCALVIEWER]              = TRUE;
      s[D3DRS_DIFFUSEMATERIALSOURCE]    = D3DMCS_COLOR1;
      s[D3DRS_SPECULARMATERIALSOURCE]   = D3DMCS_COLOR2;
      s[D3DRS_POINTSIZE]                = float_state (1.0f);
      s[D3DRS_POINTSIZE_MIN]            = float_state (1.0f);
      s[D3DRS_POINTSCALE_A]             = float_state (1.0f);
      s[D3DRS_MULTISAMPLEANTIALIAS]     = TRUE;
      s[D3DRS_MULTISAMPLEMASK]          = 0xFFFFFFFF;
      s[D3DRS_POINTSIZE_MAX]            = float_state (64.0f);
      s[D3DRS_COLORWRITEENABLE]         = 0xF;
      s[D3DRS_BLENDOP]                  = D3DBLENDOP_ADD;
      s[D3DRS_MINTESSELLATIONLEVEL]     = float_state (1.0f);
      s[D3DRS_MAXTESSELLATIONLEVEL]     = float_state (1.0f);
      s[D3DRS_ADAPTIVETESS_Z]           = float_state (1.0f);
      s[D3DRS_CCW_STENCILFAIL]          = D3DSTENCILOP_KEEP;
      s[D3DRS_CCW_STENCILZFAIL]         = D3DSTENCILOP_KEEP;
      s[D3DRS_CCW_STENCILPASS]          = D3DSTENCILOP_KEEP;
      s[D3DRS_CCW_STENCILFUNC]          = D3DCMP_ALWAYS;
      s[D3DRS_COLORWRITEENABLE1]        = 0xF;
      s[D3DRS_COLORWRITEENABLE2]        = 0xF;
      s[D3DRS_COLORWRITEENABLE3]        = 0xF;
      s[D3DRS_BLENDFACTOR]              = 0xFFFFFFFF;
      s[D3DRS_SRCBLENDALPHA]            = D3DBLEND_ONE;
      s[D3DRS_DESTBLENDALPHA]           = D3DBLEND_ZERO;
      s[D3DRS_BLENDOPALPHA]             = D3DBLENDOP_ADD;
    }

    void
    default_sampler_states (sampler_states& s)
    {
      ranges::fill (s, 0);

      s[D3DSAMP_ADDRESSU]      = D3DTADDRESS_WRAP;
      s[D3DSAMP_ADDRESSV]      = D3DTADDRESS_WRAP;
      s[D3DSAMP_ADDRESSW]      = D3DTADDRESS_WRAP;
      s[D3DSAMP_MAGFILTER]     = D3DTEXF_POINT;
      s[D3DSAMP_MINFILTER]     = D3DTEXF_POINT;
      s[D3DSAMP_MIPFILTER]     = D3DTEXF_NONE;
      s[D3DSAMP_MAXANISOTROPY] = 1;
    }

    bool
    linear_sampler (const sampler_states& s) noexcept
    {
      return s[D3DSAMP_MAGFILTER] >= D3DTEXF_LINEAR &&
             s[D3DSAMP_MINFILTER] >= D3DTEXF_LINEAR;
    }

    D3D11_BLEND_DESC
    blend_desc (const render_states& s) noexcept
    {
      D3D11_BLEND_DESC d;
      memset (&d, 0, sizeof (d));

      // The color write masks are the only per target state D3D9 has.
      //
      d.IndependentBlendEnable = TRUE;

      const bool enable (s[D3DRS_ALPHABLENDENABLE] != FALSE);

      D3D11_BLEND    src       (D3D11_BLEND_ONE);
      D3D11_BLEND    dst       (D3D11_BLEND_ZERO);
      D3D11_BLEND_OP op        (D3D11_BLEND_OP_ADD);
      D3D11_BLEND    src_alpha (D3D11_BLEND_ONE);
      D3D11_BLEND    dst_alpha (D3D11_BLEND_ZERO);
      D3D11_BLEND_OP op_alpha  (D3D11_BLEND_OP_ADD);

      // Leave the factors at their defaults when blending is off so that
      // all the disabled states share one object.
      //
      if (enable)
      {
        auto factors ([] (DWORD sb, DWORD db, D3D11_BLEND (*map) (DWORD) noexcept,
                          D3D11_BLEND& rs, D3D11_BLEND& rd)
        {
          // BOTHSRCALPHA and BOTHINVSRCALPHA set both factors and ignore
          // the destination one.
          //
          if (sb == D3DBLEND_BOTHSRCALPHA)
          {
            rs = D3D11_BLEND_SRC_ALPHA;
            rd = D3D11_BLEND_INV_SRC_ALPHA;
          }
          else if (sb == D3DBLEND_BOTHINVSRCALPHA)
          {
            rs = D3D11_BLEND_INV_SRC_ALPHA;
            rd = D3D11_BLEND_SRC_ALPHA;
          }
          else
          {
            rs = map (sb);
            rd = map (db);
          }
        });

        factors (s[D3DRS_SRCBLEND], s[D3DRS_DESTBLEND], &color_blend, src, dst);
        op = blend_op (s[D3DRS_BLENDOP]);

        if (s[D3DRS_SEPARATEALPHABLENDENABLE] != FALSE)
        {
          factors (s[D3DRS_SRCBLENDALPHA], s[D3DRS_DESTBLENDALPHA], &alpha_blend, src_alpha, dst_alpha);
          op_alpha = blend_op (s[D3DRS_BLENDOPALPHA]);
        }
        else
        {
          factors (s[D3DRS_SRCBLEND], s[D3DRS_DESTBLEND], &alpha_blend, src_alpha, dst_alpha);
          op_alpha = op;
        }
      }

      const DWORD masks[4] {s[D3DRS_COLORWRITEENABLE],
                            s[D3DRS_COLORWRITEENABLE1],
                            s[D3DRS_COLORWRITEENABLE2],
                            s[D3DRS_COLORWRITEENABLE3]};

      for (UINT i (0); i != 8; ++i)
      {
        D3D11_RENDER_TARGET_BLEND_DESC& t (d.RenderTarget[i]);

        t.BlendEnable           = enable;
        t.SrcBlend              = src;
        t.DestBlend             = dst;
        t.BlendOp               = op;
        t.SrcBlendAlpha         = src_alpha;
        t.DestBlendAlpha        = dst_alpha;
        t.BlendOpAlpha          = op_alpha;
        t.RenderTargetWriteMask = static_cast<UINT8> (masks[i < 4 ? i : 0] & 0xF);
      }

      return d;
    }

    D3D11_DEPTH_STENCIL_DESC
    depth_stencil_desc (const render_states& s) noexcept
    {
      D3D11_DEPTH_STENCIL_DESC d;
      memset (&d, 0, sizeof (d));

      d.DepthEnable    = s[D3DRS_ZENABLE] != D3DZB_FALSE;
      d.DepthWriteMask = s[D3DRS_ZWRITEENABLE] != FALSE ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
      d.DepthFunc      = comparison (s[D3DRS_ZFUNC]);

      // As with blending, leave the stencil at its defaults when it's off.
      //
      d.StencilEnable    = s[D3DRS_STENCILENABLE] != FALSE;
      d.StencilReadMask  = 0xFF;
      d.StencilWriteMask = 0xFF;

      D3D11_DEPTH_STENCILOP_DESC front {D3D11_STENCIL_OP_KEEP,
                                        D3D11_STENCIL_OP_KEEP,
                                        D3D11_STENCIL_OP_KEEP,
                                        D3D11_COMPARISON_ALWAYS};
      D3D11_DEPTH_STENCILOP_DESC back (front);

      if (d.StencilEnable)
      {
        d.StencilReadMask  = static_cast<UINT8> (s[D3DRS_STENCILMASK]);
        d.StencilWriteMask = static_cast<UINT8> (s[D3DRS_STENCILWRITEMASK]);

        front.StencilFailOp      = stencil_op (s[D3DRS_STENCILFAIL]);
        front.StencilDepthFailOp = stencil_op (s[D3DRS_STENCILZFAIL]);
        front.StencilPassOp      = stencil_op (s[D3DRS_STENCILPASS]);
        front.StencilFunc        = comparison (s[D3DRS_STENCILFUNC]);

        // D3D9's counterclockwise side is D3D11's back face since we keep
        // D3D11's clockwise front faces (see rasterizer_desc()).
        //
        if (s[D3DRS_TWOSIDEDSTENCILMODE] != FALSE)
        {
          back.StencilFailOp      = stencil_op (s[D3DRS_CCW_STENCILFAIL]);
          back.StencilDepthFailOp = stencil_op (s[D3DRS_CCW_STENCILZFAIL]);
          back.StencilPassOp      = stencil_op (s[D3DRS_CCW_STENCILPASS]);
          back.StencilFunc        = comparison (s[D3DRS_CCW_STENCILFUNC]);
        }
        else
          back = front;
      }

      d.FrontFace = front;
      d.BackFace  = back;
      return d;
    }

    D3D11_RASTERIZER_DESC
    rasterizer_desc (const render_states& s, UINT depth_bits) noexcept
    {
      D3D11_RASTERIZER_DESC d;
      memset (&d, 0, sizeof (d));

      d.FillMode = s[D3DRS_FILLMODE] == D3DFILL_WIREFRAME ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

      // D3D9 and D3D11 both consider clockwise triangles front facing by
      // default, so D3D9 culling counterclockwise ones means culling the
      // back faces.
      //
      switch (s[D3DRS_CULLMODE])
      {
        case D3DCULL_CW:  d.CullMode = D3D11_CULL_FRONT; break;
        case D3DCULL_CCW: d.CullMode = D3D11_CULL_BACK;  break;
        default:          d.CullMode = D3D11_CULL_NONE;  break;
      }

      // D3D9 adds the bias to the depth as is while D3D11 multiplies it by
      // the smallest step of the depth buffer, 2^-bits for the normalized
      // formats.
      //
      const float bias (bit_cast<float> (s[D3DRS_DEPTHBIAS]));
      d.DepthBias             = static_cast<INT> (lround (static_cast<double> (bias) * static_cast<double> (1u << depth_bits)));
      d.SlopeScaledDepthBias  = bit_cast<float> (s[D3DRS_SLOPESCALEDEPTHBIAS]);
      d.DepthClipEnable       = TRUE;
      d.ScissorEnable         = s[D3DRS_SCISSORTESTENABLE] != FALSE;
      d.MultisampleEnable     = s[D3DRS_MULTISAMPLEANTIALIAS] != FALSE;
      d.AntialiasedLineEnable = s[D3DRS_ANTIALIASEDLINEENABLE] != FALSE;
      return d;
    }

    D3D11_SAMPLER_DESC
    sampler_desc (const sampler_states& s) noexcept
    {
      D3D11_SAMPLER_DESC d;
      memset (&d, 0, sizeof (d));

      const DWORD magnify (s[D3DSAMP_MAGFILTER]);
      const DWORD minify  (s[D3DSAMP_MINFILTER]);
      const DWORD mipmap  (s[D3DSAMP_MIPFILTER]);

      if (magnify == D3DTEXF_ANISOTROPIC || minify == D3DTEXF_ANISOTROPIC)
      {
        d.Filter        = D3D11_FILTER_ANISOTROPIC;
        d.MaxAnisotropy = clamp<UINT> (s[D3DSAMP_MAXANISOTROPY], 1, 16);
      }
      else
      {
        d.Filter        = D3D11_ENCODE_BASIC_FILTER (filter_type (minify),
                                                     filter_type (magnify),
                                                     filter_type (mipmap),
                                                     D3D11_FILTER_REDUCTION_TYPE_STANDARD);
        d.MaxAnisotropy = 1;
      }

      d.AddressU       = address (s[D3DSAMP_ADDRESSU]);
      d.AddressV       = address (s[D3DSAMP_ADDRESSV]);
      d.AddressW       = address (s[D3DSAMP_ADDRESSW]);
      d.MipLODBias     = bit_cast<float> (s[D3DSAMP_MIPMAPLODBIAS]);
      d.ComparisonFunc = D3D11_COMPARISON_NEVER;

      const D3DCOLOR b (s[D3DSAMP_BORDERCOLOR]);
      d.BorderColor[0] = ((b >> 16) & 0xFF) / 255.0f;
      d.BorderColor[1] = ((b >> 8)  & 0xFF) / 255.0f;
      d.BorderColor[2] = ( b        & 0xFF) / 255.0f;
      d.BorderColor[3] = ((b >> 24) & 0xFF) / 255.0f;

      // MAXMIPLEVEL is the most detailed level D3D9 samples. Without a
      // mipmap filter D3D9 samples that level only.
      //
      d.MinLOD = static_cast<float> (s[D3DSAMP_MAXMIPLEVEL]);
      d.MaxLOD = mipmap == D3DTEXF_NONE ? d.MinLOD : D3D11_FLOAT32_MAX;
      return d;
    }
  }
}
