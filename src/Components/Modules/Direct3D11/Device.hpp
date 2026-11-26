#pragma once

#include "Formats.hpp"
#include "Object.hpp"
#include "Resources.hpp"
#include "Shaders.hpp"
#include "States.hpp"

#include <dxgi1_5.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

namespace Components
{
  namespace d3d11
  {
    class direct3d;
    class device;

    // The implicit swap chain.
    //
    // D3D9 renders into the back buffer that the swap chain owns while a
    // DXGI swap chain can't be multisampled (with the flip model, which is
    // what we want on anything recent). So the back buffer that the game
    // sees is a render target of our own, with the format and multisample
    // type it asked for, and Present() resolves (or copies) it into the
    // DXGI back buffer.
    //
    class swap_chain: public unknown<IDirect3DSwapChain9>
    {
    public:
      swap_chain (device&, HWND, const D3DPRESENT_PARAMETERS&);

      HRESULT
      create (IDXGIFactory2*);

      // Recreate the buffers for new parameters, for Reset().
      //
      HRESULT
      reset (const D3DPRESENT_PARAMETERS&);

      STDMETHOD (Present) (const RECT*, const RECT*, HWND, const RGNDATA*, DWORD) override;
      STDMETHOD (GetFrontBufferData) (IDirect3DSurface9*) override;
      STDMETHOD (GetBackBuffer) (UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9**) override;
      STDMETHOD (GetRasterStatus) (D3DRASTER_STATUS*) override;
      STDMETHOD (GetDisplayMode) (D3DDISPLAYMODE*) override;
      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD (GetPresentParameters) (D3DPRESENT_PARAMETERS*) override;

      surface*
      back_buffer () const noexcept
      {
        return back_.get ();
      }

      const D3DPRESENT_PARAMETERS&
      parameters () const noexcept
      {
        return parameters_;
      }

    private:
      HRESULT
      create_back_buffer ();

      UINT
      sync_interval () const noexcept;

      device&                  device_;
      HWND                     window_;
      D3DPRESENT_PARAMETERS    parameters_;
      ComPtr<IDXGISwapChain1>  swap_;
      private_ref<surface>     back_;
      bool                     flip_    = false;
      bool                     tearing_ = false;
    };

    class query: public unknown<IDirect3DQuery9>
    {
    public:
      query (device&, D3DQUERYTYPE);

      HRESULT
      create ();

      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD_ (D3DQUERYTYPE, GetType) () override;
      STDMETHOD_ (DWORD, GetDataSize) () override;
      STDMETHOD (Issue) (DWORD) override;
      STDMETHOD (GetData) (void*, DWORD, DWORD) override;

    private:
      device&             device_;
      D3DQUERYTYPE        type_;
      ComPtr<ID3D11Query> query_;
      bool                issued_ = false;
    };

    // The IDirect3DDevice9 over an ID3D11Device.
    //
    // The device keeps the D3D9 state as the game sets it and translates it
    // into D3D11 state right before each draw (see prepare()), sending only
    // what changed since the last one. Most of it maps one to one but a
    // few parts of D3D9 have no D3D11 counterpart and are emulated by the
    // shaders that dexilifier produces instead, using the Direct3D9State
    // constant buffer:
    //
    // - The alpha test, which the pixel shaders evaluate with the function
    //   and reference that we pass along.
    //
    // - Depth comparison when sampling a depth texture (hardware shadow
    //   maps), which the pixel shaders do themselves for the samplers that
    //   we flag as having a depth texture bound.
    //
    // - The vertex formats, which D3D11 can't all read (D3DCOLOR's byte
    //   order, UDEC3) and which the vertex shaders decode themselves from
    //   the raw data, given the D3DDECLTYPE that we pass along for each
    //   input.
    //
    // - D3D9's pixel centers, which are half a pixel off from D3D11's. The
    //   vertex shaders move the position by the offset that we pass along
    //   for the current viewport.
    //
    // The D3D9 shader constants live in constant buffers that are byte for
    // byte the D3D9 register files (float, integer, and boolean), so the
    // shaders read them at the same registers.
    //
    // The game creates the device with D3DCREATE_MULTITHREADED and does
    // create and lock resources from more than one thread, so every method
    // that touches the device state or the immediate context takes the
    // device mutex (which the resources take as well).
    //
    class device: public unknown<IDirect3DDevice9>
    {
    public:
      device (direct3d&,
              ComPtr<IDXGIAdapter1>,
              HWND focus,
              DWORD behavior,
              const D3DPRESENT_PARAMETERS&);

      ~device () override;

      HRESULT
      create (UINT adapter, D3DDEVTYPE);

      // Accessors for the objects that need the D3D11 side.
      //
      ID3D11Device1*         d3d ()     const noexcept {return device_.Get ();}
      ID3D11DeviceContext1*  context () const noexcept {return context_.Get ();}
      IDXGIAdapter1*         adapter () const noexcept {return adapter_.Get ();}
      std::recursive_mutex&  mutex ()         noexcept {return mutex_;}

      // Log a warning about something the game does that we don't support,
      // once per kind.
      //
      void
      unsupported (const char*);

      // Forget a render target that is going away (see swap_chain::reset()).
      //
      void
      unbind (surface*);

      // IUnknown and IDirect3DDevice9.
      //
      STDMETHOD (TestCooperativeLevel) () override;
      STDMETHOD_ (UINT, GetAvailableTextureMem) () override;
      STDMETHOD (EvictManagedResources) () override;
      STDMETHOD (GetDirect3D) (IDirect3D9**) override;
      STDMETHOD (GetDeviceCaps) (D3DCAPS9*) override;
      STDMETHOD (GetDisplayMode) (UINT, D3DDISPLAYMODE*) override;
      STDMETHOD (GetCreationParameters) (D3DDEVICE_CREATION_PARAMETERS*) override;
      STDMETHOD (SetCursorProperties) (UINT, UINT, IDirect3DSurface9*) override;
      STDMETHOD_ (void, SetCursorPosition) (int, int, DWORD) override;
      STDMETHOD_ (BOOL, ShowCursor) (BOOL) override;
      STDMETHOD (CreateAdditionalSwapChain) (D3DPRESENT_PARAMETERS*, IDirect3DSwapChain9**) override;
      STDMETHOD (GetSwapChain) (UINT, IDirect3DSwapChain9**) override;
      STDMETHOD_ (UINT, GetNumberOfSwapChains) () override;
      STDMETHOD (Reset) (D3DPRESENT_PARAMETERS*) override;
      STDMETHOD (Present) (const RECT*, const RECT*, HWND, const RGNDATA*) override;
      STDMETHOD (GetBackBuffer) (UINT, UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9**) override;
      STDMETHOD (GetRasterStatus) (UINT, D3DRASTER_STATUS*) override;
      STDMETHOD (SetDialogBoxMode) (BOOL) override;
      STDMETHOD_ (void, SetGammaRamp) (UINT, DWORD, const D3DGAMMARAMP*) override;
      STDMETHOD_ (void, GetGammaRamp) (UINT, D3DGAMMARAMP*) override;
      STDMETHOD (CreateTexture) (UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*) override;
      STDMETHOD (CreateVolumeTexture) (UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DVolumeTexture9**, HANDLE*) override;
      STDMETHOD (CreateCubeTexture) (UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture9**, HANDLE*) override;
      STDMETHOD (CreateVertexBuffer) (UINT, DWORD, DWORD, D3DPOOL, IDirect3DVertexBuffer9**, HANDLE*) override;
      STDMETHOD (CreateIndexBuffer) (UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DIndexBuffer9**, HANDLE*) override;
      STDMETHOD (CreateRenderTarget) (UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*) override;
      STDMETHOD (CreateDepthStencilSurface) (UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*) override;
      STDMETHOD (UpdateSurface) (IDirect3DSurface9*, const RECT*, IDirect3DSurface9*, const POINT*) override;
      STDMETHOD (UpdateTexture) (IDirect3DBaseTexture9*, IDirect3DBaseTexture9*) override;
      STDMETHOD (GetRenderTargetData) (IDirect3DSurface9*, IDirect3DSurface9*) override;
      STDMETHOD (GetFrontBufferData) (UINT, IDirect3DSurface9*) override;
      STDMETHOD (StretchRect) (IDirect3DSurface9*, const RECT*, IDirect3DSurface9*, const RECT*, D3DTEXTUREFILTERTYPE) override;
      STDMETHOD (ColorFill) (IDirect3DSurface9*, const RECT*, D3DCOLOR) override;
      STDMETHOD (CreateOffscreenPlainSurface) (UINT, UINT, D3DFORMAT, D3DPOOL, IDirect3DSurface9**, HANDLE*) override;
      STDMETHOD (SetRenderTarget) (DWORD, IDirect3DSurface9*) override;
      STDMETHOD (GetRenderTarget) (DWORD, IDirect3DSurface9**) override;
      STDMETHOD (SetDepthStencilSurface) (IDirect3DSurface9*) override;
      STDMETHOD (GetDepthStencilSurface) (IDirect3DSurface9**) override;
      STDMETHOD (BeginScene) () override;
      STDMETHOD (EndScene) () override;
      STDMETHOD (Clear) (DWORD, const D3DRECT*, DWORD, D3DCOLOR, float, DWORD) override;
      STDMETHOD (SetTransform) (D3DTRANSFORMSTATETYPE, const D3DMATRIX*) override;
      STDMETHOD (GetTransform) (D3DTRANSFORMSTATETYPE, D3DMATRIX*) override;
      STDMETHOD (MultiplyTransform) (D3DTRANSFORMSTATETYPE, const D3DMATRIX*) override;
      STDMETHOD (SetViewport) (const D3DVIEWPORT9*) override;
      STDMETHOD (GetViewport) (D3DVIEWPORT9*) override;
      STDMETHOD (SetMaterial) (const D3DMATERIAL9*) override;
      STDMETHOD (GetMaterial) (D3DMATERIAL9*) override;
      STDMETHOD (SetLight) (DWORD, const D3DLIGHT9*) override;
      STDMETHOD (GetLight) (DWORD, D3DLIGHT9*) override;
      STDMETHOD (LightEnable) (DWORD, BOOL) override;
      STDMETHOD (GetLightEnable) (DWORD, BOOL*) override;
      STDMETHOD (SetClipPlane) (DWORD, const float*) override;
      STDMETHOD (GetClipPlane) (DWORD, float*) override;
      STDMETHOD (SetRenderState) (D3DRENDERSTATETYPE, DWORD) override;
      STDMETHOD (GetRenderState) (D3DRENDERSTATETYPE, DWORD*) override;
      STDMETHOD (CreateStateBlock) (D3DSTATEBLOCKTYPE, IDirect3DStateBlock9**) override;
      STDMETHOD (BeginStateBlock) () override;
      STDMETHOD (EndStateBlock) (IDirect3DStateBlock9**) override;
      STDMETHOD (SetClipStatus) (const D3DCLIPSTATUS9*) override;
      STDMETHOD (GetClipStatus) (D3DCLIPSTATUS9*) override;
      STDMETHOD (GetTexture) (DWORD, IDirect3DBaseTexture9**) override;
      STDMETHOD (SetTexture) (DWORD, IDirect3DBaseTexture9*) override;
      STDMETHOD (GetTextureStageState) (DWORD, D3DTEXTURESTAGESTATETYPE, DWORD*) override;
      STDMETHOD (SetTextureStageState) (DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) override;
      STDMETHOD (GetSamplerState) (DWORD, D3DSAMPLERSTATETYPE, DWORD*) override;
      STDMETHOD (SetSamplerState) (DWORD, D3DSAMPLERSTATETYPE, DWORD) override;
      STDMETHOD (ValidateDevice) (DWORD*) override;
      STDMETHOD (SetPaletteEntries) (UINT, const PALETTEENTRY*) override;
      STDMETHOD (GetPaletteEntries) (UINT, PALETTEENTRY*) override;
      STDMETHOD (SetCurrentTexturePalette) (UINT) override;
      STDMETHOD (GetCurrentTexturePalette) (UINT*) override;
      STDMETHOD (SetScissorRect) (const RECT*) override;
      STDMETHOD (GetScissorRect) (RECT*) override;
      STDMETHOD (SetSoftwareVertexProcessing) (BOOL) override;
      STDMETHOD_ (BOOL, GetSoftwareVertexProcessing) () override;
      STDMETHOD (SetNPatchMode) (float) override;
      STDMETHOD_ (float, GetNPatchMode) () override;
      STDMETHOD (DrawPrimitive) (D3DPRIMITIVETYPE, UINT, UINT) override;
      STDMETHOD (DrawIndexedPrimitive) (D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT) override;
      STDMETHOD (DrawPrimitiveUP) (D3DPRIMITIVETYPE, UINT, const void*, UINT) override;
      STDMETHOD (DrawIndexedPrimitiveUP) (D3DPRIMITIVETYPE, UINT, UINT, UINT, const void*, D3DFORMAT, const void*, UINT) override;
      STDMETHOD (ProcessVertices) (UINT, UINT, UINT, IDirect3DVertexBuffer9*, IDirect3DVertexDeclaration9*, DWORD) override;
      STDMETHOD (CreateVertexDeclaration) (const D3DVERTEXELEMENT9*, IDirect3DVertexDeclaration9**) override;
      STDMETHOD (SetVertexDeclaration) (IDirect3DVertexDeclaration9*) override;
      STDMETHOD (GetVertexDeclaration) (IDirect3DVertexDeclaration9**) override;
      STDMETHOD (SetFVF) (DWORD) override;
      STDMETHOD (GetFVF) (DWORD*) override;
      STDMETHOD (CreateVertexShader) (const DWORD*, IDirect3DVertexShader9**) override;
      STDMETHOD (SetVertexShader) (IDirect3DVertexShader9*) override;
      STDMETHOD (GetVertexShader) (IDirect3DVertexShader9**) override;
      STDMETHOD (SetVertexShaderConstantF) (UINT, const float*, UINT) override;
      STDMETHOD (GetVertexShaderConstantF) (UINT, float*, UINT) override;
      STDMETHOD (SetVertexShaderConstantI) (UINT, const int*, UINT) override;
      STDMETHOD (GetVertexShaderConstantI) (UINT, int*, UINT) override;
      STDMETHOD (SetVertexShaderConstantB) (UINT, const BOOL*, UINT) override;
      STDMETHOD (GetVertexShaderConstantB) (UINT, BOOL*, UINT) override;
      STDMETHOD (SetStreamSource) (UINT, IDirect3DVertexBuffer9*, UINT, UINT) override;
      STDMETHOD (GetStreamSource) (UINT, IDirect3DVertexBuffer9**, UINT*, UINT*) override;
      STDMETHOD (SetStreamSourceFreq) (UINT, UINT) override;
      STDMETHOD (GetStreamSourceFreq) (UINT, UINT*) override;
      STDMETHOD (SetIndices) (IDirect3DIndexBuffer9*) override;
      STDMETHOD (GetIndices) (IDirect3DIndexBuffer9**) override;
      STDMETHOD (CreatePixelShader) (const DWORD*, IDirect3DPixelShader9**) override;
      STDMETHOD (SetPixelShader) (IDirect3DPixelShader9*) override;
      STDMETHOD (GetPixelShader) (IDirect3DPixelShader9**) override;
      STDMETHOD (SetPixelShaderConstantF) (UINT, const float*, UINT) override;
      STDMETHOD (GetPixelShaderConstantF) (UINT, float*, UINT) override;
      STDMETHOD (SetPixelShaderConstantI) (UINT, const int*, UINT) override;
      STDMETHOD (GetPixelShaderConstantI) (UINT, int*, UINT) override;
      STDMETHOD (SetPixelShaderConstantB) (UINT, const BOOL*, UINT) override;
      STDMETHOD (GetPixelShaderConstantB) (UINT, BOOL*, UINT) override;
      STDMETHOD (DrawRectPatch) (UINT, const float*, const D3DRECTPATCH_INFO*) override;
      STDMETHOD (DrawTriPatch) (UINT, const float*, const D3DTRIPATCH_INFO*) override;
      STDMETHOD (DeletePatch) (UINT) override;
      STDMETHOD (CreateQuery) (D3DQUERYTYPE, IDirect3DQuery9**) override;

      // Validate the description and create the image for a texture or a
      // surface. Return the D3D9 error if it's not valid.
      //
      HRESULT
      create_image (image_desc, std::unique_ptr<image>&);

    private:
      // The D3D9 register files. The sizes are the Shader Model 3 limits.
      //
      static constexpr UINT vertex_floats   = 256;
      static constexpr UINT pixel_floats    = 224;
      static constexpr UINT integers        = 16;
      static constexpr UINT booleans        = 16;

      // The D3D9 samplers: 16 for the pixel shader then 4 for the vertex
      // shader (D3DVERTEXTEXTURESAMPLER0 and up), which is how we index
      // them throughout.
      //
      static constexpr UINT pixel_samplers  = 16;
      static constexpr UINT vertex_samplers = 4;
      static constexpr UINT samplers        = pixel_samplers + vertex_samplers;

      static constexpr UINT targets         = 4;
      static constexpr UINT streams         = 16;

      // Return the sampler index of a D3D9 sampler number. Return samplers
      // if it's not a valid one.
      //
      static UINT
      sampler_index (DWORD) noexcept;

      // Return true if the environment asks for the D3D11 debug layer
      // (IW4X_D3D11_DEBUG set to anything but 0).
      //
      static bool
      debug_layer () noexcept;

      // The register files of a stage, laid out the way the shaders declare
      // them: four components per float and integer register and one
      // boolean per component, so four per register.
      //
      struct stage_constants
      {
        std::array<float, 4 * vertex_floats> floats;
        std::array<int,   4 * integers>      ints;
        std::array<BOOL,  booleans>          bools;
        bool                                 dirty_floats;
        bool                                 dirty_ints;
        bool                                 dirty_bools;
      };

      // The two Direct3D9State buffers, as the shaders declare them.
      //
      struct vertex_state
      {
        float positionFixup[4];
        UINT  inputTypes[16];
      };

      struct pixel_state
      {
        UINT alphaFunction;
        UINT alphaReference;
        UINT depthSamplers;
        UINT linearDepthSamplers;
      };

      struct stream
      {
        private_ref<vertex_buffer> buffer;
        IDirect3DVertexBuffer9*    com    = nullptr;
        UINT                       offset = 0;
        UINT                       stride = 0;
        UINT                       freq   = 1;
      };

      struct bound_texture
      {
        private_ref<texture_object> object;
        IDirect3DBaseTexture9*      com = nullptr;
      };

      // What changed since the last draw. Textures and samplers have masks
      // of their own, one bit per sampler.
      //
      enum dirt: uint32_t
      {
        dirty_targets  = 1 << 0,
        dirty_viewport = 1 << 1,
        dirty_scissor  = 1 << 2,
        dirty_blend    = 1 << 3,
        dirty_depth    = 1 << 4,
        dirty_raster   = 1 << 5,
        dirty_shaders  = 1 << 6,
        dirty_layout   = 1 << 7,
        dirty_streams  = 1 << 8,
        dirty_indices  = 1 << 9,
        dirty_pixel    = 1 << 10, // Alpha test part of pixel Direct3D9State.
        dirty_all      = 0xFFFFFFFF
      };

      // Return the device to its state after creation and Reset().
      //
      void
      reset_state ();

      // Create the automatic depth stencil, if the present parameters ask
      // for one.
      //
      HRESULT
      create_auto_depth ();

      // Send the D3D9 state to D3D11 for a draw of the primitive type.
      // Return false if the draw can't be done (no shader, say), in which
      // case the caller skips it.
      //
      bool
      prepare (D3DPRIMITIVETYPE);

      void
      apply_targets ();

      void
      apply_viewport ();

      void
      apply_constants ();

      void
      upload (ID3D11Buffer*, const void*, size_t);

      HRESULT
      create_surface (const image_desc&, IDirect3DSurface9**);

      // Return the size of the render target bound at slot 0. Return the
      // depth stencil's if there is no render target.
      //
      std::pair<UINT, UINT>
      target_size () const noexcept;

      // Copy the data of a draw from user memory into the buffer we draw
      // it from, growing it as needed. Return the offset of the copy.
      // Return UINT_MAX if we couldn't make the copy.
      //
      UINT
      append_up (ComPtr<ID3D11Buffer>&,
                 UINT& position,
                 UINT& capacity,
                 UINT bind,
                 const void*,
                 UINT size);

      direct3d&                    parent_;
      ComPtr<IDXGIAdapter1>        adapter_;
      HWND                         focus_;
      DWORD                        behavior_;
      D3DPRESENT_PARAMETERS        parameters_;
      UINT                         adapter_index_ = 0;
      D3DDEVTYPE                   type_          = D3DDEVTYPE_HAL;

      std::recursive_mutex         mutex_;
      ComPtr<ID3D11Device1>        device_;
      ComPtr<ID3D11DeviceContext1> context_;
      ComPtr<IDXGIFactory2>        factory_;
      private_ref<swap_chain>      swap_;
      private_ref<surface>         auto_depth_;

      std::set<std::string>        unsupported_;

      // The D3D9 state.
      //
      render_states                             rs_;
      std::array<sampler_states, samplers>      ss_;
      std::array<bound_texture, samplers>       textures_;
      std::array<private_ref<surface>, targets> targets_;
      private_ref<surface>                      depth_;
      private_ref<vertex_shader>                vs_;
      private_ref<pixel_shader>                 ps_;
      private_ref<vertex_declaration>           decl_;
      std::array<stream, streams>               streams_;
      private_ref<index_buffer>                 indices_;
      D3DVIEWPORT9                              viewport_ {};
      RECT                                      scissor_ {};
      stage_constants                           vertex_ {};
      stage_constants                           pixel_ {};
      D3DGAMMARAMP                              gamma_ {};
      DWORD                                     fvf_    = 0;
      BOOL                                      cursor_ = FALSE;
      bool                                      scene_  = false;
      uint32_t                                  dirty_  = dirty_all;

      // The fixed function state, which we only keep (see prepare()).
      //
      std::map<DWORD, D3DMATRIX>                     transforms_;
      std::map<DWORD, std::pair<D3DLIGHT9, BOOL>>    lights_;
      std::array<std::array<float, 4>, 6>            clip_planes_ {};
      std::array<std::array<DWORD, 33>, 8>           stages_ {};
      D3DMATERIAL9                                   material_ {};
      D3DCLIPSTATUS9                                 clip_status_ {};

      // The D3D11 side: the constant buffers b0 to b3 of each stage, the
      // absent input, and the buffers of the draws from user memory.
      //
      std::array<ComPtr<ID3D11Buffer>, 4> vertex_buffers_;
      std::array<ComPtr<ID3D11Buffer>, 4> pixel_buffers_;
      ComPtr<ID3D11Buffer>                absent_;
      ComPtr<ID3D11Buffer>                up_vertices_;
      ComPtr<ID3D11Buffer>                up_indices_;
      UINT                                up_vertex_position_ = 0;
      UINT                                up_vertex_capacity_ = 0;
      UINT                                up_index_position_  = 0;
      UINT                                up_index_capacity_  = 0;

      state_cache<D3D11_BLEND_DESC,         ID3D11BlendState>        blend_states_;
      state_cache<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState> depth_states_;
      state_cache<D3D11_RASTERIZER_DESC,    ID3D11RasterizerState>   raster_states_;
      state_cache<D3D11_SAMPLER_DESC,       ID3D11SamplerState>      sampler_states_;
      layout_cache                                                   layouts_;

      // What we last sent, to skip redundant calls.
      //
      const input_layout*                             layout_ = nullptr;
      D3DPRIMITIVETYPE                                topology_ {};
      vertex_state                                    vertex_state_ {};
      pixel_state                                     pixel_state_ {};
      bool                                            vertex_state_valid_ = false;
      bool                                            pixel_state_valid_  = false;
      std::array<ID3D11ShaderResourceView*, samplers> views_ {};
      std::array<ID3D11SamplerState*, samplers>       sampler_objects_ {};
      uint32_t                                        texture_dirty_ = ~0u;
      uint32_t                                        sampler_dirty_ = ~0u;
    };
  }
}
