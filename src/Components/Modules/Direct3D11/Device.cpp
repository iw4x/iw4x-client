#include "Device.hpp"

#include "Direct3D.hpp"

#include <algorithm> // min(), max(), ranges::fill()
#include <bit>       // bit_cast(), bit_width()
#include <cstring>   // memcpy(), memset()
#include <limits>    // numeric_limits

using namespace std;

namespace Components
{
  namespace d3d11
  {
    HRESULT
    get_device (device& d, IDirect3DDevice9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      d.AddRef ();
      *r = &d;
      return D3D_OK;
    }

    namespace
    {
      // Return the number of indices (or vertices) that the primitive
      // count covers. Return 0 for the types that D3D11 can't draw.
      //
      UINT
      vertex_count (D3DPRIMITIVETYPE t, UINT n) noexcept
      {
        switch (t)
        {
          case D3DPT_POINTLIST:     return n;
          case D3DPT_LINELIST:      return n * 2;
          case D3DPT_LINESTRIP:     return n + 1;
          case D3DPT_TRIANGLELIST:  return n * 3;
          case D3DPT_TRIANGLESTRIP: return n + 2;
          default:                  return 0;
        }
      }

      // D3D11 numbers the topologies the same as D3D9 numbers its
      // primitive types, triangle fans aside (which D3D11 doesn't have).
      //
      D3D11_PRIMITIVE_TOPOLOGY
      topology (D3DPRIMITIVETYPE t) noexcept
      {
        return static_cast<D3D11_PRIMITIVE_TOPOLOGY> (t);
      }

      void
      unpack_color (D3DCOLOR c, float (&r)[4]) noexcept
      {
        r[0] = ((c >> 16) & 0xFF) / 255.0f;
        r[1] = ((c >> 8)  & 0xFF) / 255.0f;
        r[2] = ( c        & 0xFF) / 255.0f;
        r[3] = ((c >> 24) & 0xFF) / 255.0f;
      }

      // Return the number of bits of the depth buffer's precision, which
      // is what D3D11 multiplies the depth bias by (see rasterizer_desc()).
      // For the float format the precision depends on the depth itself,
      // so we use the mantissa's bits, which is what it is around one.
      //
      UINT
      depth_bits (D3DFORMAT f) noexcept
      {
        switch (f)
        {
          case D3DFMT_D16:           return 16;
          case D3DFMT_D32F_LOCKABLE: return 23;
          default:                   return 24;
        }
      }

      const D3DMATRIX identity {1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f};
    }

    // swap_chain
    //
    swap_chain::
    swap_chain (device& d, HWND w, const D3DPRESENT_PARAMETERS& p)
      : device_ (d), window_ (w), parameters_ (p)
    {
    }

    HRESULT swap_chain::
    create (IDXGIFactory2* f)
    {
      // D3D9 takes the window's client area for the back buffer size that
      // the game leaves out.
      //
      if (parameters_.BackBufferWidth == 0 || parameters_.BackBufferHeight == 0)
      {
        RECT r {};
        GetClientRect (window_, &r);

        if (parameters_.BackBufferWidth == 0)
          parameters_.BackBufferWidth = max<LONG> (r.right - r.left, 1);

        if (parameters_.BackBufferHeight == 0)
          parameters_.BackBufferHeight = max<LONG> (r.bottom - r.top, 1);
      }

      if (parameters_.BackBufferFormat == D3DFMT_UNKNOWN)
        parameters_.BackBufferFormat = D3DFMT_X8R8G8B8;

      if (parameters_.BackBufferCount == 0)
        parameters_.BackBufferCount = 1;

      if (HRESULT hr = create_back_buffer (); FAILED (hr))
        return hr;

      // Let's have tearing if the system allows it, which is what the
      // immediate present interval means in D3D9 (and the only way to get
      // it with the flip model in a window).
      //
      ComPtr<IDXGIFactory5> f5;
      if (SUCCEEDED (f->QueryInterface (IID_PPV_ARGS (&f5))))
      {
        BOOL t (FALSE);
        tearing_ = SUCCEEDED (f5->CheckFeatureSupport (DXGI_FEATURE_PRESENT_ALLOW_TEARING, &t, sizeof (t))) && t;
      }

      DXGI_SWAP_CHAIN_DESC1 d {};
      d.Width            = parameters_.BackBufferWidth;
      d.Height           = parameters_.BackBufferHeight;
      d.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
      d.SampleDesc.Count = 1;
      d.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      d.BufferCount      = 2;
      d.Scaling          = DXGI_SCALING_STRETCH;
      d.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      d.AlphaMode        = DXGI_ALPHA_MODE_IGNORE;
      d.Flags            = tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

      flip_ = true;

      // Fall back to the blit model on systems that predate the flip one.
      //
      if (FAILED (f->CreateSwapChainForHwnd (device_.d3d (), window_, &d, nullptr, nullptr, &swap_)))
      {
        d.BufferCount = 1;
        d.SwapEffect  = DXGI_SWAP_EFFECT_DISCARD;
        d.Flags       = 0;
        flip_         = false;
        tearing_      = false;

        if (FAILED (f->CreateSwapChainForHwnd (device_.d3d (), window_, &d, nullptr, nullptr, &swap_)))
          return D3DERR_NOTAVAILABLE;
      }

      // IW4 handles Alt+Enter itself (with vid_restart).
      //
      f->MakeWindowAssociation (window_, DXGI_MWA_NO_ALT_ENTER);
      return D3D_OK;
    }

    HRESULT swap_chain::
    create_back_buffer ()
    {
      // The DXGI back buffer is B8G8R8A8, which our back buffer has to
      // match for Present() to copy it. So an X8R8G8B8 back buffer is an
      // A8R8G8B8 one underneath, which nobody can tell apart since
      // nothing samples the back buffer.
      //
      image_desc d {};
      d.type     = image_type::surface;
      d.width    = parameters_.BackBufferWidth;
      d.height   = parameters_.BackBufferHeight;
      d.depth    = 1;
      d.levels   = 1;
      d.usage    = D3DUSAGE_RENDERTARGET;
      d.pool     = D3DPOOL_DEFAULT;
      d.format   = D3DFMT_A8R8G8B8;
      d.samples  = parameters_.MultiSampleType;
      d.quality  = parameters_.MultiSampleQuality;
      d.lockable = (parameters_.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER) != 0;

      unique_ptr<image> i;
      if (HRESULT hr = device_.create_image (d, i); FAILED (hr))
        return hr;

      surface* s (new surface (device_, move (i)));
      back_ = private_ref<surface> (s);
      s->Release ();
      return D3D_OK;
    }

    HRESULT swap_chain::
    reset (const D3DPRESENT_PARAMETERS& p)
    {
      const D3DPRESENT_PARAMETERS old (parameters_);
      parameters_ = p;

      if (parameters_.BackBufferWidth == 0 || parameters_.BackBufferHeight == 0)
      {
        RECT r {};
        GetClientRect (window_, &r);

        if (parameters_.BackBufferWidth == 0)
          parameters_.BackBufferWidth = max<LONG> (r.right - r.left, 1);

        if (parameters_.BackBufferHeight == 0)
          parameters_.BackBufferHeight = max<LONG> (r.bottom - r.top, 1);
      }

      if (parameters_.BackBufferFormat == D3DFMT_UNKNOWN)
        parameters_.BackBufferFormat = D3DFMT_X8R8G8B8;

      if (parameters_.BackBufferCount == 0)
        parameters_.BackBufferCount = 1;

      if (back_)
        device_.unbind (back_.get ());

      back_ = private_ref<surface> ();

      // ResizeBuffers() fails while anything references the DXGI buffers,
      // which includes the context's bindings. We only ever touch them in
      // Present() but DXGI also wants the deferred destruction done.
      //
      device_.context ()->ClearState ();
      device_.context ()->Flush ();

      if (FAILED (swap_->ResizeBuffers (0,
                                        parameters_.BackBufferWidth,
                                        parameters_.BackBufferHeight,
                                        DXGI_FORMAT_UNKNOWN,
                                        tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)))
      {
        parameters_ = old;
        create_back_buffer ();
        return D3DERR_DEVICELOST;
      }

      return create_back_buffer ();
    }

    UINT swap_chain::
    sync_interval () const noexcept
    {
      switch (parameters_.PresentationInterval)
      {
        case D3DPRESENT_INTERVAL_IMMEDIATE: return 0;
        case D3DPRESENT_INTERVAL_TWO:       return 2;
        case D3DPRESENT_INTERVAL_THREE:     return 3;
        case D3DPRESENT_INTERVAL_FOUR:      return 4;
        default:                            return 1;
      }
    }

    HRESULT swap_chain::
    Present (const RECT*, const RECT*, HWND, const RGNDATA*, DWORD)
    {
      lock_guard l (device_.mutex ());

      // With the flip model, buffer 0 is always the one to render to next.
      //
      ComPtr<ID3D11Texture2D> b;
      if (FAILED (swap_->GetBuffer (0, IID_PPV_ARGS (&b))))
        return D3DERR_DEVICELOST;

      ID3D11DeviceContext1* c (device_.context ());
      image& i (back_->storage ());

      if (i.desc ().samples >= D3DMULTISAMPLE_2_SAMPLES)
        c->ResolveSubresource (b.Get (), 0, i.resource (), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
      else
        c->CopySubresourceRegion (b.Get (), 0, 0, 0, 0, i.resource (), 0, nullptr);

      b.Reset ();

      const UINT n (sync_interval ());
      const UINT f (n == 0 && tearing_ ? DXGI_PRESENT_ALLOW_TEARING : 0);

      const HRESULT hr (swap_->Present (n, f));

      if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
      {
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Direct3D 11 device lost ({:#x})\n",
                         static_cast<uint32_t> (device_.d3d ()->GetDeviceRemovedReason ()));
        return D3DERR_DEVICELOST;
      }

      return D3D_OK;
    }

    HRESULT swap_chain::
    GetFrontBufferData (IDirect3DSurface9*)
    {
      device_.unsupported ("GetFrontBufferData()");
      return D3DERR_INVALIDCALL;
    }

    HRESULT swap_chain::
    GetBackBuffer (UINT i, D3DBACKBUFFER_TYPE, IDirect3DSurface9** r)
    {
      if (r == nullptr || i >= parameters_.BackBufferCount)
        return D3DERR_INVALIDCALL;

      // We only have one back buffer whatever the count, which the game
      // can't tell apart since it only ever renders to the current one.
      //
      back_->AddRef ();
      *r = back_.get ();
      return D3D_OK;
    }

    HRESULT swap_chain::
    GetRasterStatus (D3DRASTER_STATUS* r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      r->InVBlank = FALSE;
      r->ScanLine = 0;
      return D3D_OK;
    }

    HRESULT swap_chain::
    GetDisplayMode (D3DDISPLAYMODE* r)
    {
      return device_.GetDisplayMode (0, r);
    }

    HRESULT swap_chain::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    HRESULT swap_chain::
    GetPresentParameters (D3DPRESENT_PARAMETERS* r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = parameters_;
      return D3D_OK;
    }

    // query
    //
    query::
    query (device& d, D3DQUERYTYPE t)
      : device_ (d), type_ (t)
    {
    }

    HRESULT query::
    create ()
    {
      D3D11_QUERY_DESC d {};

      switch (type_)
      {
        case D3DQUERYTYPE_EVENT:     d.Query = D3D11_QUERY_EVENT;     break;
        case D3DQUERYTYPE_OCCLUSION: d.Query = D3D11_QUERY_OCCLUSION; break;
        default:                     return D3DERR_NOTAVAILABLE;
      }

      return SUCCEEDED (device_.d3d ()->CreateQuery (&d, &query_))
             ? D3D_OK
             : D3DERR_NOTAVAILABLE;
    }

    HRESULT query::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    D3DQUERYTYPE query::
    GetType ()
    {
      return type_;
    }

    DWORD query::
    GetDataSize ()
    {
      return type_ == D3DQUERYTYPE_EVENT ? sizeof (BOOL) : sizeof (DWORD);
    }

    HRESULT query::
    Issue (DWORD f)
    {
      lock_guard l (device_.mutex ());
      ID3D11DeviceContext1* c (device_.context ());

      // Events only have an end, which D3D9 lets the game issue as either.
      //
      if ((f & D3DISSUE_BEGIN) != 0 && type_ != D3DQUERYTYPE_EVENT)
        c->Begin (query_.Get ());

      if ((f & D3DISSUE_END) != 0)
      {
        c->End (query_.Get ());
        issued_ = true;
      }

      return D3D_OK;
    }

    HRESULT query::
    GetData (void* data, DWORD size, DWORD f)
    {
      if (size != 0 && (data == nullptr || size < GetDataSize ()))
        return D3DERR_INVALIDCALL;

      // D3D9 reports a query that was never issued as done, with nothing
      // drawn.
      //
      if (!issued_)
      {
        if (size != 0)
        {
          if (type_ == D3DQUERYTYPE_EVENT)
            *static_cast<BOOL*> (data) = TRUE;
          else
            *static_cast<DWORD*> (data) = 0;
        }

        return S_OK;
      }

      lock_guard l (device_.mutex ());

      const UINT flags ((f & D3DGETDATA_FLUSH) != 0 ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH);
      HRESULT hr;

      if (type_ == D3DQUERYTYPE_EVENT)
      {
        BOOL v (FALSE);
        hr = device_.context ()->GetData (query_.Get (), &v, sizeof (v), flags);

        if (hr == S_OK && size != 0)
          *static_cast<BOOL*> (data) = v;
      }
      else
      {
        UINT64 v (0);
        hr = device_.context ()->GetData (query_.Get (), &v, sizeof (v), flags);

        if (hr == S_OK && size != 0)
          *static_cast<DWORD*> (data) = static_cast<DWORD> (min<UINT64> (v, numeric_limits<DWORD>::max ()));
      }

      if (hr == S_OK)
        return S_OK;

      if (hr == S_FALSE)
        return S_FALSE;

      return D3DERR_DEVICELOST;
    }

    // device
    //
    device::
    device (direct3d& p,
            ComPtr<IDXGIAdapter1> a,
            HWND focus,
            DWORD behavior,
            const D3DPRESENT_PARAMETERS& pp)
      : parent_ (p),
        adapter_ (move (a)),
        focus_ (focus),
        behavior_ (behavior),
        parameters_ (pp)
    {
      parent_.AddRef ();
    }

    device::
    ~device ()
    {
      // Drop our bindings before the D3D11 device goes away, since they
      // may be the last references to the resources.
      //
      for (bound_texture& t: textures_)
        t = bound_texture ();

      for (stream& s: streams_)
        s = stream ();

      targets_.fill (private_ref<surface> ());
      depth_       = private_ref<surface> ();
      auto_depth_  = private_ref<surface> ();
      vs_          = private_ref<vertex_shader> ();
      ps_          = private_ref<pixel_shader> ();
      decl_        = private_ref<vertex_declaration> ();
      indices_     = private_ref<index_buffer> ();
      swap_        = private_ref<swap_chain> ();

      if (context_ != nullptr)
        context_->ClearState ();

      parent_.Release ();
    }

    HRESULT device::
    create (UINT a, D3DDEVTYPE t)
    {
      adapter_index_ = a;
      type_          = t;

      // Create the device. Feature level 11 is what dexilifier compiles
      // the shaders for, so we need nothing more.
      //
      {
        const D3D_FEATURE_LEVEL levels[] {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

        UINT flags (D3D11_CREATE_DEVICE_BGRA_SUPPORT);

        // D3D9's multithreaded device does its own locking and so do we,
        // so D3D11 needn't.
        //
        if ((behavior_ & D3DCREATE_MULTITHREADED) == 0)
          flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

        if (debug_layer ())
          flags |= D3D11_CREATE_DEVICE_DEBUG;

        ComPtr<ID3D11Device>        d;
        ComPtr<ID3D11DeviceContext> c;

        HRESULT hr (D3D11CreateDevice (adapter_.Get (),
                                       D3D_DRIVER_TYPE_UNKNOWN,
                                       nullptr,
                                       flags,
                                       levels,
                                       static_cast<UINT> (size (levels)),
                                       D3D11_SDK_VERSION,
                                       &d,
                                       nullptr,
                                       &c));

        // The debug layer is an optional part of the SDK, so let's not
        // fail without it.
        //
        if (FAILED (hr) && (flags & D3D11_CREATE_DEVICE_DEBUG) != 0)
        {
          Logger::Warning (Game::CON_CHANNEL_GFX, "Direct3D 11 debug layer is not available\n");

          flags &= ~D3D11_CREATE_DEVICE_DEBUG;
          hr = D3D11CreateDevice (adapter_.Get (),
                                  D3D_DRIVER_TYPE_UNKNOWN,
                                  nullptr,
                                  flags,
                                  levels,
                                  static_cast<UINT> (size (levels)),
                                  D3D11_SDK_VERSION,
                                  &d,
                                  nullptr,
                                  &c);
        }

        if (FAILED (hr))
        {
          Logger::Warning (Game::CON_CHANNEL_GFX,
                           "Couldn't create a Direct3D 11 device ({:#x})\n",
                           static_cast<uint32_t> (hr));
          return D3DERR_NOTAVAILABLE;
        }

        if (FAILED (d.As (&device_)) || FAILED (c.As (&context_)))
        {
          Logger::Warning (Game::CON_CHANNEL_GFX, "Direct3D 11.1 is not available\n");
          return D3DERR_NOTAVAILABLE;
        }

        if (FAILED (adapter_->GetParent (IID_PPV_ARGS (&factory_))))
          return D3DERR_NOTAVAILABLE;
      }

      // Create the constant buffers, the register files first (see
      // stage_constants), then Direct3D9State.
      //
      {
        auto make ([this] (UINT n, ComPtr<ID3D11Buffer>& r)
        {
          D3D11_BUFFER_DESC d {};
          d.ByteWidth      = n;
          d.Usage          = D3D11_USAGE_DYNAMIC;
          d.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
          d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

          return SUCCEEDED (device_->CreateBuffer (&d, nullptr, &r));
        });

        if (!make (vertex_floats * 16,    vertex_buffers_[0]) ||
            !make (integers * 16,         vertex_buffers_[1]) ||
            !make (booleans * 4,          vertex_buffers_[2]) ||
            !make (sizeof (vertex_state), vertex_buffers_[3]) ||
            !make (pixel_floats * 16,     pixel_buffers_[0])  ||
            !make (integers * 16,         pixel_buffers_[1])  ||
            !make (booleans * 4,          pixel_buffers_[2])  ||
            !make (sizeof (pixel_state),  pixel_buffers_[3]))
          return D3DERR_OUTOFVIDEOMEMORY;
      }

      // Create the zeros that feed the inputs the declaration lacks (see
      // absent_input).
      //
      {
        const UINT zeros[4] {};

        D3D11_BUFFER_DESC d {};
        d.ByteWidth = sizeof (zeros);
        d.Usage     = D3D11_USAGE_IMMUTABLE;
        d.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        const D3D11_SUBRESOURCE_DATA s {zeros, 0, 0};

        if (FAILED (device_->CreateBuffer (&d, &s, &absent_)))
          return D3DERR_OUTOFVIDEOMEMORY;
      }

      // Create the implicit swap chain and depth stencil.
      //
      {
        HWND w (parameters_.hDeviceWindow != nullptr ? parameters_.hDeviceWindow : focus_);

        if (w == nullptr)
          return D3DERR_INVALIDCALL;

        if (parameters_.Windowed == FALSE)
          unsupported ("exclusive fullscreen (rendering in a window instead)");

        swap_chain* s (new swap_chain (*this, w, parameters_));

        if (HRESULT hr = s->create (factory_.Get ()); FAILED (hr))
        {
          s->Release ();
          return hr;
        }

        swap_ = private_ref<swap_chain> (s);
        s->Release ();

        parameters_ = swap_->parameters ();

        if (HRESULT hr = create_auto_depth (); FAILED (hr))
          return hr;
      }

      reset_state ();
      return D3D_OK;
    }

    bool device::
    debug_layer () noexcept
    {
      // There is no dvar this early (the device is created before the
      // renderer registers its dvars), so the environment it is.
      //
      char v[8];
      const DWORD n (GetEnvironmentVariableA ("IW4X_D3D11_DEBUG", v, sizeof (v)));
      return n != 0 && n < sizeof (v) && v[0] != '0';
    }

    HRESULT device::
    create_auto_depth ()
    {
      auto_depth_ = private_ref<surface> ();

      if (parameters_.EnableAutoDepthStencil == FALSE)
        return D3D_OK;

      image_desc d {};
      d.type    = image_type::surface;
      d.width   = parameters_.BackBufferWidth;
      d.height  = parameters_.BackBufferHeight;
      d.depth   = 1;
      d.levels  = 1;
      d.usage   = D3DUSAGE_DEPTHSTENCIL;
      d.pool    = D3DPOOL_DEFAULT;
      d.format  = parameters_.AutoDepthStencilFormat;
      d.samples = parameters_.MultiSampleType;
      d.quality = parameters_.MultiSampleQuality;

      IDirect3DSurface9* s;
      if (HRESULT hr = create_surface (d, &s); FAILED (hr))
        return hr;

      auto_depth_ = private_ref<surface> (surface_of (s));
      s->Release ();
      return D3D_OK;
    }

    void device::
    unsupported (const char* what)
    {
      lock_guard l (mutex_);

      if (unsupported_.insert (what).second)
        Logger::Warning (Game::CON_CHANNEL_GFX, "Direct3D 11: unsupported {}\n", what);
    }

    void device::
    unbind (surface* s)
    {
      lock_guard l (mutex_);

      for (private_ref<surface>& t: targets_)
      {
        if (t.get () == s)
        {
          t = private_ref<surface> ();
          dirty_ |= dirty_targets;
        }
      }

      if (depth_.get () == s)
      {
        depth_ = private_ref<surface> ();
        dirty_ |= dirty_targets;
      }
    }

    UINT device::
    sampler_index (DWORD s) noexcept
    {
      if (s < pixel_samplers)
        return s;

      if (s >= D3DVERTEXTEXTURESAMPLER0 && s <= D3DVERTEXTEXTURESAMPLER3)
        return pixel_samplers + (s - D3DVERTEXTEXTURESAMPLER0);

      return samplers;
    }

    void device::
    reset_state ()
    {
      lock_guard l (mutex_);

      default_render_states (rs_, auto_depth_ ? true : false);

      for (sampler_states& s: ss_)
        default_sampler_states (s);

      for (bound_texture& t: textures_)
        t = bound_texture ();

      for (stream& s: streams_)
        s = stream ();

      targets_.fill (private_ref<surface> ());
      targets_[0] = private_ref<surface> (swap_->back_buffer ());
      depth_      = auto_depth_;
      vs_         = private_ref<vertex_shader> ();
      ps_         = private_ref<pixel_shader> ();
      decl_       = private_ref<vertex_declaration> ();
      indices_    = private_ref<index_buffer> ();
      fvf_        = 0;

      const UINT w (parameters_.BackBufferWidth);
      const UINT h (parameters_.BackBufferHeight);

      viewport_ = D3DVIEWPORT9 {0, 0, w, h, 0.0f, 1.0f};
      scissor_  = RECT {0, 0, static_cast<LONG> (w), static_cast<LONG> (h)};

      vertex_ = stage_constants {};
      pixel_  = stage_constants {};
      vertex_.dirty_floats = vertex_.dirty_ints = vertex_.dirty_bools = true;
      pixel_.dirty_floats  = pixel_.dirty_ints  = pixel_.dirty_bools  = true;

      // Reset what we last sent so that the next draw sends everything.
      //
      layout_             = nullptr;
      topology_           = static_cast<D3DPRIMITIVETYPE> (0);
      vertex_state_valid_ = false;
      pixel_state_valid_  = false;
      views_.fill (nullptr);
      sampler_objects_.fill (nullptr);
      texture_dirty_      = ~0u;
      sampler_dirty_      = ~0u;
      dirty_              = dirty_all;

      context_->ClearState ();

      // The constant buffers and the absent input never change, so we bind
      // them once.
      //
      ID3D11Buffer* const vb[4] {vertex_buffers_[0].Get (),
                                 vertex_buffers_[1].Get (),
                                 vertex_buffers_[2].Get (),
                                 vertex_buffers_[3].Get ()};

      ID3D11Buffer* const pb[4] {pixel_buffers_[0].Get (),
                                 pixel_buffers_[1].Get (),
                                 pixel_buffers_[2].Get (),
                                 pixel_buffers_[3].Get ()};

      context_->VSSetConstantBuffers (0, 4, vb);
      context_->PSSetConstantBuffers (0, 4, pb);

      ID3D11Buffer* const a (absent_.Get ());
      const UINT zero (0);
      context_->IASetVertexBuffers (absent_slot, 1, &a, &zero, &zero);
    }

    pair<UINT, UINT> device::
    target_size () const noexcept
    {
      if (surface* s = targets_[0].get ())
        return {s->width (), s->height ()};

      if (surface* s = depth_.get ())
        return {s->width (), s->height ()};

      return {parameters_.BackBufferWidth, parameters_.BackBufferHeight};
    }

    void device::
    apply_targets ()
    {
      const bool srgb (rs_[D3DRS_SRGBWRITEENABLE] != FALSE);

      ID3D11RenderTargetView* rtv[targets] {};

      for (UINT i (0); i != targets; ++i)
      {
        if (surface* s = targets_[i].get ())
        {
          if (!s->storage ().null ())
            rtv[i] = s->target_view (srgb);
        }
      }

      // D3D9 lets the depth stencil be larger than the render target (and
      // uses its top left corner) while D3D11 wants them alike. A larger
      // one works in practice, but one that is smaller or has another
      // sample count can't, so we draw without depth then (as D3D9 would
      // refuse the draw anyway).
      //
      ID3D11DepthStencilView* dsv (nullptr);

      if (surface* d = depth_.get ())
      {
        surface* t (targets_[0].get ());

        if (t != nullptr &&
            (d->width () < t->width ()    ||
             d->height () < t->height ()  ||
             d->storage ().desc ().samples != t->storage ().desc ().samples))
          unsupported ("depth stencil that doesn't match the render target");
        else
          dsv = d->depth_view ();
      }

      context_->OMSetRenderTargets (targets, rtv, dsv);

      // A texture that is now a target can't stay bound for sampling, so
      // let's look at the bindings again.
      //
      texture_dirty_ = ~0u;
    }

    void device::
    apply_viewport ()
    {
      D3D11_VIEWPORT v;
      v.TopLeftX = static_cast<float> (viewport_.X);
      v.TopLeftY = static_cast<float> (viewport_.Y);
      v.Width    = static_cast<float> (viewport_.Width);
      v.Height   = static_cast<float> (viewport_.Height);
      v.MinDepth = viewport_.MinZ;
      v.MaxDepth = max (viewport_.MinZ, viewport_.MaxZ);

      context_->RSSetViewports (1, &v);

      // Move the positions by half a pixel so that D3D9's pixel centers
      // land on D3D11's (see the shaders' positionFixup).
      //
      const float w (viewport_.Width != 0 ? static_cast<float> (viewport_.Width) : 1.0f);
      const float h (viewport_.Height != 0 ? static_cast<float> (viewport_.Height) : 1.0f);

      vertex_state_.positionFixup[0] = 1.0f / w;
      vertex_state_.positionFixup[1] = -1.0f / h;
      vertex_state_.positionFixup[2] = 0.0f;
      vertex_state_.positionFixup[3] = 0.0f;
      vertex_state_valid_ = false;
    }

    void device::
    upload (ID3D11Buffer* b, const void* data, size_t n)
    {
      D3D11_MAPPED_SUBRESOURCE m;
      if (SUCCEEDED (context_->Map (b, 0, D3D11_MAP_WRITE_DISCARD, 0, &m)))
      {
        memcpy (m.pData, data, n);
        context_->Unmap (b, 0);
      }
    }

    void device::
    apply_constants ()
    {
      auto stage ([this] (stage_constants& s, array<ComPtr<ID3D11Buffer>, 4>& bs, UINT floats)
      {
        if (s.dirty_floats)
        {
          upload (bs[0].Get (), s.floats.data (), floats * 16);
          s.dirty_floats = false;
        }

        if (s.dirty_ints)
        {
          upload (bs[1].Get (), s.ints.data (), sizeof (s.ints));
          s.dirty_ints = false;
        }

        if (s.dirty_bools)
        {
          upload (bs[2].Get (), s.bools.data (), sizeof (s.bools));
          s.dirty_bools = false;
        }
      });

      stage (vertex_, vertex_buffers_, vertex_floats);
      stage (pixel_, pixel_buffers_, pixel_floats);

      if (!vertex_state_valid_)
      {
        upload (vertex_buffers_[3].Get (), &vertex_state_, sizeof (vertex_state_));
        vertex_state_valid_ = true;
      }

      if (!pixel_state_valid_)
      {
        upload (pixel_buffers_[3].Get (), &pixel_state_, sizeof (pixel_state_));
        pixel_state_valid_ = true;
      }
    }

    bool device::
    prepare (D3DPRIMITIVETYPE t)
    {
      // We only run the game's shaders, there is no fixed function
      // pipeline (IW4 has none either).
      //
      if (!vs_ || !ps_)
      {
        unsupported ("draw without shaders");
        return false;
      }

      if (vs_->shader == nullptr || ps_->shader == nullptr)
        return false;

      if (!decl_)
      {
        unsupported ("draw without vertex declaration");
        return false;
      }

      if (vertex_count (t, 1) == 0)
      {
        unsupported ("triangle fans");
        return false;
      }

      if ((dirty_ & dirty_targets) != 0)
        apply_targets ();

      if ((dirty_ & dirty_viewport) != 0)
        apply_viewport ();

      if ((dirty_ & dirty_scissor) != 0)
      {
        const D3D11_RECT r {scissor_.left, scissor_.top, scissor_.right, scissor_.bottom};
        context_->RSSetScissorRects (1, &r);
      }

      if ((dirty_ & dirty_shaders) != 0)
      {
        context_->VSSetShader (vs_->shader.Get (), nullptr, 0);
        context_->PSSetShader (ps_->shader.Get (), nullptr, 0);
      }

      if ((dirty_ & dirty_layout) != 0)
      {
        const input_layout* il (layouts_.find (device_.Get (), *decl_.get (), *vs_.get ()));
        if (il == nullptr)
          return false;

        if (il != layout_)
        {
          context_->IASetInputLayout (il->layout.Get ());
          layout_ = il;

          if (!equal (begin (il->types), end (il->types), begin (vertex_state_.inputTypes)))
          {
            copy (begin (il->types), end (il->types), begin (vertex_state_.inputTypes));
            vertex_state_valid_ = false;
          }
        }
      }

      if (t != topology_)
      {
        context_->IASetPrimitiveTopology (topology (t));
        topology_ = t;
      }

      if ((dirty_ & dirty_streams) != 0)
      {
        ID3D11Buffer* bs[streams];
        UINT          strides[streams];
        UINT          offsets[streams];

        for (UINT i (0); i != streams; ++i)
        {
          const stream& s (streams_[i]);
          bs[i]      = s.buffer ? s.buffer->storage.get () : nullptr;
          strides[i] = s.stride;
          offsets[i] = s.offset;
        }

        context_->IASetVertexBuffers (0, streams, bs, strides, offsets);
      }

      if ((dirty_ & dirty_indices) != 0)
      {
        if (indices_)
          context_->IASetIndexBuffer (indices_->storage.get (), indices_->format (), 0);
        else
          context_->IASetIndexBuffer (nullptr, DXGI_FORMAT_R16_UINT, 0);
      }

      if ((dirty_ & dirty_blend) != 0)
      {
        ID3D11BlendState* s (blend_states_.find (blend_desc (rs_),
                                                 [this] (const D3D11_BLEND_DESC& d, ID3D11BlendState** r)
        {
          return device_->CreateBlendState (&d, r);
        }));

        float f[4];
        unpack_color (rs_[D3DRS_BLENDFACTOR], f);
        context_->OMSetBlendState (s, f, rs_[D3DRS_MULTISAMPLEMASK]);
      }

      if ((dirty_ & dirty_depth) != 0)
      {
        ID3D11DepthStencilState* s (depth_states_.find (depth_stencil_desc (rs_),
                                                        [this] (const D3D11_DEPTH_STENCIL_DESC& d, ID3D11DepthStencilState** r)
        {
          return device_->CreateDepthStencilState (&d, r);
        }));

        context_->OMSetDepthStencilState (s, rs_[D3DRS_STENCILREF] & 0xFF);
      }

      if ((dirty_ & dirty_raster) != 0)
      {
        const UINT bits (depth_ ? depth_bits (depth_->storage ().desc ().format) : 24);

        ID3D11RasterizerState* s (raster_states_.find (rasterizer_desc (rs_, bits),
                                                       [this] (const D3D11_RASTERIZER_DESC& d, ID3D11RasterizerState** r)
        {
          return device_->CreateRasterizerState (&d, r);
        }));

        context_->RSSetState (s);
      }

      // Bind the textures and samplers that changed. Note that the depth
      // comparison flags depend on both.
      //
      if (texture_dirty_ != 0 || sampler_dirty_ != 0)
      {
        pixel_state ps (pixel_state_);
        ps.depthSamplers       = 0;
        ps.linearDepthSamplers = 0;

        for (UINT i (0); i != samplers; ++i)
        {
          const uint32_t bit (1u << i);
          texture_object* to (textures_[i].object.get ());

          if ((texture_dirty_ & bit) != 0)
          {
            ID3D11ShaderResourceView* v (nullptr);

            if (to != nullptr)
            {
              image& im (to->storage ());

              // D3D11 unbinds a texture that is also a target (and the
              // debug layer complains), and D3D9 leaves the result
              // undefined, so we sample nothing instead.
              //
              bool target (false);
              for (const private_ref<surface>& s: targets_)
                target = target || (s && &s->storage () == &im);

              target = target || (depth_ && &depth_->storage () == &im);

              if (!target)
                v = im.shader_view (ss_[i][D3DSAMP_SRGBTEXTURE] != FALSE);
            }

            if (v != views_[i])
            {
              views_[i] = v;

              if (i < pixel_samplers)
                context_->PSSetShaderResources (i, 1, &v);
              else
                context_->VSSetShaderResources (i - pixel_samplers, 1, &v);
            }
          }

          if ((sampler_dirty_ & bit) != 0)
          {
            ID3D11SamplerState* s (sampler_states_.find (sampler_desc (ss_[i]),
                                                         [this] (const D3D11_SAMPLER_DESC& d, ID3D11SamplerState** r)
            {
              return device_->CreateSamplerState (&d, r);
            }));

            if (s != sampler_objects_[i])
            {
              sampler_objects_[i] = s;

              if (i < pixel_samplers)
                context_->PSSetSamplers (i, 1, &s);
              else
                context_->VSSetSamplers (i - pixel_samplers, 1, &s);
            }
          }

          if (i < pixel_samplers && to != nullptr && views_[i] != nullptr &&
              depth_format (*to->storage ().format ()))
          {
            ps.depthSamplers |= bit;

            if (linear_sampler (ss_[i]))
              ps.linearDepthSamplers |= bit;
          }
        }

        if (ps.depthSamplers != pixel_state_.depthSamplers ||
            ps.linearDepthSamplers != pixel_state_.linearDepthSamplers)
        {
          pixel_state_        = ps;
          pixel_state_valid_  = false;
        }

        texture_dirty_ = 0;
        sampler_dirty_ = 0;
      }

      if ((dirty_ & dirty_pixel) != 0)
      {
        const UINT f (rs_[D3DRS_ALPHATESTENABLE] != FALSE ? rs_[D3DRS_ALPHAFUNC] : D3DCMP_ALWAYS);
        const UINT r (rs_[D3DRS_ALPHAREF] & 0xFF);

        if (f != pixel_state_.alphaFunction || r != pixel_state_.alphaReference)
        {
          pixel_state_.alphaFunction  = f;
          pixel_state_.alphaReference = r;
          pixel_state_valid_          = false;
        }
      }

      apply_constants ();

      dirty_ = 0;
      return true;
    }

    HRESULT device::
    create_image (image_desc d, unique_ptr<image>& r)
    {
      if (d.width == 0 || d.height == 0 || d.depth == 0)
        return D3DERR_INVALIDCALL;

      const format_info* f (find_format (d.format));
      if (f == nullptr)
        return D3DERR_INVALIDCALL;

      const bool target (( d.usage & D3DUSAGE_RENDERTARGET) != 0);
      const bool depth  (( d.usage & D3DUSAGE_DEPTHSTENCIL) != 0);

      if ((target || depth) && d.pool != D3DPOOL_DEFAULT)
        return D3DERR_INVALIDCALL;

      if (depth && !depth_format (*f))
        return D3DERR_INVALIDCALL;

      if (d.format == null_format && !(target && d.type == image_type::surface) &&
          !(target && d.type == image_type::texture))
        return D3DERR_INVALIDCALL;

      // Zero levels means the whole chain, down to 1x1x1.
      //
      const UINT chain (bit_width (max ({d.width, d.height, d.depth})));

      if (d.levels == 0)
        d.levels = chain;
      else if (d.levels > chain)
        return D3DERR_INVALIDCALL;

      // Translate the multisample type into a sample count. The
      // nonmaskable type picks the count by quality level, so it becomes
      // the count it picks, with the standard pattern.
      //
      if (d.samples != D3DMULTISAMPLE_NONE)
      {
        const UINT n (parent_.sample_count (adapter_index_, d.format, d.samples, d.quality));

        if (n == 0)
          return D3DERR_INVALIDCALL;

        d.samples = static_cast<D3DMULTISAMPLE_TYPE> (n);
        d.quality = 0;

        if (n == 1)
          d.samples = D3DMULTISAMPLE_NONE;
      }

      unique_ptr<image> i (make_unique<image> (*this, d, f));

      if (HRESULT hr = i->create (); FAILED (hr))
        return hr;

      r = move (i);
      return D3D_OK;
    }

    HRESULT device::
    create_surface (const image_desc& d, IDirect3DSurface9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      unique_ptr<image> i;
      if (HRESULT hr = create_image (d, i); FAILED (hr))
        return hr;

      *r = new surface (*this, move (i));
      return D3D_OK;
    }

    UINT device::
    append_up (ComPtr<ID3D11Buffer>& b, UINT& position, UINT& capacity, UINT bind, const void* data, UINT n)
    {
      // Grow the buffer to fit, at least doubling it.
      //
      if (b == nullptr || n > capacity)
      {
        UINT c (max<UINT> (capacity * 2, 1 << 20));
        while (c < n)
          c *= 2;

        D3D11_BUFFER_DESC d {};
        d.ByteWidth      = c;
        d.Usage          = D3D11_USAGE_DYNAMIC;
        d.BindFlags      = bind;
        d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        b.Reset ();
        if (FAILED (device_->CreateBuffer (&d, nullptr, &b)))
        {
          capacity = 0;
          return numeric_limits<UINT>::max ();
        }

        capacity = c;
        position = capacity; // Make the map below discard.
      }

      // Append until the buffer is full, then start over by discarding it,
      // which is the usual way to stream into a dynamic buffer.
      //
      const bool discard (position + n > capacity);
      if (discard)
        position = 0;

      D3D11_MAPPED_SUBRESOURCE m;
      if (FAILED (context_->Map (b.Get (),
                                 0,
                                 discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
                                 0,
                                 &m)))
        return numeric_limits<UINT>::max ();

      memcpy (static_cast<std::byte*> (m.pData) + position, data, n);
      context_->Unmap (b.Get (), 0);

      const UINT r (position);
      position += n;
      return r;
    }

    // IDirect3DDevice9
    //
    HRESULT device::
    TestCooperativeLevel ()
    {
      return D3D_OK;
    }

    UINT device::
    GetAvailableTextureMem ()
    {
      // D3D9 reports what is left of the video memory, rounded to the
      // megabyte, which is what IW4 (and Streaming) budgets with. DXGI 1.4
      // has the budget, otherwise we report the dedicated memory.
      //
      UINT64 n (0);

      ComPtr<IDXGIAdapter3> a;
      if (SUCCEEDED (adapter_.As (&a)))
      {
        DXGI_QUERY_VIDEO_MEMORY_INFO i;
        if (SUCCEEDED (a->QueryVideoMemoryInfo (0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &i)))
          n = i.Budget > i.CurrentUsage ? i.Budget - i.CurrentUsage : 0;
      }

      if (n == 0)
      {
        DXGI_ADAPTER_DESC1 d;
        if (SUCCEEDED (adapter_->GetDesc1 (&d)))
          n = d.DedicatedVideoMemory;
      }

      // The count is 32-bit, so cap it just under 4GB as D3D9 does.
      //
      n = min<UINT64> (n, 0xFFE00000);
      return static_cast<UINT> (n & ~UINT64 (0xFFFFF));
    }

    HRESULT device::
    EvictManagedResources ()
    {
      return D3D_OK;
    }

    HRESULT device::
    GetDirect3D (IDirect3D9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      parent_.AddRef ();
      *r = &parent_;
      return D3D_OK;
    }

    HRESULT device::
    GetDeviceCaps (D3DCAPS9* c)
    {
      return parent_.GetDeviceCaps (adapter_index_, type_, c);
    }

    HRESULT device::
    GetDisplayMode (UINT i, D3DDISPLAYMODE* r)
    {
      if (i != 0)
        return D3DERR_INVALIDCALL;

      return parent_.GetAdapterDisplayMode (adapter_index_, r);
    }

    HRESULT device::
    GetCreationParameters (D3DDEVICE_CREATION_PARAMETERS* r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      r->AdapterOrdinal = adapter_index_;
      r->DeviceType     = type_;
      r->hFocusWindow   = focus_;
      r->BehaviorFlags  = behavior_;
      return D3D_OK;
    }

    // IW4 draws its own cursor, so a hardware cursor is something we only
    // pretend to have.
    //
    HRESULT device::
    SetCursorProperties (UINT, UINT, IDirect3DSurface9* s)
    {
      return s != nullptr ? D3D_OK : D3DERR_INVALIDCALL;
    }

    void device::
    SetCursorPosition (int, int, DWORD)
    {
    }

    BOOL device::
    ShowCursor (BOOL s)
    {
      lock_guard l (mutex_);

      const BOOL r (cursor_);
      cursor_ = s;
      return r;
    }

    HRESULT device::
    CreateAdditionalSwapChain (D3DPRESENT_PARAMETERS*, IDirect3DSwapChain9**)
    {
      unsupported ("additional swap chains");
      return D3DERR_NOTAVAILABLE;
    }

    HRESULT device::
    GetSwapChain (UINT i, IDirect3DSwapChain9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if (i != 0)
        return D3DERR_INVALIDCALL;

      swap_->AddRef ();
      *r = swap_.get ();
      return D3D_OK;
    }

    UINT device::
    GetNumberOfSwapChains ()
    {
      return 1;
    }

    HRESULT device::
    Reset (D3DPRESENT_PARAMETERS* pp)
    {
      if (pp == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      // Drop every binding first: D3D9 wants the game to release its
      // default pool resources before a reset and we want the old back
      // buffer and depth stencil gone before we resize.
      //
      for (bound_texture& t: textures_)
        t = bound_texture ();

      for (stream& s: streams_)
        s = stream ();

      targets_.fill (private_ref<surface> ());
      depth_      = private_ref<surface> ();
      auto_depth_ = private_ref<surface> ();
      context_->ClearState ();

      if (pp->Windowed == FALSE)
        unsupported ("exclusive fullscreen (rendering in a window instead)");

      if (HRESULT hr = swap_->reset (*pp); FAILED (hr))
      {
        reset_state ();
        return hr;
      }

      parameters_ = swap_->parameters ();
      *pp = parameters_;

      HRESULT hr (create_auto_depth ());
      reset_state ();
      return SUCCEEDED (hr) ? D3D_OK : D3DERR_DEVICELOST;
    }

    HRESULT device::
    Present (const RECT* s, const RECT* d, HWND w, const RGNDATA* r)
    {
      return swap_->Present (s, d, w, r, 0);
    }

    HRESULT device::
    GetBackBuffer (UINT sc, UINT i, D3DBACKBUFFER_TYPE t, IDirect3DSurface9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if (sc != 0)
        return D3DERR_INVALIDCALL;

      return swap_->GetBackBuffer (i, t, r);
    }

    HRESULT device::
    GetRasterStatus (UINT sc, D3DRASTER_STATUS* r)
    {
      if (sc != 0)
        return D3DERR_INVALIDCALL;

      return swap_->GetRasterStatus (r);
    }

    HRESULT device::
    SetDialogBoxMode (BOOL)
    {
      return D3D_OK;
    }

    // D3D9 ignores the gamma ramp in a window, which is where we always
    // render, so we only remember it.
    //
    void device::
    SetGammaRamp (UINT sc, DWORD, const D3DGAMMARAMP* g)
    {
      if (sc == 0 && g != nullptr)
      {
        lock_guard l (mutex_);
        gamma_ = *g;
      }
    }

    void device::
    GetGammaRamp (UINT sc, D3DGAMMARAMP* g)
    {
      if (sc == 0 && g != nullptr)
      {
        lock_guard l (mutex_);
        *g = gamma_;
      }
    }

    HRESULT device::
    CreateTexture (UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT f, D3DPOOL p,
                   IDirect3DTexture9** r, HANDLE* shared)
    {
      if (r == nullptr || shared != nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      image_desc d {};
      d.type   = image_type::texture;
      d.width  = w;
      d.height = h;
      d.depth  = 1;
      d.levels = levels;
      d.usage  = usage;
      d.pool   = p;
      d.format = f;

      unique_ptr<image> i;
      if (HRESULT hr = create_image (d, i); FAILED (hr))
        return hr;

      *r = new texture (*this, move (i));
      return D3D_OK;
    }

    HRESULT device::
    CreateVolumeTexture (UINT w, UINT h, UINT dp, UINT levels, DWORD usage, D3DFORMAT f, D3DPOOL p,
                         IDirect3DVolumeTexture9** r, HANDLE* shared)
    {
      if (r == nullptr || shared != nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if ((usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL)) != 0)
        return D3DERR_INVALIDCALL;

      image_desc d {};
      d.type   = image_type::volume;
      d.width  = w;
      d.height = h;
      d.depth  = dp;
      d.levels = levels;
      d.usage  = usage;
      d.pool   = p;
      d.format = f;

      unique_ptr<image> i;
      if (HRESULT hr = create_image (d, i); FAILED (hr))
        return hr;

      *r = new volume_texture (*this, move (i));
      return D3D_OK;
    }

    HRESULT device::
    CreateCubeTexture (UINT e, UINT levels, DWORD usage, D3DFORMAT f, D3DPOOL p,
                       IDirect3DCubeTexture9** r, HANDLE* shared)
    {
      if (r == nullptr || shared != nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if ((usage & D3DUSAGE_DEPTHSTENCIL) != 0)
        return D3DERR_INVALIDCALL;

      image_desc d {};
      d.type   = image_type::cube;
      d.width  = e;
      d.height = e;
      d.depth  = 1;
      d.levels = levels;
      d.usage  = usage;
      d.pool   = p;
      d.format = f;

      unique_ptr<image> i;
      if (HRESULT hr = create_image (d, i); FAILED (hr))
        return hr;

      *r = new cube_texture (*this, move (i));
      return D3D_OK;
    }

    HRESULT device::
    CreateVertexBuffer (UINT n, DWORD usage, DWORD fvf, D3DPOOL p,
                        IDirect3DVertexBuffer9** r, HANDLE* shared)
    {
      if (r == nullptr || shared != nullptr || n == 0)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      vertex_buffer* b (new vertex_buffer (*this, n, usage, fvf, p));

      if (HRESULT hr = b->storage.create (); FAILED (hr))
      {
        b->Release ();
        return hr;
      }

      *r = b;
      return D3D_OK;
    }

    HRESULT device::
    CreateIndexBuffer (UINT n, DWORD usage, D3DFORMAT f, D3DPOOL p,
                       IDirect3DIndexBuffer9** r, HANDLE* shared)
    {
      if (r == nullptr || shared != nullptr || n == 0)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      if (f != D3DFMT_INDEX16 && f != D3DFMT_INDEX32)
        return D3DERR_INVALIDCALL;

      index_buffer* b (new index_buffer (*this, n, usage, f, p));

      if (HRESULT hr = b->storage.create (); FAILED (hr))
      {
        b->Release ();
        return hr;
      }

      *r = b;
      return D3D_OK;
    }

    HRESULT device::
    CreateRenderTarget (UINT w, UINT h, D3DFORMAT f, D3DMULTISAMPLE_TYPE ms, DWORD q, BOOL lockable,
                        IDirect3DSurface9** r, HANDLE* shared)
    {
      if (shared != nullptr)
        return D3DERR_INVALIDCALL;

      image_desc d {};
      d.type     = image_type::surface;
      d.width    = w;
      d.height   = h;
      d.depth    = 1;
      d.levels   = 1;
      d.usage    = D3DUSAGE_RENDERTARGET;
      d.pool     = D3DPOOL_DEFAULT;
      d.format   = f;
      d.samples  = ms;
      d.quality  = q;
      d.lockable = lockable != FALSE;

      return create_surface (d, r);
    }

    HRESULT device::
    CreateDepthStencilSurface (UINT w, UINT h, D3DFORMAT f, D3DMULTISAMPLE_TYPE ms, DWORD q, BOOL,
                               IDirect3DSurface9** r, HANDLE* shared)
    {
      if (shared != nullptr)
        return D3DERR_INVALIDCALL;

      image_desc d {};
      d.type    = image_type::surface;
      d.width   = w;
      d.height  = h;
      d.depth   = 1;
      d.levels  = 1;
      d.usage   = D3DUSAGE_DEPTHSTENCIL;
      d.pool    = D3DPOOL_DEFAULT;
      d.format  = f;
      d.samples = ms;
      d.quality = q;

      return create_surface (d, r);
    }

    HRESULT device::
    UpdateSurface (IDirect3DSurface9* from, const RECT* fr, IDirect3DSurface9* to, const POINT* tp)
    {
      if (from == nullptr || to == nullptr)
        return D3DERR_INVALIDCALL;

      surface* f (surface_of (from));
      surface* t (surface_of (to));

      if (fr != nullptr || tp != nullptr)
      {
        const bool whole (fr == nullptr ||
                          (fr->left == 0 && fr->top == 0 &&
                           static_cast<UINT> (fr->right) == f->width () &&
                           static_cast<UINT> (fr->bottom) == f->height ()));

        if (!whole || (tp != nullptr && (tp->x != 0 || tp->y != 0)))
        {
          unsupported ("partial UpdateSurface()");
          return D3DERR_INVALIDCALL;
        }
      }

      return t->storage ().copy_from (t->subresource (), f->storage (), f->subresource ());
    }

    HRESULT device::
    UpdateTexture (IDirect3DBaseTexture9* from, IDirect3DBaseTexture9* to)
    {
      texture_object* f (texture_of (from));
      texture_object* t (texture_of (to));

      if (f == nullptr || t == nullptr || from->GetType () != to->GetType ())
        return D3DERR_INVALIDCALL;

      image& fi (f->storage ());
      image& ti (t->storage ());

      // The source may have more levels than the destination, in which
      // case D3D9 skips the source's top ones to match the sizes.
      //
      UINT skip (0);
      while (skip < fi.desc ().levels && fi.width (skip) > ti.width (0))
        ++skip;

      if (fi.desc ().levels - skip < ti.desc ().levels)
        return D3DERR_INVALIDCALL;

      for (UINT layer (0); layer != ti.layers (); ++layer)
      {
        for (UINT l (0); l != ti.desc ().levels; ++l)
        {
          if (HRESULT hr = ti.copy_from (ti.subresource (l, layer), fi, fi.subresource (l + skip, layer)); FAILED (hr))
            return hr;
        }
      }

      return D3D_OK;
    }

    HRESULT device::
    GetRenderTargetData (IDirect3DSurface9* from, IDirect3DSurface9* to)
    {
      if (from == nullptr || to == nullptr)
        return D3DERR_INVALIDCALL;

      surface* f (surface_of (from));
      surface* t (surface_of (to));

      image& fi (f->storage ());
      image& ti (t->storage ());

      // D3D9 refuses to read a multisampled target back as well.
      //
      if (fi.resource () == nullptr                          ||
          fi.desc ().samples != D3DMULTISAMPLE_NONE          ||
          fi.format () != ti.format ()                       ||
          fi.format ()->convert != conversion::none          ||
          f->width () != t->width ()                         ||
          f->height () != t->height ())
        return D3DERR_INVALIDCALL;

      // Copy into a staging texture and from there into the destination's
      // system memory copy, through its lock.
      //
      D3D11_TEXTURE2D_DESC d {};
      d.Width            = f->width ();
      d.Height           = f->height ();
      d.MipLevels        = 1;
      d.ArraySize        = 1;
      d.Format           = fi.format ()->resource;
      d.SampleDesc.Count = 1;
      d.Usage            = D3D11_USAGE_STAGING;
      d.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

      ComPtr<ID3D11Texture2D> staging;
      if (FAILED (device_->CreateTexture2D (&d, nullptr, &staging)))
        return D3DERR_OUTOFVIDEOMEMORY;

      lock_guard l (mutex_);

      context_->CopySubresourceRegion (staging.Get (), 0, 0, 0, 0, fi.resource (), f->subresource (), nullptr);

      D3D11_MAPPED_SUBRESOURCE m;
      if (FAILED (context_->Map (staging.Get (), 0, D3D11_MAP_READ, 0, &m)))
        return D3DERR_DEVICELOST;

      D3DLOCKED_BOX b;
      if (HRESULT hr = ti.lock (t->subresource (), b, nullptr, 0); FAILED (hr))
      {
        context_->Unmap (staging.Get (), 0);
        return hr;
      }

      const UINT row (row_pitch (*fi.format (), f->width ()));
      const UINT rows (mip_blocks (*fi.format (), f->height (), 0));

      for (UINT y (0); y != rows; ++y)
        memcpy (static_cast<std::byte*> (b.pBits) + static_cast<size_t> (y) * b.RowPitch,
                static_cast<const std::byte*> (m.pData) + static_cast<size_t> (y) * m.RowPitch,
                row);

      ti.unlock (t->subresource ());
      context_->Unmap (staging.Get (), 0);
      return D3D_OK;
    }

    HRESULT device::
    GetFrontBufferData (UINT, IDirect3DSurface9*)
    {
      unsupported ("GetFrontBufferData()");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    StretchRect (IDirect3DSurface9* from, const RECT* fr, IDirect3DSurface9* to, const RECT* tr,
                 D3DTEXTUREFILTERTYPE)
    {
      if (from == nullptr || to == nullptr)
        return D3DERR_INVALIDCALL;

      surface* f (surface_of (from));
      surface* t (surface_of (to));

      image& fi (f->storage ());
      image& ti (t->storage ());

      if (fi.null () || ti.null ())
        return D3D_OK;

      if (fi.resource () == nullptr || ti.resource () == nullptr)
        return D3DERR_INVALIDCALL;

      const RECT fw {0, 0, static_cast<LONG> (f->width ()), static_cast<LONG> (f->height ())};
      const RECT tw {0, 0, static_cast<LONG> (t->width ()), static_cast<LONG> (t->height ())};

      const RECT& a (fr != nullptr ? *fr : fw);
      const RECT& b (tr != nullptr ? *tr : tw);

      const LONG w (a.right - a.left);
      const LONG h (a.bottom - a.top);

      if (w <= 0 || h <= 0 || b.right - b.left != w || b.bottom - b.top != h)
      {
        // @@ Scaling needs a draw, which IW4 doesn't seem to need.
        //
        unsupported ("scaling StretchRect()");
        return D3DERR_INVALIDCALL;
      }

      const bool fms (fi.desc ().samples != D3DMULTISAMPLE_NONE);
      const bool tms (ti.desc ().samples != D3DMULTISAMPLE_NONE);

      lock_guard l (mutex_);

      // Resolve a multisampled source into a single sampled destination,
      // which D3D11 only does whole.
      //
      if (fms && !tms)
      {
        if (w != fw.right || h != fw.bottom || a.left != 0 || a.top != 0 ||
            b.left != 0 || b.top != 0 || w != tw.right || h != tw.bottom ||
            depth_format (*fi.format ()))
        {
          unsupported ("partial (or depth) multisample resolve");
          return D3DERR_INVALIDCALL;
        }

        context_->ResolveSubresource (ti.resource (), t->subresource (),
                                      fi.resource (), f->subresource (),
                                      ti.format ()->view);
        return D3D_OK;
      }

      if (fms != tms || fi.desc ().samples != ti.desc ().samples)
        return D3DERR_INVALIDCALL;

      // D3D11 only copies multisampled and depth resources whole.
      //
      const bool whole (a.left == 0 && a.top == 0 && b.left == 0 && b.top == 0 &&
                        w == fw.right && h == fw.bottom && w == tw.right && h == tw.bottom);

      if ((fms || depth_format (*fi.format ())) && !whole)
      {
        unsupported ("partial multisample (or depth) StretchRect()");
        return D3DERR_INVALIDCALL;
      }

      const D3D11_BOX box {static_cast<UINT> (a.left),
                           static_cast<UINT> (a.top),
                           0,
                           static_cast<UINT> (a.right),
                           static_cast<UINT> (a.bottom),
                           1};

      context_->CopySubresourceRegion (ti.resource (), t->subresource (),
                                       static_cast<UINT> (b.left), static_cast<UINT> (b.top), 0,
                                       fi.resource (), f->subresource (),
                                       whole ? nullptr : &box);
      return D3D_OK;
    }

    HRESULT device::
    ColorFill (IDirect3DSurface9* s, const RECT* r, D3DCOLOR c)
    {
      if (s == nullptr)
        return D3DERR_INVALIDCALL;

      surface* x (surface_of (s));
      return x->storage ().fill (x->subresource (), r, c);
    }

    HRESULT device::
    CreateOffscreenPlainSurface (UINT w, UINT h, D3DFORMAT f, D3DPOOL p,
                                 IDirect3DSurface9** r, HANDLE* shared)
    {
      if (shared != nullptr || p == D3DPOOL_MANAGED)
        return D3DERR_INVALIDCALL;

      image_desc d {};
      d.type   = image_type::surface;
      d.width  = w;
      d.height = h;
      d.depth  = 1;
      d.levels = 1;
      d.pool   = p;
      d.format = f;

      return create_surface (d, r);
    }

    HRESULT device::
    SetRenderTarget (DWORD i, IDirect3DSurface9* s)
    {
      if (i >= targets || (i == 0 && s == nullptr))
        return D3DERR_INVALIDCALL;

      surface* x (surface_of (s));

      if (x != nullptr && (x->storage ().desc ().usage & D3DUSAGE_RENDERTARGET) == 0)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      targets_[i] = private_ref<surface> (x);
      dirty_ |= dirty_targets;

      // Setting the first target resets the viewport and the scissor to
      // its size.
      //
      if (i == 0)
      {
        viewport_ = D3DVIEWPORT9 {0, 0, x->width (), x->height (), 0.0f, 1.0f};
        scissor_  = RECT {0, 0, static_cast<LONG> (x->width ()), static_cast<LONG> (x->height ())};
        dirty_ |= dirty_viewport | dirty_scissor;
      }

      return D3D_OK;
    }

    HRESULT device::
    GetRenderTarget (DWORD i, IDirect3DSurface9** r)
    {
      if (r == nullptr || i >= targets)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = targets_[i].get ();
      if (*r == nullptr)
        return D3DERR_NOTFOUND;

      (*r)->AddRef ();
      return D3D_OK;
    }

    HRESULT device::
    SetDepthStencilSurface (IDirect3DSurface9* s)
    {
      surface* x (surface_of (s));

      if (x != nullptr && (x->storage ().desc ().usage & D3DUSAGE_DEPTHSTENCIL) == 0)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      // The depth format decides the depth bias scale.
      //
      depth_ = private_ref<surface> (x);
      dirty_ |= dirty_targets | dirty_raster;
      return D3D_OK;
    }

    HRESULT device::
    GetDepthStencilSurface (IDirect3DSurface9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = depth_.get ();
      if (*r == nullptr)
        return D3DERR_NOTFOUND;

      (*r)->AddRef ();
      return D3D_OK;
    }

    HRESULT device::
    BeginScene ()
    {
      lock_guard l (mutex_);

      if (scene_)
        return D3DERR_INVALIDCALL;

      scene_ = true;
      return D3D_OK;
    }

    HRESULT device::
    EndScene ()
    {
      lock_guard l (mutex_);

      if (!scene_)
        return D3DERR_INVALIDCALL;

      scene_ = false;
      return D3D_OK;
    }

    HRESULT device::
    Clear (DWORD n, const D3DRECT* rs, DWORD flags, D3DCOLOR c, float z, DWORD stencil)
    {
      if (n != 0 && rs == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      if ((dirty_ & dirty_targets) != 0)
      {
        apply_targets ();
        dirty_ &= ~dirty_targets;
      }

      // D3D9 clears the viewport, limited by the scissor if it's enabled,
      // and by the rectangles if any.
      //
      const auto [tw, th] (target_size ());

      RECT area {static_cast<LONG> (viewport_.X),
                 static_cast<LONG> (viewport_.Y),
                 static_cast<LONG> (viewport_.X + viewport_.Width),
                 static_cast<LONG> (viewport_.Y + viewport_.Height)};

      if (rs_[D3DRS_SCISSORTESTENABLE] != FALSE)
        IntersectRect (&area, &area, &scissor_);

      vector<D3D11_RECT> rects;

      if (n != 0)
      {
        rects.reserve (n);
        for (DWORD i (0); i != n; ++i)
        {
          const RECT x {rs[i].x1, rs[i].y1, rs[i].x2, rs[i].y2};
          RECT y;
          if (IntersectRect (&y, &x, &area))
            rects.push_back (D3D11_RECT {y.left, y.top, y.right, y.bottom});
        }

        if (rects.empty ())
          return D3D_OK;
      }
      else if (area.left > 0 || area.top > 0 ||
               area.right < static_cast<LONG> (tw) || area.bottom < static_cast<LONG> (th))
      {
        if (IsRectEmpty (&area))
          return D3D_OK;

        rects.push_back (D3D11_RECT {area.left, area.top, area.right, area.bottom});
      }

      if ((flags & D3DCLEAR_TARGET) != 0)
      {
        float color[4];
        unpack_color (c, color);

        for (const private_ref<surface>& t: targets_)
        {
          if (!t || t->storage ().null ())
            continue;

          ID3D11RenderTargetView* v (t->target_view (false));
          if (v == nullptr)
            continue;

          if (rects.empty ())
            context_->ClearRenderTargetView (v, color);
          else
            context_->ClearView (v, color, rects.data (), static_cast<UINT> (rects.size ()));
        }
      }

      if ((flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)) != 0 && depth_)
      {
        // D3D11 can't clear a part of a depth stencil without a draw. IW4
        // clears its depth whole, so let's clear it whole and say so if
        // that ever changes.
        //
        if (!rects.empty ())
          unsupported ("partial depth stencil clear (clearing it whole)");

        UINT f (0);
        if ((flags & D3DCLEAR_ZBUFFER) != 0) f |= D3D11_CLEAR_DEPTH;
        if ((flags & D3DCLEAR_STENCIL) != 0) f |= D3D11_CLEAR_STENCIL;

        if (ID3D11DepthStencilView* v = depth_->depth_view ())
          context_->ClearDepthStencilView (v, f, z, static_cast<UINT8> (stencil));
      }

      return D3D_OK;
    }

    // The fixed function state. We don't render with it (see prepare())
    // but we keep it so that the game reads back what it set.
    //
    HRESULT device::
    SetTransform (D3DTRANSFORMSTATETYPE t, const D3DMATRIX* m)
    {
      if (m == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      transforms_[t] = *m;
      return D3D_OK;
    }

    HRESULT device::
    GetTransform (D3DTRANSFORMSTATETYPE t, D3DMATRIX* m)
    {
      if (m == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      auto i (transforms_.find (t));
      *m = i != transforms_.end () ? i->second : identity;
      return D3D_OK;
    }

    HRESULT device::
    MultiplyTransform (D3DTRANSFORMSTATETYPE t, const D3DMATRIX* m)
    {
      if (m == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      D3DMATRIX a;
      GetTransform (t, &a);

      D3DMATRIX r;
      for (int i (0); i != 4; ++i)
        for (int j (0); j != 4; ++j)
        {
          float v (0.0f);
          for (int k (0); k != 4; ++k)
            v += a.m[i][k] * m->m[k][j];
          r.m[i][j] = v;
        }

      transforms_[t] = r;
      return D3D_OK;
    }

    HRESULT device::
    SetViewport (const D3DVIEWPORT9* v)
    {
      if (v == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      viewport_ = *v;
      dirty_ |= dirty_viewport;
      return D3D_OK;
    }

    HRESULT device::
    GetViewport (D3DVIEWPORT9* v)
    {
      if (v == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *v = viewport_;
      return D3D_OK;
    }

    HRESULT device::
    SetMaterial (const D3DMATERIAL9* m)
    {
      if (m == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      material_ = *m;
      return D3D_OK;
    }

    HRESULT device::
    GetMaterial (D3DMATERIAL9* m)
    {
      if (m == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *m = material_;
      return D3D_OK;
    }

    HRESULT device::
    SetLight (DWORD i, const D3DLIGHT9* x)
    {
      if (x == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      lights_[i].first = *x;
      return D3D_OK;
    }

    HRESULT device::
    GetLight (DWORD i, D3DLIGHT9* x)
    {
      if (x == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      auto j (lights_.find (i));
      if (j == lights_.end ())
        return D3DERR_INVALIDCALL;

      *x = j->second.first;
      return D3D_OK;
    }

    HRESULT device::
    LightEnable (DWORD i, BOOL e)
    {
      lock_guard l (mutex_);

      // Enabling a light that was never set sets it to the default light,
      // a white directional one.
      //
      auto j (lights_.find (i));
      if (j == lights_.end ())
      {
        D3DLIGHT9 x {};
        x.Type        = D3DLIGHT_DIRECTIONAL;
        x.Diffuse     = D3DCOLORVALUE {1.0f, 1.0f, 1.0f, 0.0f};
        x.Direction.z = 1.0f;
        j = lights_.emplace (i, make_pair (x, FALSE)).first;
      }

      j->second.second = e;
      return D3D_OK;
    }

    HRESULT device::
    GetLightEnable (DWORD i, BOOL* e)
    {
      if (e == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      auto j (lights_.find (i));
      if (j == lights_.end ())
        return D3DERR_INVALIDCALL;

      *e = j->second.second != FALSE ? 128 : 0; // As D3D9 does.
      return D3D_OK;
    }

    HRESULT device::
    SetClipPlane (DWORD i, const float* p)
    {
      if (p == nullptr || i >= clip_planes_.size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      copy (p, p + 4, clip_planes_[i].begin ());
      return D3D_OK;
    }

    HRESULT device::
    GetClipPlane (DWORD i, float* p)
    {
      if (p == nullptr || i >= clip_planes_.size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      ranges::copy (clip_planes_[i], p);
      return D3D_OK;
    }

    HRESULT device::
    SetRenderState (D3DRENDERSTATETYPE s, DWORD v)
    {
      if (static_cast<UINT> (s) >= rs_.size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      if (rs_[s] == v)
        return D3D_OK;

      rs_[s] = v;

      switch (s)
      {
        case D3DRS_ALPHABLENDENABLE:
        case D3DRS_SRCBLEND:
        case D3DRS_DESTBLEND:
        case D3DRS_BLENDOP:
        case D3DRS_SEPARATEALPHABLENDENABLE:
        case D3DRS_SRCBLENDALPHA:
        case D3DRS_DESTBLENDALPHA:
        case D3DRS_BLENDOPALPHA:
        case D3DRS_COLORWRITEENABLE:
        case D3DRS_COLORWRITEENABLE1:
        case D3DRS_COLORWRITEENABLE2:
        case D3DRS_COLORWRITEENABLE3:
        case D3DRS_BLENDFACTOR:
        case D3DRS_MULTISAMPLEMASK:
          dirty_ |= dirty_blend;
          break;

        case D3DRS_ZENABLE:
        case D3DRS_ZWRITEENABLE:
        case D3DRS_ZFUNC:
        case D3DRS_STENCILENABLE:
        case D3DRS_STENCILFAIL:
        case D3DRS_STENCILZFAIL:
        case D3DRS_STENCILPASS:
        case D3DRS_STENCILFUNC:
        case D3DRS_STENCILREF:
        case D3DRS_STENCILMASK:
        case D3DRS_STENCILWRITEMASK:
        case D3DRS_TWOSIDEDSTENCILMODE:
        case D3DRS_CCW_STENCILFAIL:
        case D3DRS_CCW_STENCILZFAIL:
        case D3DRS_CCW_STENCILPASS:
        case D3DRS_CCW_STENCILFUNC:
          dirty_ |= dirty_depth;
          break;

        case D3DRS_FILLMODE:
        case D3DRS_CULLMODE:
        case D3DRS_DEPTHBIAS:
        case D3DRS_SLOPESCALEDEPTHBIAS:
        case D3DRS_SCISSORTESTENABLE:
        case D3DRS_MULTISAMPLEANTIALIAS:
        case D3DRS_ANTIALIASEDLINEENABLE:
          dirty_ |= dirty_raster;
          break;

        case D3DRS_ALPHATESTENABLE:
        case D3DRS_ALPHAREF:
        case D3DRS_ALPHAFUNC:
          dirty_ |= dirty_pixel;
          break;

        case D3DRS_SRGBWRITEENABLE:
          dirty_ |= dirty_targets;
          break;

        case D3DRS_CLIPPLANEENABLE:
          if (v != 0)
            unsupported ("user clip planes");
          break;

        default:
          break;
      }

      return D3D_OK;
    }

    HRESULT device::
    GetRenderState (D3DRENDERSTATETYPE s, DWORD* v)
    {
      if (v == nullptr || static_cast<UINT> (s) >= rs_.size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *v = rs_[s];
      return D3D_OK;
    }

    // IW4 doesn't use state blocks, which would mean recording every
    // setter, so we leave them out.
    //
    HRESULT device::
    CreateStateBlock (D3DSTATEBLOCKTYPE, IDirect3DStateBlock9** r)
    {
      if (r != nullptr)
        *r = nullptr;

      unsupported ("state blocks");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    BeginStateBlock ()
    {
      unsupported ("state blocks");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    EndStateBlock (IDirect3DStateBlock9** r)
    {
      if (r != nullptr)
        *r = nullptr;

      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetClipStatus (const D3DCLIPSTATUS9* s)
    {
      if (s == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      clip_status_ = *s;
      return D3D_OK;
    }

    HRESULT device::
    GetClipStatus (D3DCLIPSTATUS9* s)
    {
      if (s == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *s = clip_status_;
      return D3D_OK;
    }

    HRESULT device::
    GetTexture (DWORD s, IDirect3DBaseTexture9** r)
    {
      const UINT i (sampler_index (s));

      if (r == nullptr || i >= samplers)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = textures_[i].com;
      if (*r != nullptr)
        (*r)->AddRef ();

      return D3D_OK;
    }

    HRESULT device::
    SetTexture (DWORD s, IDirect3DBaseTexture9* t)
    {
      const UINT i (sampler_index (s));

      if (i >= samplers)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      bound_texture& b (textures_[i]);
      if (b.com == t)
        return D3D_OK;

      b.object = private_ref<texture_object> (texture_of (t));
      b.com    = t;

      texture_dirty_ |= 1u << i;
      return D3D_OK;
    }

    // The texture stages are fixed function state as well.
    //
    HRESULT device::
    GetTextureStageState (DWORD s, D3DTEXTURESTAGESTATETYPE t, DWORD* v)
    {
      if (v == nullptr || s >= stages_.size () || static_cast<UINT> (t) >= stages_[0].size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *v = stages_[s][t];
      return D3D_OK;
    }

    HRESULT device::
    SetTextureStageState (DWORD s, D3DTEXTURESTAGESTATETYPE t, DWORD v)
    {
      if (s >= stages_.size () || static_cast<UINT> (t) >= stages_[0].size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      stages_[s][t] = v;
      return D3D_OK;
    }

    HRESULT device::
    GetSamplerState (DWORD s, D3DSAMPLERSTATETYPE t, DWORD* v)
    {
      const UINT i (sampler_index (s));

      if (v == nullptr || i >= samplers || static_cast<UINT> (t) >= ss_[0].size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *v = ss_[i][t];
      return D3D_OK;
    }

    HRESULT device::
    SetSamplerState (DWORD s, D3DSAMPLERSTATETYPE t, DWORD v)
    {
      const UINT i (sampler_index (s));

      if (i >= samplers || static_cast<UINT> (t) >= ss_[0].size ())
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      if (ss_[i][t] == v)
        return D3D_OK;

      ss_[i][t] = v;

      // The sRGB state picks the view rather than the sampler. The
      // filters also decide the depth comparison flags (see prepare()),
      // which we recompute with the samplers.
      //
      if (t == D3DSAMP_SRGBTEXTURE)
        texture_dirty_ |= 1u << i;
      else
        sampler_dirty_ |= 1u << i;

      return D3D_OK;
    }

    HRESULT device::
    ValidateDevice (DWORD* n)
    {
      if (n == nullptr)
        return D3DERR_INVALIDCALL;

      *n = 1;
      return D3D_OK;
    }

    // Palettized textures are something no D3D9 driver of this era
    // supports, and neither do we.
    //
    HRESULT device::
    SetPaletteEntries (UINT, const PALETTEENTRY*)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    GetPaletteEntries (UINT, PALETTEENTRY*)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetCurrentTexturePalette (UINT)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    GetCurrentTexturePalette (UINT*)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetScissorRect (const RECT* r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      scissor_ = *r;
      dirty_ |= dirty_scissor;
      return D3D_OK;
    }

    HRESULT device::
    GetScissorRect (RECT* r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *r = scissor_;
      return D3D_OK;
    }

    HRESULT device::
    SetSoftwareVertexProcessing (BOOL)
    {
      // Only mixed vertex processing devices may switch and ours is a
      // hardware one.
      //
      return D3DERR_INVALIDCALL;
    }

    BOOL device::
    GetSoftwareVertexProcessing ()
    {
      return FALSE;
    }

    HRESULT device::
    SetNPatchMode (float n)
    {
      if (n != 0.0f)
        unsupported ("N-patches");

      return D3D_OK;
    }

    float device::
    GetNPatchMode ()
    {
      return 0.0f;
    }

    HRESULT device::
    DrawPrimitive (D3DPRIMITIVETYPE t, UINT start, UINT n)
    {
      lock_guard l (mutex_);

      if (n == 0 || !prepare (t))
        return D3D_OK;

      context_->Draw (vertex_count (t, n), start);
      return D3D_OK;
    }

    HRESULT device::
    DrawIndexedPrimitive (D3DPRIMITIVETYPE t, INT base, UINT, UINT, UINT start, UINT n)
    {
      lock_guard l (mutex_);

      if (!indices_)
        return D3DERR_INVALIDCALL;

      if (n == 0 || !prepare (t))
        return D3D_OK;

      context_->DrawIndexed (vertex_count (t, n), start, base);
      return D3D_OK;
    }

    HRESULT device::
    DrawPrimitiveUP (D3DPRIMITIVETYPE t, UINT n, const void* data, UINT stride)
    {
      if (data == nullptr || stride == 0)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      const UINT count (vertex_count (t, n));

      // D3D9 unbinds stream 0 after a draw from user memory, whatever
      // happens.
      //
      streams_[0] = stream ();
      dirty_ |= dirty_streams;

      if (count == 0)
        return D3D_OK;

      const UINT o (append_up (up_vertices_, up_vertex_position_, up_vertex_capacity_,
                               D3D11_BIND_VERTEX_BUFFER, data, count * stride));
      if (o == numeric_limits<UINT>::max () || !prepare (t))
        return D3D_OK;

      ID3D11Buffer* const b (up_vertices_.Get ());
      context_->IASetVertexBuffers (0, 1, &b, &stride, &o);
      dirty_ |= dirty_streams;

      context_->Draw (count, 0);
      return D3D_OK;
    }

    HRESULT device::
    DrawIndexedPrimitiveUP (D3DPRIMITIVETYPE t, UINT min, UINT vertices, UINT n,
                            const void* indices, D3DFORMAT f, const void* data, UINT stride)
    {
      if (data == nullptr || indices == nullptr || stride == 0 ||
          (f != D3DFMT_INDEX16 && f != D3DFMT_INDEX32))
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      const UINT count (vertex_count (t, n));

      // As above, plus the indices.
      //
      streams_[0] = stream ();
      indices_    = private_ref<index_buffer> ();
      dirty_ |= dirty_streams | dirty_indices;

      if (count == 0)
        return D3D_OK;

      const UINT is (f == D3DFMT_INDEX16 ? 2 : 4);

      // The indices are relative to the start of the data, so we copy the
      // vertices up to the last one they use.
      //
      const UINT vo (append_up (up_vertices_, up_vertex_position_, up_vertex_capacity_,
                                D3D11_BIND_VERTEX_BUFFER, data, (min + vertices) * stride));
      const UINT io (append_up (up_indices_, up_index_position_, up_index_capacity_,
                                D3D11_BIND_INDEX_BUFFER, indices, count * is));

      if (vo == numeric_limits<UINT>::max () || io == numeric_limits<UINT>::max () || !prepare (t))
        return D3D_OK;

      ID3D11Buffer* const b (up_vertices_.Get ());
      context_->IASetVertexBuffers (0, 1, &b, &stride, &vo);
      context_->IASetIndexBuffer (up_indices_.Get (),
                                  is == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
                                  io);
      dirty_ |= dirty_streams | dirty_indices;

      context_->DrawIndexed (count, 0, 0);
      return D3D_OK;
    }

    HRESULT device::
    ProcessVertices (UINT, UINT, UINT, IDirect3DVertexBuffer9*, IDirect3DVertexDeclaration9*, DWORD)
    {
      unsupported ("ProcessVertices()");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    CreateVertexDeclaration (const D3DVERTEXELEMENT9* e, IDirect3DVertexDeclaration9** r)
    {
      if (e == nullptr || r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = new vertex_declaration (*this, e);
      return D3D_OK;
    }

    HRESULT device::
    SetVertexDeclaration (IDirect3DVertexDeclaration9* d)
    {
      lock_guard l (mutex_);

      vertex_declaration* x (static_cast<vertex_declaration*> (d));
      if (x == decl_.get ())
        return D3D_OK;

      decl_ = private_ref<vertex_declaration> (x);
      fvf_  = 0;
      dirty_ |= dirty_layout;
      return D3D_OK;
    }

    HRESULT device::
    GetVertexDeclaration (IDirect3DVertexDeclaration9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = decl_.get ();
      if (*r != nullptr)
        (*r)->AddRef ();

      return D3D_OK;
    }

    HRESULT device::
    SetFVF (DWORD v)
    {
      lock_guard l (mutex_);

      // An FVF replaces the declaration, which we can't draw with.
      //
      if (v != 0)
      {
        unsupported ("FVF");
        decl_ = private_ref<vertex_declaration> ();
        dirty_ |= dirty_layout;
      }

      fvf_ = v;
      return D3D_OK;
    }

    HRESULT device::
    GetFVF (DWORD* v)
    {
      if (v == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *v = fvf_;
      return D3D_OK;
    }

    namespace
    {
      // Return the translation of the D3D9 shader, logging the missing
      // ones once each.
      //
      span<const std::byte>
      find_shader (uint64_t key, const char* what)
      {
        span<const std::byte> r (shaders ().find (key));

        if (r.empty ())
        {
          static mutex m;
          static set<uint64_t> missing;

          lock_guard l (m);
          if (missing.insert (key).second)
            Logger::Warning (Game::CON_CHANNEL_GFX,
                             "Direct3D 11: no translation of {} shader {:016x}, skipping its draws\n",
                             what,
                             key);
        }

        return r;
      }
    }

    HRESULT device::
    CreateVertexShader (const DWORD* f, IDirect3DVertexShader9** r)
    {
      if (f == nullptr || r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      const auto [key, n] (hash_bytecode (f));
      if (n == 0)
        return D3DERR_INVALIDCALL;

      vertex_shader* s (new vertex_shader (*this, f, n, key));

      s->dxbc = find_shader (key, "vertex");

      if (!s->dxbc.empty () &&
          FAILED (device_->CreateVertexShader (s->dxbc.data (), s->dxbc.size (), nullptr, &s->shader)))
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Direct3D 11: couldn't create vertex shader {:016x}\n",
                         key);

      *r = s;
      return D3D_OK;
    }

    HRESULT device::
    SetVertexShader (IDirect3DVertexShader9* s)
    {
      lock_guard l (mutex_);

      vertex_shader* x (static_cast<vertex_shader*> (s));
      if (x == vs_.get ())
        return D3D_OK;

      vs_ = private_ref<vertex_shader> (x);
      dirty_ |= dirty_shaders | dirty_layout;
      return D3D_OK;
    }

    HRESULT device::
    GetVertexShader (IDirect3DVertexShader9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = vs_.get ();
      if (*r != nullptr)
        (*r)->AddRef ();

      return D3D_OK;
    }

    namespace
    {
      // Copy constant registers in and out of a register file, checking
      // the range.
      //
      template <typename T>
        requires std::is_trivially_copyable_v<T>
      bool
      set_registers (T* file, UINT size, UINT start, const T* data, UINT count, UINT width)
      {
        if (data == nullptr || start > size || count > size - start)
          return false;

        memcpy (file + start * width, data, static_cast<size_t> (count) * width * sizeof (T));
        return true;
      }

      template <typename T>
        requires std::is_trivially_copyable_v<T>
      bool
      get_registers (const T* file, UINT size, UINT start, T* data, UINT count, UINT width)
      {
        if (data == nullptr || start > size || count > size - start)
          return false;

        memcpy (data, file + start * width, static_cast<size_t> (count) * width * sizeof (T));
        return true;
      }
    }

    HRESULT device::
    SetVertexShaderConstantF (UINT s, const float* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (vertex_.floats.data (), vertex_floats, s, d, n, 4))
        return D3DERR_INVALIDCALL;

      vertex_.dirty_floats = true;
      return D3D_OK;
    }

    HRESULT device::
    GetVertexShaderConstantF (UINT s, float* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (vertex_.floats.data (), vertex_floats, s, d, n, 4) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetVertexShaderConstantI (UINT s, const int* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (vertex_.ints.data (), integers, s, d, n, 4))
        return D3DERR_INVALIDCALL;

      vertex_.dirty_ints = true;
      return D3D_OK;
    }

    HRESULT device::
    GetVertexShaderConstantI (UINT s, int* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (vertex_.ints.data (), integers, s, d, n, 4) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetVertexShaderConstantB (UINT s, const BOOL* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (vertex_.bools.data (), booleans, s, d, n, 1))
        return D3DERR_INVALIDCALL;

      vertex_.dirty_bools = true;
      return D3D_OK;
    }

    HRESULT device::
    GetVertexShaderConstantB (UINT s, BOOL* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (vertex_.bools.data (), booleans, s, d, n, 1) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetStreamSource (UINT i, IDirect3DVertexBuffer9* b, UINT offset, UINT stride)
    {
      if (i >= streams)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      stream& s (streams_[i]);
      if (s.com == b && s.offset == offset && s.stride == stride)
        return D3D_OK;

      s.buffer = private_ref<vertex_buffer> (static_cast<vertex_buffer*> (b));
      s.com    = b;
      s.offset = offset;
      s.stride = stride;

      dirty_ |= dirty_streams;
      return D3D_OK;
    }

    HRESULT device::
    GetStreamSource (UINT i, IDirect3DVertexBuffer9** b, UINT* offset, UINT* stride)
    {
      if (i >= streams || b == nullptr || offset == nullptr || stride == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      const stream& s (streams_[i]);

      *b = s.com;
      if (*b != nullptr)
        (*b)->AddRef ();

      *offset = s.offset;
      *stride = s.stride;
      return D3D_OK;
    }

    HRESULT device::
    SetStreamSourceFreq (UINT i, UINT f)
    {
      if (i >= streams)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      if (f != 1)
        unsupported ("instancing");

      streams_[i].freq = f;
      return D3D_OK;
    }

    HRESULT device::
    GetStreamSourceFreq (UINT i, UINT* f)
    {
      if (i >= streams || f == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);
      *f = streams_[i].freq;
      return D3D_OK;
    }

    HRESULT device::
    SetIndices (IDirect3DIndexBuffer9* b)
    {
      lock_guard l (mutex_);

      index_buffer* x (static_cast<index_buffer*> (b));
      if (x == indices_.get ())
        return D3D_OK;

      indices_ = private_ref<index_buffer> (x);
      dirty_ |= dirty_indices;
      return D3D_OK;
    }

    HRESULT device::
    GetIndices (IDirect3DIndexBuffer9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = indices_.get ();
      if (*r != nullptr)
        (*r)->AddRef ();

      return D3D_OK;
    }

    HRESULT device::
    CreatePixelShader (const DWORD* f, IDirect3DPixelShader9** r)
    {
      if (f == nullptr || r == nullptr)
        return D3DERR_INVALIDCALL;

      *r = nullptr;

      const auto [key, n] (hash_bytecode (f));
      if (n == 0)
        return D3DERR_INVALIDCALL;

      pixel_shader* s (new pixel_shader (*this, f, n, key));

      const span<const std::byte> dxbc (find_shader (key, "pixel"));

      if (!dxbc.empty () &&
          FAILED (device_->CreatePixelShader (dxbc.data (), dxbc.size (), nullptr, &s->shader)))
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Direct3D 11: couldn't create pixel shader {:016x}\n",
                         key);

      *r = s;
      return D3D_OK;
    }

    HRESULT device::
    SetPixelShader (IDirect3DPixelShader9* s)
    {
      lock_guard l (mutex_);

      pixel_shader* x (static_cast<pixel_shader*> (s));
      if (x == ps_.get ())
        return D3D_OK;

      ps_ = private_ref<pixel_shader> (x);
      dirty_ |= dirty_shaders;
      return D3D_OK;
    }

    HRESULT device::
    GetPixelShader (IDirect3DPixelShader9** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard l (mutex_);

      *r = ps_.get ();
      if (*r != nullptr)
        (*r)->AddRef ();

      return D3D_OK;
    }

    HRESULT device::
    SetPixelShaderConstantF (UINT s, const float* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (pixel_.floats.data (), pixel_floats, s, d, n, 4))
        return D3DERR_INVALIDCALL;

      pixel_.dirty_floats = true;
      return D3D_OK;
    }

    HRESULT device::
    GetPixelShaderConstantF (UINT s, float* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (pixel_.floats.data (), pixel_floats, s, d, n, 4) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetPixelShaderConstantI (UINT s, const int* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (pixel_.ints.data (), integers, s, d, n, 4))
        return D3DERR_INVALIDCALL;

      pixel_.dirty_ints = true;
      return D3D_OK;
    }

    HRESULT device::
    GetPixelShaderConstantI (UINT s, int* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (pixel_.ints.data (), integers, s, d, n, 4) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    HRESULT device::
    SetPixelShaderConstantB (UINT s, const BOOL* d, UINT n)
    {
      lock_guard l (mutex_);

      if (!set_registers (pixel_.bools.data (), booleans, s, d, n, 1))
        return D3DERR_INVALIDCALL;

      pixel_.dirty_bools = true;
      return D3D_OK;
    }

    HRESULT device::
    GetPixelShaderConstantB (UINT s, BOOL* d, UINT n)
    {
      lock_guard l (mutex_);
      return get_registers (pixel_.bools.data (), booleans, s, d, n, 1) ? D3D_OK : D3DERR_INVALIDCALL;
    }

    // IW4 has no use for patches either.
    //
    HRESULT device::
    DrawRectPatch (UINT, const float*, const D3DRECTPATCH_INFO*)
    {
      unsupported ("patches");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    DrawTriPatch (UINT, const float*, const D3DTRIPATCH_INFO*)
    {
      unsupported ("patches");
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    DeletePatch (UINT)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT device::
    CreateQuery (D3DQUERYTYPE t, IDirect3DQuery9** r)
    {
      query* q (new query (*this, t));

      // A null result asks whether the type is supported.
      //
      HRESULT hr (q->create ());

      if (r == nullptr || FAILED (hr))
      {
        q->Release ();

        if (r != nullptr)
          *r = nullptr;

        return hr;
      }

      *r = q;
      return D3D_OK;
    }
  }
}
