#include "Formats.hpp"

#include <algorithm> // ranges::find()
#include <cstring>   // memcpy()

using namespace std;

namespace Components
{
  namespace d3d11
  {
    namespace
    {
      constexpr D3DFORMAT fourcc_dxt1 (D3DFMT_DXT1);
      constexpr D3DFORMAT fourcc_dxt2 (D3DFMT_DXT2);
      constexpr D3DFORMAT fourcc_dxt3 (D3DFMT_DXT3);
      constexpr D3DFORMAT fourcc_dxt4 (D3DFMT_DXT4);
      constexpr D3DFORMAT fourcc_dxt5 (D3DFMT_DXT5);
      constexpr D3DFORMAT fourcc_ati1 (static_cast<D3DFORMAT> (MAKEFOURCC ('A', 'T', 'I', '1')));
      constexpr D3DFORMAT fourcc_ati2 (static_cast<D3DFORMAT> (MAKEFOURCC ('A', 'T', 'I', '2')));

      // The formats that we support, which are the ones IW4 creates (see
      // Image_Create2DTexture_PC() and friends) plus the ones a D3D9 driver
      // for this kind of hardware would also offer and that map to DXGI
      // without surprises. Anything else fails CheckDeviceFormat() and
      // texture creation, which is what a D3D9 driver does as well.
      //
      // Note that X8R8G8B8 maps to B8G8R8X8, which D3D11 samples with an
      // alpha of one, the same as D3D9.
      //
      constexpr DXGI_FORMAT unknown (DXGI_FORMAT_UNKNOWN);

      const format_info formats[]
      {
        {D3DFMT_A8R8G8B8,      DXGI_FORMAT_B8G8R8A8_TYPELESS,  DXGI_FORMAT_B8G8R8A8_UNORM,        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,   unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_X8R8G8B8,      DXGI_FORMAT_B8G8R8X8_TYPELESS,  DXGI_FORMAT_B8G8R8X8_UNORM,        DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,   unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_A8B8G8R8,      DXGI_FORMAT_R8G8B8A8_TYPELESS,  DXGI_FORMAT_R8G8B8A8_UNORM,        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,   unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_R5G6B5,        DXGI_FORMAT_B5G6R5_UNORM,       DXGI_FORMAT_B5G6R5_UNORM,          DXGI_FORMAT_B5G6R5_UNORM,          unknown,                        conversion::none, 1, 2,  2},
        {D3DFMT_A1R5G5B5,      DXGI_FORMAT_B5G5R5A1_UNORM,     DXGI_FORMAT_B5G5R5A1_UNORM,        DXGI_FORMAT_B5G5R5A1_UNORM,        unknown,                        conversion::none, 1, 2,  2},
        {D3DFMT_A4R4G4B4,      DXGI_FORMAT_B4G4R4A4_UNORM,     DXGI_FORMAT_B4G4R4A4_UNORM,        DXGI_FORMAT_B4G4R4A4_UNORM,        unknown,                        conversion::none, 1, 2,  2},
        {D3DFMT_A2B10G10R10,   DXGI_FORMAT_R10G10B10A2_UNORM,  DXGI_FORMAT_R10G10B10A2_UNORM,     DXGI_FORMAT_R10G10B10A2_UNORM,     unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_G16R16,        DXGI_FORMAT_R16G16_UNORM,       DXGI_FORMAT_R16G16_UNORM,          DXGI_FORMAT_R16G16_UNORM,          unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_A16B16G16R16,  DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM,    DXGI_FORMAT_R16G16B16A16_UNORM,    unknown,                        conversion::none, 1, 8,  8},
        {D3DFMT_A8,            DXGI_FORMAT_A8_UNORM,           DXGI_FORMAT_A8_UNORM,              DXGI_FORMAT_A8_UNORM,              unknown,                        conversion::none, 1, 1,  1},
        {D3DFMT_L8,            DXGI_FORMAT_R8G8B8A8_TYPELESS,  DXGI_FORMAT_R8G8B8A8_UNORM,        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,   unknown,                        conversion::l8,   1, 1,  4},
        {D3DFMT_A8L8,          DXGI_FORMAT_R8G8B8A8_TYPELESS,  DXGI_FORMAT_R8G8B8A8_UNORM,        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,   unknown,                        conversion::a8l8, 1, 2,  4},
        {D3DFMT_V8U8,          DXGI_FORMAT_R8G8_SNORM,         DXGI_FORMAT_R8G8_SNORM,            DXGI_FORMAT_R8G8_SNORM,            unknown,                        conversion::none, 1, 2,  2},
        {D3DFMT_Q8W8V8U8,      DXGI_FORMAT_R8G8B8A8_SNORM,     DXGI_FORMAT_R8G8B8A8_SNORM,        DXGI_FORMAT_R8G8B8A8_SNORM,        unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_R16F,          DXGI_FORMAT_R16_FLOAT,          DXGI_FORMAT_R16_FLOAT,             DXGI_FORMAT_R16_FLOAT,             unknown,                        conversion::none, 1, 2,  2},
        {D3DFMT_G16R16F,       DXGI_FORMAT_R16G16_FLOAT,       DXGI_FORMAT_R16G16_FLOAT,          DXGI_FORMAT_R16G16_FLOAT,          unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_A16B16G16R16F, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,    DXGI_FORMAT_R16G16B16A16_FLOAT,    unknown,                        conversion::none, 1, 8,  8},
        {D3DFMT_R32F,          DXGI_FORMAT_R32_FLOAT,          DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_R32_FLOAT,             unknown,                        conversion::none, 1, 4,  4},
        {D3DFMT_G32R32F,       DXGI_FORMAT_R32G32_FLOAT,       DXGI_FORMAT_R32G32_FLOAT,          DXGI_FORMAT_R32G32_FLOAT,          unknown,                        conversion::none, 1, 8,  8},
        {D3DFMT_A32B32G32R32F, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,    DXGI_FORMAT_R32G32B32A32_FLOAT,    unknown,                        conversion::none, 1, 16, 16},
        {fourcc_dxt1,          DXGI_FORMAT_BC1_TYPELESS,       DXGI_FORMAT_BC1_UNORM,             DXGI_FORMAT_BC1_UNORM_SRGB,        unknown,                        conversion::none, 4, 8,  8},
        {fourcc_dxt2,          DXGI_FORMAT_BC2_TYPELESS,       DXGI_FORMAT_BC2_UNORM,             DXGI_FORMAT_BC2_UNORM_SRGB,        unknown,                        conversion::none, 4, 16, 16},
        {fourcc_dxt3,          DXGI_FORMAT_BC2_TYPELESS,       DXGI_FORMAT_BC2_UNORM,             DXGI_FORMAT_BC2_UNORM_SRGB,        unknown,                        conversion::none, 4, 16, 16},
        {fourcc_dxt4,          DXGI_FORMAT_BC3_TYPELESS,       DXGI_FORMAT_BC3_UNORM,             DXGI_FORMAT_BC3_UNORM_SRGB,        unknown,                        conversion::none, 4, 16, 16},
        {fourcc_dxt5,          DXGI_FORMAT_BC3_TYPELESS,       DXGI_FORMAT_BC3_UNORM,             DXGI_FORMAT_BC3_UNORM_SRGB,        unknown,                        conversion::none, 4, 16, 16},
        {fourcc_ati1,          DXGI_FORMAT_BC4_UNORM,          DXGI_FORMAT_BC4_UNORM,             DXGI_FORMAT_BC4_UNORM,             unknown,                        conversion::none, 4, 8,  8},
        {fourcc_ati2,          DXGI_FORMAT_BC5_UNORM,          DXGI_FORMAT_BC5_UNORM,             DXGI_FORMAT_BC5_UNORM,             unknown,                        conversion::none, 4, 16, 16},
        {D3DFMT_D16,           DXGI_FORMAT_R16_TYPELESS,       DXGI_FORMAT_R16_UNORM,             DXGI_FORMAT_R16_UNORM,             DXGI_FORMAT_D16_UNORM,          conversion::none, 1, 2,  2},
        {D3DFMT_D24S8,         DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, conversion::none, 1, 4,  4},
        {D3DFMT_D24X8,         DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, conversion::none, 1, 4,  4},
        {D3DFMT_D32F_LOCKABLE, DXGI_FORMAT_R32_TYPELESS,       DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_D32_FLOAT,          conversion::none, 1, 4,  4},
        {null_format,          unknown,                        unknown,                           unknown,                           unknown,                        conversion::none, 1, 4,  4}
      };
    }

    const format_info*
    find_format (D3DFORMAT f)
    {
      auto i (ranges::find (formats, f, &format_info::d3d9));
      return i != ranges::end (formats) ? &*i : nullptr;
    }

    void
    convert (const format_info& f,
             const std::byte* from, size_t from_pitch,
             std::byte* to, size_t to_pitch,
             uint32_t width, uint32_t rows) noexcept
    {
      for (uint32_t y (0); y != rows; ++y)
      {
        const std::byte* s (from + y * from_pitch);
        std::byte* d (to + y * to_pitch);

        switch (f.convert)
        {
          case conversion::none:
          {
            memcpy (d, s, static_cast<size_t> (width) * f.size);
            break;
          }
          case conversion::l8:
          {
            for (uint32_t x (0); x != width; ++x, ++s, d += 4)
            {
              d[0] = d[1] = d[2] = s[0];
              d[3] = std::byte {0xFF};
            }
            break;
          }
          case conversion::a8l8:
          {
            // A8L8 stores the luminance in the low byte.
            //
            for (uint32_t x (0); x != width; ++x, s += 2, d += 4)
            {
              d[0] = d[1] = d[2] = s[0];
              d[3] = s[1];
            }
            break;
          }
        }
      }
    }

    DXGI_FORMAT
    vertex_format (D3DDECLTYPE t) noexcept
    {
      // These are the formats that the DecodeInput() function emitted by
      // dexilifier expects, one case per D3DDECLTYPE. Everything arrives as
      // unsigned integers, in particular the floats as their bits, so that
      // the shader can decode each type the way D3D9 does. The ones that
      // need sign extension (SHORT, DEC3N) also arrive unsigned and the
      // shader extends them.
      //
      switch (t)
      {
        case D3DDECLTYPE_FLOAT1:    return DXGI_FORMAT_R32_UINT;
        case D3DDECLTYPE_FLOAT2:    return DXGI_FORMAT_R32G32_UINT;
        case D3DDECLTYPE_FLOAT3:    return DXGI_FORMAT_R32G32B32_UINT;
        case D3DDECLTYPE_FLOAT4:    return DXGI_FORMAT_R32G32B32A32_UINT;
        case D3DDECLTYPE_D3DCOLOR:  return DXGI_FORMAT_R8G8B8A8_UINT;
        case D3DDECLTYPE_UBYTE4:    return DXGI_FORMAT_R8G8B8A8_UINT;
        case D3DDECLTYPE_SHORT2:    return DXGI_FORMAT_R16G16_UINT;
        case D3DDECLTYPE_SHORT4:    return DXGI_FORMAT_R16G16B16A16_UINT;
        case D3DDECLTYPE_UBYTE4N:   return DXGI_FORMAT_R8G8B8A8_UINT;
        case D3DDECLTYPE_SHORT2N:   return DXGI_FORMAT_R16G16_UINT;
        case D3DDECLTYPE_SHORT4N:   return DXGI_FORMAT_R16G16B16A16_UINT;
        case D3DDECLTYPE_USHORT2N:  return DXGI_FORMAT_R16G16_UINT;
        case D3DDECLTYPE_USHORT4N:  return DXGI_FORMAT_R16G16B16A16_UINT;
        case D3DDECLTYPE_UDEC3:     return DXGI_FORMAT_R10G10B10A2_UINT;
        case D3DDECLTYPE_DEC3N:     return DXGI_FORMAT_R10G10B10A2_UINT;
        case D3DDECLTYPE_FLOAT16_2: return DXGI_FORMAT_R16G16_UINT;
        case D3DDECLTYPE_FLOAT16_4: return DXGI_FORMAT_R16G16B16A16_UINT;
        default:                    return DXGI_FORMAT_UNKNOWN;
      }
    }

    uint32_t
    vertex_size (D3DDECLTYPE t) noexcept
    {
      switch (t)
      {
        case D3DDECLTYPE_FLOAT1:    return 4;
        case D3DDECLTYPE_FLOAT2:    return 8;
        case D3DDECLTYPE_FLOAT3:    return 12;
        case D3DDECLTYPE_FLOAT4:    return 16;
        case D3DDECLTYPE_D3DCOLOR:  return 4;
        case D3DDECLTYPE_UBYTE4:    return 4;
        case D3DDECLTYPE_SHORT2:    return 4;
        case D3DDECLTYPE_SHORT4:    return 8;
        case D3DDECLTYPE_UBYTE4N:   return 4;
        case D3DDECLTYPE_SHORT2N:   return 4;
        case D3DDECLTYPE_SHORT4N:   return 8;
        case D3DDECLTYPE_USHORT2N:  return 4;
        case D3DDECLTYPE_USHORT4N:  return 8;
        case D3DDECLTYPE_UDEC3:     return 4;
        case D3DDECLTYPE_DEC3N:     return 4;
        case D3DDECLTYPE_FLOAT16_2: return 4;
        case D3DDECLTYPE_FLOAT16_4: return 8;
        default:                    return 0;
      }
    }
  }
}
