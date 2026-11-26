#pragma once

#include <d3d11_1.h>

#include <cstddef>
#include <cstdint>

namespace Components
{
  namespace d3d11
  {
    // How a D3D9 format maps to DXGI.
    //
    // A texture is created with the resource format and viewed with the
    // others. Where a format has an sRGB variant, the resource format is
    // the typeless one so that we can create both views of it and pick
    // between them at draw time, as D3DSAMP_SRGBTEXTURE and
    // D3DRS_SRGBWRITEENABLE do in D3D9. Where it has none, the sRGB views
    // are the same as the linear ones (D3D9 quietly ignores sRGB for such
    // formats as well).
    //
    // The D3D9 data layout is what the game sees when it locks a surface.
    // Most formats store it unchanged but D3D11 has no luminance formats,
    // so those are expanded into RGBA when we upload them (see convert()).
    //
    enum class conversion
    {
      none,
      l8,  // L8 to R8G8B8A8 as (L, L, L, 1).
      a8l8 // A8L8 to R8G8B8A8 as (L, L, L, A).
    };

    struct format_info
    {
      D3DFORMAT   d3d9;
      DXGI_FORMAT resource;
      DXGI_FORMAT view;      // Shader resource and render target view.
      DXGI_FORMAT view_srgb;
      DXGI_FORMAT depth;     // Depth stencil view, if any.
      conversion  convert;
      uint32_t    block;     // Edge of a block in texels, 4 if compressed.
      uint32_t    size;      // Bytes in a block in the D3D9 layout.
      uint32_t    card_size; // Bytes in a block in the DXGI layout.
    };

    // Return the format information for the D3D9 format. Return nullptr if
    // we don't support it.
    //
    const format_info*
    find_format (D3DFORMAT);

    // Return true if the format is a depth format.
    //
    inline bool
    depth_format (const format_info& f) noexcept
    {
      return f.depth != DXGI_FORMAT_UNKNOWN;
    }

    // The D3D9 four-character code formats that drivers use as extensions.
    // NULL is a render target that takes no memory, used to render depth
    // only.
    //
    constexpr D3DFORMAT null_format (static_cast<D3DFORMAT> (MAKEFOURCC ('N', 'U', 'L', 'L')));

    // Return the number of blocks along the extent e at mipmap level m.
    //
    inline uint32_t
    mip_blocks (const format_info& f, uint32_t e, uint32_t m) noexcept
    {
      const uint32_t x (e >> m);
      return ((x != 0 ? x : 1) + f.block - 1) / f.block;
    }

    // Return the extent e at mipmap level m.
    //
    inline uint32_t
    mip_extent (uint32_t e, uint32_t m) noexcept
    {
      const uint32_t x (e >> m);
      return x != 0 ? x : 1;
    }

    // Return the pitch of a row of blocks in the D3D9 layout and the DXGI
    // layout, respectively.
    //
    inline uint32_t
    row_pitch (const format_info& f, uint32_t width) noexcept
    {
      return ((width + f.block - 1) / f.block) * f.size;
    }

    inline uint32_t
    card_row_pitch (const format_info& f, uint32_t width) noexcept
    {
      return ((width + f.block - 1) / f.block) * f.card_size;
    }

    // Convert rows of blocks from the D3D9 layout to the DXGI layout. The
    // width is in blocks.
    //
    void
    convert (const format_info&,
             const std::byte* from, size_t from_pitch,
             std::byte* to, size_t to_pitch,
             uint32_t width, uint32_t rows) noexcept;

    // Return the DXGI format of a vertex element that D3DDECLTYPE describes,
    // as the shaders expect it: raw, with any decoding done in the shader
    // (see the DecodeInput() function that dexilifier emits).
    //
    DXGI_FORMAT
    vertex_format (D3DDECLTYPE) noexcept;

    // Return the size in bytes of a vertex element of type D3DDECLTYPE.
    // Return 0 if the type is not valid.
    //
    uint32_t
    vertex_size (D3DDECLTYPE) noexcept;
  }
}
