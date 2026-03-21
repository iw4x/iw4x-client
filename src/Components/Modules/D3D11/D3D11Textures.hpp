#pragma once

namespace D3D11
{
	class D3D11Context;
	class D3D11Surface;

	class D3D11SwapChain : public IDirect3DSwapChain9
	{
	public:
		D3D11SwapChain(D3D11Context* d3d11Context, D3DPRESENT_PARAMETERS* pPresentationParameters);
		virtual ~D3D11SwapChain() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DSwapChain9 methods ***/
		STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) override;
		STDMETHOD(GetFrontBufferData)(THIS_ IDirect3DSurface9* pDestSurface) override;
		STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
		STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus) override;
		STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode) override;
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) override;

		ID3D11Texture2D* GetRenderTarget() const { return m_renderTarget.Get(); }
		DXGI_FORMAT GetFormat() const { return m_desc.BufferDesc.Format; }
		HRESULT TestCooperativeLevel();
		void TestBind();
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		D3DPRESENT_PARAMETERS m_params;
		DXGI_SWAP_CHAIN_DESC m_desc;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTarget;
		D3D11Surface* m_backBuffers;
	};

	class D3D11Texture : public IDirect3DTexture9
	{
	public:
		D3D11Texture(D3D11Context* d3d11Context, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
		virtual ~D3D11Texture();

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DResource9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
		STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
		STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
		STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
		STDMETHOD_(DWORD, GetPriority)(THIS) override;
		STDMETHOD_(void, PreLoad)(THIS) override;
		STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;

		/*** IDirect3DBaseTexture9 methods ***/
		STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) override;
		STDMETHOD_(DWORD, GetLOD)(THIS) override;
		STDMETHOD_(DWORD, GetLevelCount)(THIS) override;
		STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) override;
		STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) override;
		STDMETHOD_(void, GenerateMipSubLevels)(THIS) override;

		/*** IDirect3DTexture9 methods ***/
		STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC* pDesc) override;
		STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level, IDirect3DSurface9** ppSurfaceLevel) override;
		STDMETHOD(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
		STDMETHOD(UnlockRect)(THIS_ UINT Level) override;
		STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect) override;

		/*** D3D11Texture methods ***/
		ID3D11Texture2D* GetResource() const { return m_pID3D11Texture2D.Get(); };
		ID3D11ShaderResourceView* GetShaderResourceView();
		const D3D11_TEXTURE2D_DESC& GetDesc() const { return m_desc; }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pID3D11Texture2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pID3D11ShaderResourceView;
		D3D11Surface* m_surface = nullptr; // for RT/DST

		D3D11_TEXTURE2D_DESC m_desc;
		size_t m_formatSize;
		std::vector<std::vector<uint8_t>> m_texturesData;
	};

	class D3D11VolumeTexture : public IDirect3DVolumeTexture9
	{
	public:
		D3D11VolumeTexture(D3D11Context* d3d11Context, UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
		virtual ~D3D11VolumeTexture() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DResource9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
		STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
		STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
		STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
		STDMETHOD_(DWORD, GetPriority)(THIS) override;
		STDMETHOD_(void, PreLoad)(THIS) override;
		STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;

		/*** IDirect3DBaseTexture9 methods ***/
		STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) override;
		STDMETHOD_(DWORD, GetLOD)(THIS) override;
		STDMETHOD_(DWORD, GetLevelCount)(THIS) override;
		STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) override;
		STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) override;
		STDMETHOD_(void, GenerateMipSubLevels)(THIS) override;

		/*** IDirect3DVolumeTexture9 methods ***/
		STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC* pDesc) override;
		STDMETHOD(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume9** ppVolumeLevel) override;
		STDMETHOD(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags) override;
		STDMETHOD(UnlockBox)(THIS_ UINT Level) override;
		STDMETHOD(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox) override;

		/*** D3D11VolumeTexture methods ***/
		ID3D11ShaderResourceView* GetShaderResourceView();
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> m_pID3D11Texture3D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pID3D11ShaderResourceView;

		D3D11_TEXTURE3D_DESC m_desc;
		size_t m_formatSize;
		std::vector<uint8_t> m_textureData;
	};

	class D3D11CubeTexture : public IDirect3DCubeTexture9
	{
	public:
		D3D11CubeTexture(D3D11Context* d3d11Context, UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
		virtual ~D3D11CubeTexture() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DResource9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
		STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
		STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
		STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
		STDMETHOD_(DWORD, GetPriority)(THIS) override;
		STDMETHOD_(void, PreLoad)(THIS) override;
		STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;

		/*** IDirect3DBaseTexture9 methods ***/
		STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) override;
		STDMETHOD_(DWORD, GetLOD)(THIS) override;
		STDMETHOD_(DWORD, GetLevelCount)(THIS) override;
		STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) override;
		STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) override;
		STDMETHOD_(void, GenerateMipSubLevels)(THIS) override;

		/*** IDirect3DCubeTexture9 methods ***/
		STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC* pDesc) override;
		STDMETHOD(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface) override;
		STDMETHOD(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
		STDMETHOD(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType, UINT Level) override;
		STDMETHOD(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect) override;

		/*** D3D11CubeTexture methods ***/
		ID3D11ShaderResourceView* GetShaderResourceView();
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pID3D11Texture2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pID3D11ShaderResourceView;

		D3D11_TEXTURE2D_DESC m_desc;
		size_t m_formatSize;
		std::array<std::vector<std::vector<uint8_t>>, 6> m_texturesData;
	};

	class D3D11Surface : public IDirect3DSurface9
	{
	public:
		D3D11Surface(D3D11Context* d3d11Context, D3D11Texture* parent);
		D3D11Surface(D3D11Context* d3d11Context, D3D11SwapChain* parent);
		virtual ~D3D11Surface();

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DResource9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) override;
		STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
		STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) override;
		STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) override;
		STDMETHOD_(DWORD, GetPriority)(THIS) override;
		STDMETHOD_(void, PreLoad)(THIS) override;
		STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) override;

		/*** IDirect3DSurface9 methods ***/
		STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer) override;
		STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC* pDesc) override;
		STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
		STDMETHOD(UnlockRect)(THIS) override;
		STDMETHOD(GetDC)(THIS_ HDC* phdc) override;
		STDMETHOD(ReleaseDC)(THIS_ HDC hdc) override;

		/*** D3D11Surface methods ***/
		ID3D11RenderTargetView* GetRTV() const { return m_pID3D11RenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDSV() const { return m_pID3D11DepthStencilView.Get(); }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		D3D11Texture* m_parentTexture; // need a way to do the same with swapchain, union????

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pID3D11RenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pID3D11DepthStencilView;
	};
}
