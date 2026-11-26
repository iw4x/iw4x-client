#include "Direct3D.hpp"

#include "Device.hpp"
#include "Formats.hpp"

#include <algorithm> // ranges::sort(), ranges::unique()
#include <cmath>     // lround()
#include <cstring>   // memset()

using namespace std;

namespace Components
{
  namespace d3d11
  {
    namespace
    {
      // The formats that D3D9 accepts for the display and the back buffer.
      // We only offer the 32-bit ones, which is what DXGI scans out.
      //
      bool
      display_format (D3DFORMAT f) noexcept
      {
        return f == D3DFMT_X8R8G8B8 || f == D3DFMT_A8R8G8B8;
      }

      // The four-character codes that D3D9 drivers expose as vendor hacks
      // (resolvable depth textures, NULL, alpha to coverage, and so on).
      // We support NULL and none of the others: the game checks for them
      // with CheckDeviceFormat() and has a fallback for each.
      //
      bool
      fourcc_hack (D3DFORMAT f) noexcept
      {
        const DWORD v (static_cast<DWORD> (f));
        return v > 0xFFFF && find_format (f) == nullptr;
      }
    }

    HRESULT direct3d::
    create ()
    {
      if (FAILED (CreateDXGIFactory1 (IID_PPV_ARGS (&factory_))))
        return E_FAIL;

      ComPtr<IDXGIAdapter1> a;
      for (UINT i (0); factory_->EnumAdapters1 (i, &a) != DXGI_ERROR_NOT_FOUND; ++i)
      {
        DXGI_ADAPTER_DESC1 d;
        if (FAILED (a->GetDesc1 (&d)) || (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
          continue;

        ComPtr<IDXGIOutput> o;
        for (UINT j (0); a->EnumOutputs (j, &o) != DXGI_ERROR_NOT_FOUND; ++j)
          adapters_.push_back (adapter_info {a, o, d, nullptr, {}});
      }

      // No outputs at all. Offer the first hardware adapter anyway (and
      // the software one if that's all there is).
      //
      if (adapters_.empty ())
      {
        for (UINT i (0); factory_->EnumAdapters1 (i, &a) != DXGI_ERROR_NOT_FOUND; ++i)
        {
          DXGI_ADAPTER_DESC1 d;
          if (SUCCEEDED (a->GetDesc1 (&d)))
          {
            adapters_.push_back (adapter_info {a, nullptr, d, nullptr, {}});
            break;
          }
        }
      }

      return adapters_.empty () ? E_FAIL : S_OK;
    }

    ID3D11Device* direct3d::
    probe (UINT a)
    {
      adapter_info& ai (adapters_[a]);

      if (ai.probe == nullptr)
      {
        const D3D_FEATURE_LEVEL levels[] {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

        D3D11CreateDevice (ai.adapter.Get (),
                           D3D_DRIVER_TYPE_UNKNOWN,
                           nullptr,
                           0,
                           levels,
                           static_cast<UINT> (size (levels)),
                           D3D11_SDK_VERSION,
                           &ai.probe,
                           nullptr,
                           nullptr);
      }

      return ai.probe.Get ();
    }

    const vector<D3DDISPLAYMODE>& direct3d::
    modes (UINT a)
    {
      adapter_info& ai (adapters_[a]);

      if (ai.modes.empty ())
      {
        // DXGI lists every mode once per scaling and scanline ordering, and
        // with fractional refresh rates, so we deduplicate what D3D9 would
        // report as the same mode.
        //
        if (ai.output != nullptr)
        {
          for (DXGI_FORMAT f: {DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM})
          {
            UINT n (0);
            if (FAILED (ai.output->GetDisplayModeList (f, 0, &n, nullptr)) || n == 0)
              continue;

            vector<DXGI_MODE_DESC> ms (n);
            if (FAILED (ai.output->GetDisplayModeList (f, 0, &n, ms.data ())))
              continue;

            for (const DXGI_MODE_DESC& m: ms)
            {
              const UINT hz (m.RefreshRate.Denominator != 0
                             ? static_cast<UINT> (lround (static_cast<double> (m.RefreshRate.Numerator) /
                                                          m.RefreshRate.Denominator))
                             : 0);

              ai.modes.push_back (D3DDISPLAYMODE {m.Width, m.Height, hz, D3DFMT_X8R8G8B8});
            }

            break;
          }
        }

        if (ai.modes.empty ())
        {
          D3DDISPLAYMODE m;
          if (SUCCEEDED (GetAdapterDisplayMode (a, &m)))
            ai.modes.push_back (m);
        }

        auto key ([] (const D3DDISPLAYMODE& m)
        {
          return tuple (m.Width, m.Height, m.RefreshRate);
        });

        ranges::sort (ai.modes, {}, key);
        auto r (ranges::unique (ai.modes, {}, key));
        ai.modes.erase (r.begin (), r.end ());
      }

      return ai.modes;
    }

    HRESULT direct3d::
    RegisterSoftwareDevice (void*)
    {
      return D3DERR_INVALIDCALL;
    }

    UINT direct3d::
    GetAdapterCount ()
    {
      return static_cast<UINT> (adapters_.size ());
    }

    HRESULT direct3d::
    GetAdapterIdentifier (UINT a, DWORD, D3DADAPTER_IDENTIFIER9* r)
    {
      if (r == nullptr || a >= adapters_.size ())
        return D3DERR_INVALIDCALL;

      const adapter_info& ai (adapters_[a]);

      memset (r, 0, sizeof (*r));
      strcpy_s (r->Driver, "d3d11.dll");
      WideCharToMultiByte (CP_UTF8, 0, ai.desc.Description, -1,
                           r->Description, sizeof (r->Description), nullptr, nullptr);

      if (ai.output != nullptr)
      {
        DXGI_OUTPUT_DESC od;
        if (SUCCEEDED (ai.output->GetDesc (&od)))
          WideCharToMultiByte (CP_UTF8, 0, od.DeviceName, -1,
                               r->DeviceName, sizeof (r->DeviceName), nullptr, nullptr);
      }

      LARGE_INTEGER v;
      if (SUCCEEDED (ai.adapter->CheckInterfaceSupport (__uuidof (IDXGIDevice), &v)))
        r->DriverVersion = v;

      r->VendorId = ai.desc.VendorId;
      r->DeviceId = ai.desc.DeviceId;
      r->SubSysId = ai.desc.SubSysId;
      r->Revision = ai.desc.Revision;

      // D3D9 identifies the adapter by GUID. The LUID is the closest DXGI
      // has and stays the same until the next reboot, which is all the
      // game needs (it keys its cached settings on the vendor and device
      // anyway).
      //
      memcpy (&r->DeviceIdentifier, &ai.desc.AdapterLuid, sizeof (ai.desc.AdapterLuid));
      r->WHQLLevel = 1;
      return D3D_OK;
    }

    UINT direct3d::
    GetAdapterModeCount (UINT a, D3DFORMAT f)
    {
      if (a >= adapters_.size () || !display_format (f))
        return 0;

      return static_cast<UINT> (modes (a).size ());
    }

    HRESULT direct3d::
    EnumAdapterModes (UINT a, D3DFORMAT f, UINT i, D3DDISPLAYMODE* r)
    {
      if (r == nullptr || a >= adapters_.size () || !display_format (f))
        return D3DERR_INVALIDCALL;

      const vector<D3DDISPLAYMODE>& ms (modes (a));
      if (i >= ms.size ())
        return D3DERR_INVALIDCALL;

      *r = ms[i];
      r->Format = f;
      return D3D_OK;
    }

    HRESULT direct3d::
    GetAdapterDisplayMode (UINT a, D3DDISPLAYMODE* r)
    {
      if (r == nullptr || a >= adapters_.size ())
        return D3DERR_INVALIDCALL;

      DEVMODEW m {};
      m.dmSize = sizeof (m);

      const wchar_t* name (nullptr);
      DXGI_OUTPUT_DESC od;
      if (adapters_[a].output != nullptr && SUCCEEDED (adapters_[a].output->GetDesc (&od)))
        name = od.DeviceName;

      if (!EnumDisplaySettingsW (name, ENUM_CURRENT_SETTINGS, &m))
        return D3DERR_INVALIDCALL;

      r->Width       = m.dmPelsWidth;
      r->Height      = m.dmPelsHeight;
      r->RefreshRate = m.dmDisplayFrequency;
      r->Format      = D3DFMT_X8R8G8B8;
      return D3D_OK;
    }

    HRESULT direct3d::
    CheckDeviceType (UINT a, D3DDEVTYPE t, D3DFORMAT af, D3DFORMAT bf, BOOL)
    {
      if (a >= adapters_.size () || t != D3DDEVTYPE_HAL)
        return D3DERR_NOTAVAILABLE;

      if (!display_format (af) || (bf != D3DFMT_UNKNOWN && !display_format (bf)))
        return D3DERR_NOTAVAILABLE;

      return D3D_OK;
    }

    HRESULT direct3d::
    CheckDeviceFormat (UINT a, D3DDEVTYPE t, D3DFORMAT, DWORD usage, D3DRESOURCETYPE rt, D3DFORMAT f)
    {
      if (a >= adapters_.size () || t != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;

      // Buffers come in any format.
      //
      if (rt == D3DRTYPE_VERTEXBUFFER || rt == D3DRTYPE_INDEXBUFFER)
        return D3D_OK;

      if (f == null_format)
        return (usage & D3DUSAGE_RENDERTARGET) != 0 && rt == D3DRTYPE_SURFACE
               ? D3D_OK
               : D3DERR_NOTAVAILABLE;

      if (fourcc_hack (f))
        return D3DERR_NOTAVAILABLE;

      const format_info* fi (find_format (f));
      if (fi == nullptr)
        return D3DERR_NOTAVAILABLE;

      ID3D11Device* d (probe (a));
      if (d == nullptr)
        return D3DERR_NOTAVAILABLE;

      auto supports ([d] (DXGI_FORMAT x, UINT what)
      {
        UINT s (0);
        return SUCCEEDED (d->CheckFormatSupport (x, &s)) && (s & what) == what;
      });

      const bool depth (depth_format (*fi));

      if ((usage & D3DUSAGE_DEPTHSTENCIL) != 0)
        return depth && supports (fi->depth, D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
               ? D3D_OK
               : D3DERR_NOTAVAILABLE;

      if (depth && rt != D3DRTYPE_TEXTURE && rt != D3DRTYPE_SURFACE)
        return D3DERR_NOTAVAILABLE;

      UINT need (0);
      switch (rt)
      {
        case D3DRTYPE_TEXTURE:       need = D3D11_FORMAT_SUPPORT_TEXTURE2D;   break;
        case D3DRTYPE_CUBETEXTURE:   need = D3D11_FORMAT_SUPPORT_TEXTURECUBE; break;
        case D3DRTYPE_VOLUMETEXTURE: need = D3D11_FORMAT_SUPPORT_TEXTURE3D;   break;
        default:                     break;
      }

      if ((usage & D3DUSAGE_RENDERTARGET) != 0)
        need |= D3D11_FORMAT_SUPPORT_RENDER_TARGET;

      if ((usage & D3DUSAGE_QUERY_FILTER) != 0)
        need |= D3D11_FORMAT_SUPPORT_SHADER_SAMPLE;

      if ((usage & D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING) != 0)
        need |= D3D11_FORMAT_SUPPORT_BLENDABLE;

      if (!supports (fi->view, need))
        return D3DERR_NOTAVAILABLE;

      if ((usage & (D3DUSAGE_QUERY_SRGBREAD | D3DUSAGE_QUERY_SRGBWRITE)) != 0 &&
          fi->view_srgb == fi->view)
        return D3DERR_NOTAVAILABLE;

      if ((usage & D3DUSAGE_AUTOGENMIPMAP) != 0 &&
          !supports (fi->view, D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
        return D3DOK_NOAUTOGEN;

      return D3D_OK;
    }

    UINT direct3d::
    sample_count (UINT a, D3DFORMAT f, D3DMULTISAMPLE_TYPE t, DWORD q)
    {
      if (t == D3DMULTISAMPLE_NONE)
        return 1;

      const format_info* fi (find_format (f));
      ID3D11Device* d (a < adapters_.size () ? probe (a) : nullptr);
      if (fi == nullptr || d == nullptr)
        return 0;

      const DXGI_FORMAT x (depth_format (*fi) ? fi->depth : fi->view);

      auto supported ([d, x] (UINT n)
      {
        UINT levels (0);
        return SUCCEEDED (d->CheckMultisampleQualityLevels (x, n, &levels)) && levels != 0;
      });

      // The nonmaskable type has the sample counts as its quality levels,
      // starting from 2.
      //
      if (t == D3DMULTISAMPLE_NONMASKABLE)
      {
        DWORD level (0);
        for (UINT n (2); n <= 8; n *= 2)
        {
          if (supported (n) && level++ == q)
            return n;
        }

        return 0;
      }

      return supported (static_cast<UINT> (t)) ? static_cast<UINT> (t) : 0;
    }

    HRESULT direct3d::
    CheckDeviceMultiSampleType (UINT a, D3DDEVTYPE t, D3DFORMAT f, BOOL, D3DMULTISAMPLE_TYPE ms, DWORD* q)
    {
      if (a >= adapters_.size () || t != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;

      if (ms == D3DMULTISAMPLE_NONMASKABLE)
      {
        DWORD n (0);
        while (sample_count (a, f, ms, n) != 0)
          ++n;

        if (q != nullptr)
          *q = n;

        return n != 0 ? D3D_OK : D3DERR_NOTAVAILABLE;
      }

      if (sample_count (a, f, ms, 0) == 0)
        return D3DERR_NOTAVAILABLE;

      // We only ever create the standard pattern (quality zero), so that's
      // the one level we report.
      //
      if (q != nullptr)
        *q = 1;

      return D3D_OK;
    }

    HRESULT direct3d::
    CheckDepthStencilMatch (UINT a, D3DDEVTYPE t, D3DFORMAT, D3DFORMAT, D3DFORMAT ds)
    {
      if (a >= adapters_.size () || t != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;

      const format_info* fi (find_format (ds));
      return fi != nullptr && depth_format (*fi) ? D3D_OK : D3DERR_NOTAVAILABLE;
    }

    HRESULT direct3d::
    CheckDeviceFormatConversion (UINT a, D3DDEVTYPE t, D3DFORMAT from, D3DFORMAT to)
    {
      if (a >= adapters_.size () || t != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;

      // StretchRect() converts between formats with a draw, so any pair of
      // renderable color formats would work. But we only copy and resolve
      // for now, so only report what that handles.
      //
      return from == to ? D3D_OK : D3DERR_NOTAVAILABLE;
    }

    HRESULT direct3d::
    GetDeviceCaps (UINT a, D3DDEVTYPE t, D3DCAPS9* c)
    {
      if (c == nullptr || a >= adapters_.size ())
        return D3DERR_INVALIDCALL;

      if (t != D3DDEVTYPE_HAL)
        return D3DERR_NOTAVAILABLE;

      memset (c, 0, sizeof (*c));

      c->DeviceType     = D3DDEVTYPE_HAL;
      c->AdapterOrdinal = a;

      c->Caps2 = D3DCAPS2_CANAUTOGENMIPMAP | D3DCAPS2_DYNAMICTEXTURES | D3DCAPS2_FULLSCREENGAMMA;
      c->Caps3 = D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD |
                 D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION      |
                 D3DCAPS3_COPY_TO_VIDMEM                   |
                 D3DCAPS3_COPY_TO_SYSTEMMEM;

      c->PresentationIntervals = D3DPRESENT_INTERVAL_IMMEDIATE |
                                 D3DPRESENT_INTERVAL_ONE       |
                                 D3DPRESENT_INTERVAL_TWO       |
                                 D3DPRESENT_INTERVAL_THREE     |
                                 D3DPRESENT_INTERVAL_FOUR;

      c->CursorCaps = D3DCURSORCAPS_COLOR;

      c->DevCaps = D3DDEVCAPS_EXECUTESYSTEMMEMORY    |
                   D3DDEVCAPS_EXECUTEVIDEOMEMORY     |
                   D3DDEVCAPS_TLVERTEXSYSTEMMEMORY   |
                   D3DDEVCAPS_TLVERTEXVIDEOMEMORY    |
                   D3DDEVCAPS_TEXTURESYSTEMMEMORY    |
                   D3DDEVCAPS_TEXTUREVIDEOMEMORY     |
                   D3DDEVCAPS_DRAWPRIMTLVERTEX       |
                   D3DDEVCAPS_CANRENDERAFTERFLIP     |
                   D3DDEVCAPS_TEXTURENONLOCALVIDMEM  |
                   D3DDEVCAPS_DRAWPRIMITIVES2        |
                   D3DDEVCAPS_DRAWPRIMITIVES2EX      |
                   D3DDEVCAPS_HWTRANSFORMANDLIGHT    |
                   D3DDEVCAPS_CANBLTSYSTONONLOCAL    |
                   D3DDEVCAPS_HWRASTERIZATION        |
                   D3DDEVCAPS_PUREDEVICE;

      c->PrimitiveMiscCaps = D3DPMISCCAPS_MASKZ                      |
                             D3DPMISCCAPS_CULLNONE                   |
                             D3DPMISCCAPS_CULLCW                     |
                             D3DPMISCCAPS_CULLCCW                    |
                             D3DPMISCCAPS_COLORWRITEENABLE           |
                             D3DPMISCCAPS_CLIPTLVERTS                |
                             D3DPMISCCAPS_TSSARGTEMP                 |
                             D3DPMISCCAPS_BLENDOP                    |
                             D3DPMISCCAPS_INDEPENDENTWRITEMASKS      |
                             D3DPMISCCAPS_PERSTAGECONSTANT           |
                             D3DPMISCCAPS_FOGANDSPECULARALPHA        |
                             D3DPMISCCAPS_SEPARATEALPHABLEND         |
                             D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS    |
                             D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING |
                             D3DPMISCCAPS_FOGVERTEXCLAMPED;

      c->RasterCaps = D3DPRASTERCAPS_DITHER              |
                      D3DPRASTERCAPS_ZTEST               |
                      D3DPRASTERCAPS_FOGVERTEX           |
                      D3DPRASTERCAPS_FOGTABLE            |
                      D3DPRASTERCAPS_MIPMAPLODBIAS       |
                      D3DPRASTERCAPS_ANISOTROPY          |
                      D3DPRASTERCAPS_WFOG                |
                      D3DPRASTERCAPS_ZFOG                |
                      D3DPRASTERCAPS_COLORPERSPECTIVE    |
                      D3DPRASTERCAPS_SCISSORTEST         |
                      D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
                      D3DPRASTERCAPS_DEPTHBIAS           |
                      D3DPRASTERCAPS_MULTISAMPLE_TOGGLE;

      const DWORD cmp (D3DPCMPCAPS_NEVER        |
                       D3DPCMPCAPS_LESS         |
                       D3DPCMPCAPS_EQUAL        |
                       D3DPCMPCAPS_LESSEQUAL    |
                       D3DPCMPCAPS_GREATER      |
                       D3DPCMPCAPS_NOTEQUAL     |
                       D3DPCMPCAPS_GREATEREQUAL |
                       D3DPCMPCAPS_ALWAYS);

      c->ZCmpCaps     = cmp;
      c->AlphaCmpCaps = cmp;

      const DWORD blend (D3DPBLENDCAPS_ZERO            |
                         D3DPBLENDCAPS_ONE             |
                         D3DPBLENDCAPS_SRCCOLOR        |
                         D3DPBLENDCAPS_INVSRCCOLOR     |
                         D3DPBLENDCAPS_SRCALPHA        |
                         D3DPBLENDCAPS_INVSRCALPHA     |
                         D3DPBLENDCAPS_DESTALPHA       |
                         D3DPBLENDCAPS_INVDESTALPHA    |
                         D3DPBLENDCAPS_DESTCOLOR       |
                         D3DPBLENDCAPS_INVDESTCOLOR    |
                         D3DPBLENDCAPS_SRCALPHASAT     |
                         D3DPBLENDCAPS_BOTHSRCALPHA    |
                         D3DPBLENDCAPS_BOTHINVSRCALPHA |
                         D3DPBLENDCAPS_BLENDFACTOR);

      c->SrcBlendCaps  = blend;
      c->DestBlendCaps = blend;

      c->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB    |
                     D3DPSHADECAPS_SPECULARGOURAUDRGB |
                     D3DPSHADECAPS_ALPHAGOURAUDBLEND  |
                     D3DPSHADECAPS_FOGGOURAUD;

      // No pow2 restrictions: D3D11 hardware handles any size.
      //
      c->TextureCaps = D3DPTEXTURECAPS_PERSPECTIVE  |
                       D3DPTEXTURECAPS_ALPHA        |
                       D3DPTEXTURECAPS_PROJECTED    |
                       D3DPTEXTURECAPS_CUBEMAP      |
                       D3DPTEXTURECAPS_VOLUMEMAP    |
                       D3DPTEXTURECAPS_MIPMAP       |
                       D3DPTEXTURECAPS_MIPVOLUMEMAP |
                       D3DPTEXTURECAPS_MIPCUBEMAP;

      const DWORD filter (D3DPTFILTERCAPS_MINFPOINT       |
                          D3DPTFILTERCAPS_MINFLINEAR      |
                          D3DPTFILTERCAPS_MINFANISOTROPIC |
                          D3DPTFILTERCAPS_MIPFPOINT       |
                          D3DPTFILTERCAPS_MIPFLINEAR      |
                          D3DPTFILTERCAPS_MAGFPOINT       |
                          D3DPTFILTERCAPS_MAGFLINEAR      |
                          D3DPTFILTERCAPS_MAGFANISOTROPIC);

      c->TextureFilterCaps       = filter;
      c->CubeTextureFilterCaps   = filter;
      c->VolumeTextureFilterCaps = filter;

      const DWORD addr (D3DPTADDRESSCAPS_WRAP          |
                        D3DPTADDRESSCAPS_MIRROR        |
                        D3DPTADDRESSCAPS_CLAMP         |
                        D3DPTADDRESSCAPS_BORDER        |
                        D3DPTADDRESSCAPS_INDEPENDENTUV |
                        D3DPTADDRESSCAPS_MIRRORONCE);

      c->TextureAddressCaps       = addr;
      c->VolumeTextureAddressCaps = addr;

      c->LineCaps = D3DLINECAPS_TEXTURE  |
                    D3DLINECAPS_ZTEST    |
                    D3DLINECAPS_BLEND    |
                    D3DLINECAPS_ALPHACMP |
                    D3DLINECAPS_FOG      |
                    D3DLINECAPS_ANTIALIAS;

      c->MaxTextureWidth       = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
      c->MaxTextureHeight      = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
      c->MaxVolumeExtent       = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
      c->MaxTextureRepeat      = 8192;
      c->MaxTextureAspectRatio = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
      c->MaxAnisotropy         = 16;
      c->MaxVertexW            = 1e10f;

      c->GuardBandLeft   = -32768.0f;
      c->GuardBandTop    = -32768.0f;
      c->GuardBandRight  = 32768.0f;
      c->GuardBandBottom = 32768.0f;

      c->StencilCaps = D3DSTENCILCAPS_KEEP    |
                       D3DSTENCILCAPS_ZERO    |
                       D3DSTENCILCAPS_REPLACE |
                       D3DSTENCILCAPS_INCRSAT |
                       D3DSTENCILCAPS_DECRSAT |
                       D3DSTENCILCAPS_INVERT  |
                       D3DSTENCILCAPS_INCR    |
                       D3DSTENCILCAPS_DECR    |
                       D3DSTENCILCAPS_TWOSIDED;

      c->FVFCaps                 = 8 | D3DFVFCAPS_PSIZE;
      c->TextureOpCaps           = 0x03FEFFFF;
      c->MaxTextureBlendStages   = 8;
      c->MaxSimultaneousTextures = 8;

      c->VertexProcessingCaps = D3DVTXPCAPS_TEXGEN            |
                                D3DVTXPCAPS_MATERIALSOURCE7   |
                                D3DVTXPCAPS_DIRECTIONALLIGHTS |
                                D3DVTXPCAPS_POSITIONALLIGHTS  |
                                D3DVTXPCAPS_LOCALVIEWER       |
                                D3DVTXPCAPS_TWEENING          |
                                D3DVTXPCAPS_TEXGEN_SPHEREMAP;

      // We don't emulate user clip planes, so let's not claim any.
      //
      c->MaxActiveLights          = 8;
      c->MaxUserClipPlanes        = 0;
      c->MaxVertexBlendMatrices   = 4;
      c->MaxPointSize             = 256.0f;
      c->MaxPrimitiveCount        = 0xFFFFFF;
      c->MaxVertexIndex           = 0xFFFFFF;
      c->MaxStreams               = 16;
      c->MaxStreamStride          = 508;
      c->VertexShaderVersion      = D3DVS_VERSION (3, 0);
      c->MaxVertexShaderConst     = 256;
      c->PixelShaderVersion       = D3DPS_VERSION (3, 0);
      c->PixelShader1xMaxValue    = 65504.0f;

      c->DevCaps2 = D3DDEVCAPS2_STREAMOFFSET                       |
                    D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET |
                    D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES;

      c->NumberOfAdaptersInGroup = 1;

      c->DeclTypes = D3DDTCAPS_UBYTE4    |
                     D3DDTCAPS_UBYTE4N   |
                     D3DDTCAPS_SHORT2N   |
                     D3DDTCAPS_SHORT4N   |
                     D3DDTCAPS_USHORT2N  |
                     D3DDTCAPS_USHORT4N  |
                     D3DDTCAPS_UDEC3     |
                     D3DDTCAPS_DEC3N     |
                     D3DDTCAPS_FLOAT16_2 |
                     D3DDTCAPS_FLOAT16_4;

      c->NumSimultaneousRTs = 4;

      // We only resolve and copy in StretchRect() for now, which ignores
      // the filter. Point filtering is the honest answer.
      //
      c->StretchRectFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MAGFPOINT;

      c->VS20Caps.Caps                     = D3DVS20CAPS_PREDICATION;
      c->VS20Caps.DynamicFlowControlDepth  = D3DVS20_MAX_DYNAMICFLOWCONTROLDEPTH;
      c->VS20Caps.NumTemps                 = D3DVS20_MAX_NUMTEMPS;
      c->VS20Caps.StaticFlowControlDepth   = D3DVS20_MAX_STATICFLOWCONTROLDEPTH;

      c->PS20Caps.Caps                     = D3DPS20CAPS_ARBITRARYSWIZZLE      |
                                             D3DPS20CAPS_GRADIENTINSTRUCTIONS  |
                                             D3DPS20CAPS_PREDICATION           |
                                             D3DPS20CAPS_NODEPENDENTREADLIMIT  |
                                             D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT;
      c->PS20Caps.DynamicFlowControlDepth  = D3DPS20_MAX_DYNAMICFLOWCONTROLDEPTH;
      c->PS20Caps.NumTemps                 = D3DPS20_MAX_NUMTEMPS;
      c->PS20Caps.StaticFlowControlDepth   = D3DPS20_MAX_STATICFLOWCONTROLDEPTH;
      c->PS20Caps.NumInstructionSlots      = D3DPS20_MAX_NUMINSTRUCTIONSLOTS;

      c->VertexTextureFilterCaps = D3DPTFILTERCAPS_MINFPOINT  |
                                   D3DPTFILTERCAPS_MINFLINEAR |
                                   D3DPTFILTERCAPS_MAGFPOINT  |
                                   D3DPTFILTERCAPS_MAGFLINEAR;

      c->MaxVShaderInstructionsExecuted    = 0xFFFFFFFF;
      c->MaxPShaderInstructionsExecuted    = 0xFFFFFFFF;
      c->MaxVertexShader30InstructionSlots = D3DMAX30SHADERINSTRUCTIONS;
      c->MaxPixelShader30InstructionSlots  = D3DMAX30SHADERINSTRUCTIONS;
      return D3D_OK;
    }

    HMONITOR direct3d::
    GetAdapterMonitor (UINT a)
    {
      if (a >= adapters_.size () || adapters_[a].output == nullptr)
        return MonitorFromWindow (nullptr, MONITOR_DEFAULTTOPRIMARY);

      DXGI_OUTPUT_DESC od;
      return SUCCEEDED (adapters_[a].output->GetDesc (&od)) ? od.Monitor : nullptr;
    }

    HRESULT direct3d::
    CreateDevice (UINT a, D3DDEVTYPE t, HWND focus, DWORD behavior,
                  D3DPRESENT_PARAMETERS* pp, IDirect3DDevice9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if (pp == nullptr || a >= adapters_.size ())
        return D3DERR_INVALIDCALL;

      if (t != D3DDEVTYPE_HAL)
        return D3DERR_NOTAVAILABLE;

      device* d (new device (*this, adapters_[a].adapter, focus, behavior, *pp));

      if (HRESULT hr = d->create (a, t); FAILED (hr))
      {
        d->Release ();
        return hr;
      }

      // D3D9 fills in the back buffer size and format it picked if the game
      // left them out.
      //
      ComPtr<IDirect3DSwapChain9> sc;
      if (SUCCEEDED (d->GetSwapChain (0, &sc)))
        sc->GetPresentParameters (pp);

      *r = d;
      return D3D_OK;
    }
  }
}
