#pragma once

#include <cstdint>

namespace Vulkan
{
  // Direct3D 9 types and constants.
  //
  // We declare the subset that the renderer uses, with the values from the
  // Direct3D 9 headers. This way the Vulkan code doesn't need d3d9.h and
  // Vulkan.cpp converts between the two where they meet.
  //
  // Note that vertex_element is the exception since we reinterpret IW4's
  // D3DVERTEXELEMENT9 arrays in place. So its layout must match (which
  // Vulkan.cpp checks with static_assert).
  //
  namespace d3d9
  {
    // Note that the DXT formats are FOURCC codes.
    //
    enum format: std::uint32_t
    {
      fmt_unknown       = 0,
      fmt_a8r8g8b8      = 21,
      fmt_x8r8g8b8      = 22,
      fmt_r5g6b5        = 23,
      fmt_a1r5g5b5      = 25,
      fmt_a8            = 28,
      fmt_a8b8g8r8      = 32,
      fmt_g16r16        = 34,
      fmt_a16b16g16r16  = 36,
      fmt_p8            = 41,
      fmt_l8            = 50,
      fmt_a8l8          = 51,
      fmt_d16_lockable  = 70,
      fmt_d15s1         = 73,
      fmt_d24s8         = 75,
      fmt_d24x8         = 77,
      fmt_d16           = 80,
      fmt_d24fs8        = 83,
      fmt_vertexdata    = 100,
      fmt_index16       = 101,
      fmt_index32       = 102,
      fmt_g16r16f       = 112,
      fmt_a16b16g16r16f = 113,
      fmt_r32f          = 114,
      fmt_dxt1          = 827611204,
      fmt_dxt3          = 861165636,
      fmt_dxt5          = 894720068
    };

    enum resource_type: std::uint32_t
    {
      rtype_surface       = 1,
      rtype_volume        = 2,
      rtype_texture       = 3,
      rtype_volumetexture = 4,
      rtype_cubetexture   = 5,
      rtype_vertexbuffer  = 6,
      rtype_indexbuffer   = 7
    };

    enum pool: std::uint32_t
    {
      pool_default   = 0,
      pool_managed   = 1,
      pool_systemmem = 2,
      pool_scratch   = 3
    };

    enum multisample_type: std::uint32_t
    {
      multisample_none      = 0,
      multisample_2_samples = 2
    };

    enum cubemap_faces: std::uint32_t
    {
      cubemap_face_positive_x = 0,
      cubemap_face_negative_x = 1,
      cubemap_face_positive_y = 2,
      cubemap_face_negative_y = 3,
      cubemap_face_positive_z = 4,
      cubemap_face_negative_z = 5
    };

    enum query_type: std::uint32_t
    {
      querytype_event     = 8,
      querytype_occlusion = 9
    };

    enum primitive_type: std::uint32_t
    {
      pt_pointlist     = 1,
      pt_linelist      = 2,
      pt_linestrip     = 3,
      pt_trianglelist  = 4,
      pt_trianglestrip = 5,
      pt_trianglefan   = 6
    };

    enum render_state_type: std::uint32_t
    {
      rs_zenable                  = 7,
      rs_fillmode                 = 8,
      rs_zwriteenable             = 14,
      rs_alphatestenable          = 15,
      rs_srcblend                 = 19,
      rs_destblend                = 20,
      rs_cullmode                 = 22,
      rs_zfunc                    = 23,
      rs_alpharef                 = 24,
      rs_alphafunc                = 25,
      rs_alphablendenable         = 27,
      rs_stencilenable            = 52,
      rs_stencilfail              = 53,
      rs_stencilzfail             = 54,
      rs_stencilpass              = 55,
      rs_stencilfunc              = 56,
      rs_stencilref               = 57,
      rs_stencilmask              = 58,
      rs_stencilwritemask         = 59,
      rs_pointsize                = 154,
      rs_colorwriteenable         = 168,
      rs_blendop                  = 171,
      rs_scissortestenable        = 174,
      rs_slopescaledepthbias      = 175,
      rs_adaptivetess_y           = 181,
      rs_twosidedstencilmode      = 185,
      rs_ccw_stencilfail          = 186,
      rs_ccw_stencilzfail         = 187,
      rs_ccw_stencilpass          = 188,
      rs_ccw_stencilfunc          = 189,
      rs_colorwriteenable1        = 190,
      rs_colorwriteenable2        = 191,
      rs_colorwriteenable3        = 192,
      rs_blendfactor              = 193,
      rs_srgbwriteenable          = 194,
      rs_depthbias                = 195,
      rs_separatealphablendenable = 206,
      rs_srcblendalpha            = 207,
      rs_destblendalpha           = 208,
      rs_blendopalpha             = 209
    };

    enum sampler_state_type: std::uint32_t
    {
      samp_addressu      = 1,
      samp_addressv      = 2,
      samp_addressw      = 3,
      samp_bordercolor   = 4,
      samp_magfilter     = 5,
      samp_minfilter     = 6,
      samp_mipfilter     = 7,
      samp_mipmaplodbias = 8,
      samp_maxmiplevel   = 9,
      samp_maxanisotropy = 10,
      samp_srgbtexture   = 11
    };

    enum texture_filter_type: std::uint32_t
    {
      texf_none        = 0,
      texf_point       = 1,
      texf_linear      = 2,
      texf_anisotropic = 3
    };

    enum texture_address: std::uint32_t
    {
      taddress_wrap       = 1,
      taddress_mirror     = 2,
      taddress_clamp      = 3,
      taddress_border     = 4,
      taddress_mirroronce = 5
    };

    enum blend: std::uint32_t
    {
      blend_zero            = 1,
      blend_one             = 2,
      blend_srccolor        = 3,
      blend_invsrccolor     = 4,
      blend_srcalpha        = 5,
      blend_invsrcalpha     = 6,
      blend_destalpha       = 7,
      blend_invdestalpha    = 8,
      blend_destcolor       = 9,
      blend_invdestcolor    = 10,
      blend_srcalphasat     = 11,
      blend_bothsrcalpha    = 12,
      blend_bothinvsrcalpha = 13,
      blend_blendfactor     = 14,
      blend_invblendfactor  = 15
    };

    enum blend_op: std::uint32_t
    {
      blendop_add         = 1,
      blendop_subtract    = 2,
      blendop_revsubtract = 3,
      blendop_min         = 4,
      blendop_max         = 5
    };

    enum cmp_func: std::uint32_t
    {
      cmp_never        = 1,
      cmp_less         = 2,
      cmp_equal        = 3,
      cmp_lessequal    = 4,
      cmp_greater      = 5,
      cmp_notequal     = 6,
      cmp_greaterequal = 7,
      cmp_always       = 8
    };

    enum cull: std::uint32_t
    {
      cull_none = 1,
      cull_cw   = 2,
      cull_ccw  = 3
    };

    enum fill_mode: std::uint32_t
    {
      fill_wireframe = 2,
      fill_solid     = 3
    };

    enum stencil_op: std::uint32_t
    {
      stencilop_keep    = 1,
      stencilop_zero    = 2,
      stencilop_replace = 3,
      stencilop_incrsat = 4,
      stencilop_decrsat = 5,
      stencilop_invert  = 6,
      stencilop_incr    = 7,
      stencilop_decr    = 8
    };

    enum zbuffer_type: std::uint32_t
    {
      zb_false = 0,
      zb_true  = 1
    };

    enum decl_type: std::uint32_t
    {
      decltype_float1    = 0,
      decltype_float2    = 1,
      decltype_float3    = 2,
      decltype_float4    = 3,
      decltype_d3dcolor  = 4,
      decltype_ubyte4    = 5,
      decltype_short2    = 6,
      decltype_short4    = 7,
      decltype_ubyte4n   = 8,
      decltype_short2n   = 9,
      decltype_short4n   = 10,
      decltype_ushort2n  = 11,
      decltype_ushort4n  = 12,
      decltype_udec3     = 13,
      decltype_dec3n     = 14,
      decltype_float16_2 = 15,
      decltype_float16_4 = 16,
      decltype_unused    = 17
    };

    enum decl_method: std::uint32_t
    {
      declmethod_default = 0
    };

    enum decl_usage: std::uint32_t
    {
      declusage_position     = 0,
      declusage_blendweight  = 1,
      declusage_blendindices = 2,
      declusage_normal       = 3,
      declusage_psize        = 4,
      declusage_texcoord     = 5,
      declusage_tangent      = 6,
      declusage_binormal     = 7,
      declusage_tessfactor   = 8,
      declusage_positiont    = 9,
      declusage_color        = 10,
      declusage_fog          = 11,
      declusage_depth        = 12,
      declusage_sample       = 13
    };

    // D3DUSAGE_*.
    //
    inline constexpr std::uint32_t usage_rendertarget = 1;
    inline constexpr std::uint32_t usage_depthstencil = 2;
    inline constexpr std::uint32_t usage_writeonly    = 8;
    inline constexpr std::uint32_t usage_dynamic      = 512;

    // D3DLOCK_*.
    //
    inline constexpr std::uint32_t lock_readonly    = 16;
    inline constexpr std::uint32_t lock_nooverwrite = 4096;
    inline constexpr std::uint32_t lock_discard     = 8192;

    // D3DCLEAR_*.
    //
    inline constexpr std::uint32_t clear_target  = 1;
    inline constexpr std::uint32_t clear_zbuffer = 2;
    inline constexpr std::uint32_t clear_stencil = 4;

    // D3DISSUE_*.
    //
    inline constexpr std::uint32_t issue_end   = 1;
    inline constexpr std::uint32_t issue_begin = 2;

    // D3DCOLORWRITEENABLE_*.
    //
    inline constexpr std::uint32_t colorwriteenable_red   = 1;
    inline constexpr std::uint32_t colorwriteenable_green = 2;
    inline constexpr std::uint32_t colorwriteenable_blue  = 4;
    inline constexpr std::uint32_t colorwriteenable_alpha = 8;

    // D3DVERTEXTEXTURESAMPLER0. The vertex texture samplers are numbered
    // from here.
    //
    inline constexpr std::uint32_t vertex_texture_sampler0 = 257;

    using color = std::uint32_t; // D3DCOLOR, 0xAARRGGBB.

    struct vertex_element
    {
      std::uint16_t stream;
      std::uint16_t offset;
      std::uint8_t type;
      std::uint8_t method;
      std::uint8_t usage;
      std::uint8_t usage_index;
    };

    // D3DDECL_END(), which terminates vertex element arrays.
    //
    inline constexpr vertex_element decl_end
    {
      0xff, 0, decltype_unused, declmethod_default, 0, 0
    };

    struct viewport
    {
      std::uint32_t x;
      std::uint32_t y;
      std::uint32_t width;
      std::uint32_t height;
      float min_z;
      float max_z;
    };

    struct rect
    {
      std::int32_t left;
      std::int32_t top;
      std::int32_t right;
      std::int32_t bottom;
    };

    struct point
    {
      std::int32_t x;
      std::int32_t y;
    };

    // D3DRECT, which Clear() takes.
    //
    struct clear_rect
    {
      std::int32_t x1;
      std::int32_t y1;
      std::int32_t x2;
      std::int32_t y2;
    };

    struct box
    {
      std::uint32_t left;
      std::uint32_t top;
      std::uint32_t right;
      std::uint32_t bottom;
      std::uint32_t front;
      std::uint32_t back;
    };

    struct locked_rect
    {
      std::int32_t pitch;
      void* bits;
    };

    struct locked_box
    {
      std::int32_t row_pitch;
      std::int32_t slice_pitch;
      void* bits;
    };

    struct surface_desc
    {
      d3d9::format format;
      resource_type type;
      std::uint32_t usage;
      d3d9::pool pool;
      multisample_type multisample;
      std::uint32_t multisample_quality;
      std::uint32_t width;
      std::uint32_t height;
    };

    struct volume_desc
    {
      d3d9::format format;
      resource_type type;
      std::uint32_t usage;
      d3d9::pool pool;
      std::uint32_t width;
      std::uint32_t height;
      std::uint32_t depth;
    };

    struct display_mode
    {
      std::uint32_t width;
      std::uint32_t height;
      std::uint32_t refresh_rate;
      d3d9::format format;
    };

    // @@ Nothing uses this. Can we drop it?
    //
    struct caps;
  }
}
