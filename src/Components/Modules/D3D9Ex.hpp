#pragma once

namespace Components
{
	class D3D9Ex : public Component
	{
	public:
		D3D9Ex();

	private:
		class D3D9Device : public IDirect3DDevice9
		{
		public:
			D3D9Device(IDirect3DDevice9* pOriginal) : m_pIDirect3DDevice9(pOriginal) {}
			virtual ~D3D9Device() = default;

			HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj) override;
			ULONG   WINAPI AddRef() override;
			ULONG   WINAPI Release() override;
			HRESULT WINAPI TestCooperativeLevel() override;
			UINT    WINAPI GetAvailableTextureMem() override;
			HRESULT WINAPI EvictManagedResources() override;
			HRESULT WINAPI GetDirect3D(IDirect3D9** ppD3D9) override;
			HRESULT WINAPI GetDeviceCaps(D3DCAPS9* pCaps) override;
			HRESULT WINAPI GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) override;
			HRESULT WINAPI GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) override;
			HRESULT WINAPI SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) override;
			void    WINAPI SetCursorPosition(int X, int Y, DWORD Flags) override;
			BOOL    WINAPI ShowCursor(BOOL bShow) override;
			HRESULT WINAPI CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) override;
			HRESULT WINAPI GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) override;
			UINT    WINAPI GetNumberOfSwapChains() override;
			HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override;
			HRESULT WINAPI Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
			HRESULT WINAPI GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
			HRESULT WINAPI GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) override;
			HRESULT WINAPI SetDialogBoxMode(BOOL bEnableDialogs) override;
			void    WINAPI SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) override;
			void    WINAPI GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) override;
			HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
			HRESULT WINAPI CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
			HRESULT WINAPI UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) override;
			HRESULT WINAPI UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) override;
			HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) override;
			HRESULT WINAPI GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) override;
			HRESULT WINAPI StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) override;
			HRESULT WINAPI ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) override;
			HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
			HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
			HRESULT WINAPI GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) override;
			HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) override;
			HRESULT WINAPI GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) override;
			HRESULT WINAPI BeginScene() override;
			HRESULT WINAPI EndScene() override;
			HRESULT WINAPI Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
			HRESULT WINAPI SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override;
			HRESULT WINAPI GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override;
			HRESULT WINAPI MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override;
			HRESULT WINAPI SetViewport(CONST D3DVIEWPORT9* pViewport) override;
			HRESULT WINAPI GetViewport(D3DVIEWPORT9* pViewport) override;
			HRESULT WINAPI SetMaterial(CONST D3DMATERIAL9* pMaterial) override;
			HRESULT WINAPI GetMaterial(D3DMATERIAL9* pMaterial) override;
			HRESULT WINAPI SetLight(DWORD Index, CONST D3DLIGHT9* pLight) override;
			HRESULT WINAPI GetLight(DWORD Index, D3DLIGHT9* pLight) override;
			HRESULT WINAPI LightEnable(DWORD Index, BOOL Enable) override;
			HRESULT WINAPI GetLightEnable(DWORD Index, BOOL* pEnable) override;
			HRESULT WINAPI SetClipPlane(DWORD Index, CONST float* pPlane) override;
			HRESULT WINAPI GetClipPlane(DWORD Index, float* pPlane) override;
			HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;
			HRESULT WINAPI GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) override;
			HRESULT WINAPI CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) override;
			HRESULT WINAPI BeginStateBlock() override;
			HRESULT WINAPI EndStateBlock(IDirect3DStateBlock9** ppSB) override;
			HRESULT WINAPI SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) override;
			HRESULT WINAPI GetClipStatus(D3DCLIPSTATUS9* pClipStatus) override;
			HRESULT WINAPI GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) override;
			HRESULT WINAPI SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
			HRESULT WINAPI GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
			HRESULT WINAPI SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
			HRESULT WINAPI GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) override;
			HRESULT WINAPI SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
			HRESULT WINAPI ValidateDevice(DWORD* pNumPasses) override;
			HRESULT WINAPI SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) override;
			HRESULT WINAPI GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) override;
			HRESULT WINAPI SetCurrentTexturePalette(UINT PaletteNumber) override;
			HRESULT WINAPI GetCurrentTexturePalette(UINT *PaletteNumber) override;
			HRESULT WINAPI SetScissorRect(CONST RECT* pRect) override;
			HRESULT WINAPI GetScissorRect(RECT* pRect) override;
			HRESULT WINAPI SetSoftwareVertexProcessing(BOOL bSoftware) override;
			BOOL    WINAPI GetSoftwareVertexProcessing() override;
			HRESULT WINAPI SetNPatchMode(float nSegments) override;
			float   WINAPI GetNPatchMode() override;
			HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
			HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
			HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
			HRESULT WINAPI DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
			HRESULT WINAPI ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) override;
			HRESULT WINAPI CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) override;
			HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) override;
			HRESULT WINAPI GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) override;
			HRESULT WINAPI SetFVF(DWORD FVF) override;
			HRESULT WINAPI GetFVF(DWORD* pFVF) override;
			HRESULT WINAPI CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) override;
			HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9* pShader) override;
			HRESULT WINAPI GetVertexShader(IDirect3DVertexShader9** ppShader) override;
			HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
			HRESULT WINAPI GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
			HRESULT WINAPI SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
			HRESULT WINAPI GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
			HRESULT WINAPI SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
			HRESULT WINAPI GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
			HRESULT WINAPI SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) override;
			HRESULT WINAPI GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride) override;
			HRESULT WINAPI SetStreamSourceFreq(UINT StreamNumber, UINT Divider) override;
			HRESULT WINAPI GetStreamSourceFreq(UINT StreamNumber, UINT* Divider) override;
			HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9* pIndexData) override;
			HRESULT WINAPI GetIndices(IDirect3DIndexBuffer9** ppIndexData) override;
			HRESULT WINAPI CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) override;
			HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9* pShader) override;
			HRESULT WINAPI GetPixelShader(IDirect3DPixelShader9** ppShader) override;
			HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) override;
			HRESULT WINAPI GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
			HRESULT WINAPI SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) override;
			HRESULT WINAPI GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
			HRESULT WINAPI SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) override;
			HRESULT WINAPI GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
			HRESULT WINAPI DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) override;
			HRESULT WINAPI DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) override;
			HRESULT WINAPI DeletePatch(UINT Handle) override;
			HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) override;

		private:
			IDirect3DDevice9 *m_pIDirect3DDevice9;
		};

		class D3D9 : public IDirect3D9
		{
		public:
			D3D9(IDirect3D9Ex *pOriginal) : m_pIDirect3D9(pOriginal) {};
			virtual ~D3D9() {};

			HRESULT  WINAPI QueryInterface(REFIID riid, void** ppvObj) override;
			ULONG    WINAPI AddRef() override;
			ULONG    WINAPI Release() override;
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

		private:
			IDirect3D9 *m_pIDirect3D9;
		};

		static Dvar::Var RUseD3D9Ex;

		static IDirect3D9* CALLBACK Direct3DCreate9Stub(UINT sdk);
	};
}
