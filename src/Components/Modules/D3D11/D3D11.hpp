#pragma once

namespace D3D11 {
	class DXGI;
	class D3D11SwapChain;
	class D3D11Context;
	class D3D11InputLayout;
	class D3D11VertexShader;
	class D3D11PixelShader;
	class D3D11Surface;

	class D3D11Query : public IDirect3DQuery9
	{
	public:
		D3D11Query(D3D11Context* d3d11Context, D3DQUERYTYPE Type);
		virtual ~D3D11Query() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DQuery9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD_(D3DQUERYTYPE, GetType)(THIS) override;
		STDMETHOD_(DWORD, GetDataSize)(THIS) override;
		STDMETHOD(Issue)(THIS_ DWORD dwIssueFlags) override;
		STDMETHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags) override;

		UINT GetDataSizeD3D11();
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		D3DQUERYTYPE m_type;
		CD3D11_QUERY_DESC m_desc;
		Microsoft::WRL::ComPtr<ID3D11Query> m_query;
	};

	class D3D11Context : public IDirect3DDevice9
	{
	public:
		D3D11Context(DXGI* dxgi, IDXGIAdapter* adapter, D3DPRESENT_PARAMETERS* pPresentationParameters);
		virtual ~D3D11Context() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DDevice9 methods ***/
		STDMETHOD(TestCooperativeLevel)(THIS) override;
		STDMETHOD_(UINT, GetAvailableTextureMem)(THIS) override;
		STDMETHOD(EvictManagedResources)(THIS) override;
		STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9) override;
		STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) override;
		STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode) override;
		STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS* pParameters) override;
		STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) override;
		STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags) override;
		STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) override;
		STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) override;
		STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) override;
		STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS) override;
		STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) override;
		STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
		STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
		STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) override;
		STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs) override;
		STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) override;
		STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp) override;
		STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
		STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
		STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) override;
		STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) override;
		STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) override;
		STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface) override;
		STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) override;
		STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) override;
		STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
		STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
		STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) override;
		STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil) override;
		STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface) override;
		STDMETHOD(BeginScene)(THIS) override;
		STDMETHOD(EndScene)(THIS) override;
		STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
		STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override;
		STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override;
		STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*) override;
		STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport) override;
		STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport) override;
		STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial) override;
		STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial) override;
		STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9*) override;
		STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9*) override;
		STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) override;
		STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable) override;
		STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane) override;
		STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane) override;
		STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) override;
		STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue) override;
		STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) override;
		STDMETHOD(BeginStateBlock)(THIS) override;
		STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB) override;
		STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus) override;
		STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus) override;
		STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture) override;
		STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
		STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
		STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
		STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) override;
		STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
		STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses) override;
		STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries) override;
		STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries) override;
		STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber) override;
		STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT* PaletteNumber) override;
		STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect) override;
		STDMETHOD(GetScissorRect)(THIS_ RECT* pRect) override;
		STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware) override;
		STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS) override;
		STDMETHOD(SetNPatchMode)(THIS_ float nSegments) override;
		STDMETHOD_(float, GetNPatchMode)(THIS) override;
		STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
		STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
		STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
		STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
		STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) override;
		STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) override;
		STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl) override;
		STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl) override;
		STDMETHOD(SetFVF)(THIS_ DWORD FVF) override;
		STDMETHOD(GetFVF)(THIS_ DWORD* pFVF) override;
		STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) override;
		STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader) override;
		STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader) override;
		STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
		STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
		STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
		STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
		STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
		STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
		STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) override;
		STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) override;
		STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting) override;
		STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting) override;
		STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData) override;
		STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData) override;
		STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) override;
		STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader) override;
		STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader) override;
		STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
		STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
		STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
		STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
		STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
		STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
		STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) override;
		STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) override;
		STDMETHOD(DeletePatch)(THIS_ UINT Handle) override;
		STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) override;

		void ApplyInternalState();
		ID3D11Device* GetDevice() { return m_pID3D11Device.Get(); }
		ID3D11DeviceContext* GetDeviceContext() { return m_pID3D11DeviceContext.Get(); }
		DXGI* GetDXGI() { return m_dxgi; }
		MOJOSHADER_d3d11Context* GetMojoshaderCtx() { return m_mojoshaderCtx; };

		std::mutex m_mutex;
	private:
		std::atomic<ULONG> m_refCount;

		// in DX11 the concept of a IDirect3DDevice9 is split between:
		// - a ID3D11Device for the resource management
		// - a ID3D11DeviceContext for the command buffer
		Microsoft::WRL::ComPtr<ID3D11Device> m_pID3D11Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pID3D11DeviceContext;

		std::unordered_map<size_t, D3D11InputLayout*> m_inputLayoutCache;
		D3D11InputLayout* m_currentInputLayout;
		bool m_currentInputLayoutDirty = true;

		MOJOSHADER_d3d11Context* m_mojoshaderCtx;
		float* m_vsf = nullptr;
		int* m_vsi = nullptr;
		unsigned char* m_vsb = nullptr;
		float* m_psf = nullptr;
		int* m_psi = nullptr;
		unsigned char* m_psb = nullptr;

		D3D11VertexShader* m_currentVS;
		bool m_currentVSDirty = true;

		D3D11PixelShader* m_currentPS;
		bool m_currentPSDirty = true;

		D3D11Surface* m_currentRenderTargets[D3D_MAX_SIMULTANEOUS_RENDERTARGETS] = {};
		D3D11Surface* m_currentDepthStencilTarget = nullptr;
		bool m_currentTargetsDirty = true;

		std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> m_depthStencilStateCache;
		D3D11_DEPTH_STENCIL_DESC m_currentDSDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
		bool m_currentDSStateDirty = true;
		UINT m_currentStencilRef = 0;

		D3D11_BLEND_DESC currentBlendDesc;
		D3D11_RASTERIZER_DESC currentRasterizerDesc;
		D3D11_SAMPLER_DESC currentSamplerDesc;

		DXGI* m_dxgi;
		D3D11SwapChain* m_swapChain;
	};

	// In D3D9 there is one monitor per adapter, it is no longer the case in D3D11,
	// a IDXGIAdapter (representing the driver) can contain multiple DXGIOutputs (representing the screens)
	// we merge both for our layer to reproduce the D3D9 behavior
	struct D3D11Adapter
	{
		UINT m_index;
		// todo cache desc
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;
		Microsoft::WRL::ComPtr<IDXGIOutput> m_output;
		std::vector<DXGI_MODE_DESC> m_modes;

		HMONITOR GetMonitor();
		HRESULT GetIdentifier(D3DADAPTER_IDENTIFIER9* pIdentifier);
	};

	class DXGI : public IDirect3D9
	{
	public:
		DXGI();
		virtual ~DXGI() {};

		/*** IUnknown methods ***/
		HRESULT  WINAPI QueryInterface(REFIID riid, void** ppvObj) override;
		ULONG    WINAPI AddRef() override;
		ULONG    WINAPI Release() override;

		/*** IDirect3D9 methods ***/
		HRESULT  WINAPI RegisterSoftwareDevice(void* pInitializeFunction) override;
		UINT     WINAPI GetAdapterCount() override;
		HRESULT  WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
		UINT     WINAPI GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) override;
		HRESULT  WINAPI EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;
		HRESULT  WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override;
		HRESULT  WINAPI CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
		HRESULT  WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
		HRESULT  WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
		HRESULT  WINAPI CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
		HRESULT  WINAPI CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
		HRESULT  WINAPI GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
		HMONITOR WINAPI GetAdapterMonitor(UINT Adapter) override;
		HRESULT  WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) override;

		IDXGIFactory* GetFactory() { return m_pIDXGIFactory.Get(); }
	private:
		std::atomic<ULONG> m_refCount;

		Microsoft::WRL::ComPtr<IDXGIFactory> m_pIDXGIFactory;

		std::vector<D3D11Adapter> m_adapters;
	};
}

namespace Components {
	class D3D11 : public Component
	{
	public:
		D3D11();

	private:
		static IDirect3D9* CALLBACK Direct3DCreate9Stub(UINT sdk);
	};
}
