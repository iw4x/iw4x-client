#include <STDInclude.hpp>
#include "D3D11Utils.hpp"
#include "D3D11Textures.hpp"
#include "D3D11.hpp"

#pragma region D3D11Surface
D3D11::D3D11Surface::D3D11Surface(D3D11Context* ctx, D3D11Texture* parent) : m_refCount(0), m_d3dCtx(ctx), m_parentTexture(parent)
{
	auto desc = parent->GetDesc();
	if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) != 0) {
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, desc.Format);
		ctx->GetDevice()->CreateRenderTargetView(
			parent->GetResource(),
			&renderTargetViewDesc,
			m_pID3D11RenderTargetView.ReleaseAndGetAddressOf()
		);
	} else if ((desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0) {
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		ctx->GetDevice()->CreateDepthStencilView(
			parent->GetResource(),
			&depthStencilViewDesc,
			m_pID3D11DepthStencilView.ReleaseAndGetAddressOf()
		);
	} else {
		NOT_IMPLEMENTED
	}

	// need to check if the refcounting is right to avoid surface leak or unexpected delete
	if (parent) parent->AddRef();
}

D3D11::D3D11Surface::D3D11Surface(D3D11Context* ctx, D3D11SwapChain* parent) : m_refCount(0), m_d3dCtx(ctx), m_parentTexture(nullptr)
{
	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, parent->GetFormat());
	ctx->GetDevice()->CreateRenderTargetView(
		parent->GetRenderTarget(),
		&renderTargetViewDesc,
		m_pID3D11RenderTargetView.ReleaseAndGetAddressOf()
	);
}

D3D11::D3D11Surface::~D3D11Surface()
{
	if (m_parentTexture) m_parentTexture->Release();
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11Surface::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11Surface::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11Surface::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11Surface::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11Surface::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Surface::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Surface::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11Surface::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11Surface::GetType()
{
	return D3DRTYPE_SURFACE;
}

/*** IDirect3DSurface9 methods ***/
HRESULT D3D11::D3D11Surface::GetContainer(REFIID riid, void** ppContainer)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::GetDesc(D3DSURFACE_DESC* pDesc)
{
	*pDesc = {
		D3DFMT_A8R8G8B8, // D3DFORMAT           Format;
		D3DRTYPE_SURFACE, // D3DRESOURCETYPE     Type;
		D3DUSAGE_RENDERTARGET, // DWORD               Usage;
		D3DPOOL_DEFAULT, // D3DPOOL             Pool;
		D3DMULTISAMPLE_4_SAMPLES, // D3DMULTISAMPLE_TYPE MultiSampleType;
		0, // DWORD               MultiSampleQuality;
		1360, // UINT                Width;
		768, // UINT                Height;
	};
	return D3D_OK;
}
HRESULT D3D11::D3D11Surface::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::UnlockRect()
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::GetDC(HDC* phdc)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Surface::ReleaseDC(HDC hdc)
{
	NOT_IMPLEMENTED_ERROR
}
#pragma endregion

#pragma region D3D11SwapChain
D3D11::D3D11SwapChain::D3D11SwapChain(D3D11Context* ctx, D3DPRESENT_PARAMETERS* pPresentationParameters) : m_refCount(0), m_d3dCtx(ctx), m_params(*pPresentationParameters)
{
	m_desc = {};
	m_desc.BufferDesc.Width = pPresentationParameters->BackBufferWidth;
	m_desc.BufferDesc.Height = pPresentationParameters->BackBufferHeight;
	m_desc.BufferDesc.RefreshRate = { 60, 1 };
	m_desc.BufferDesc.Format = D3DFORMAT_to_DXGI_FORMAT(pPresentationParameters->BackBufferFormat);
	m_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_desc.BufferCount = pPresentationParameters->BackBufferCount;
	m_desc.OutputWindow = pPresentationParameters->hDeviceWindow;
	m_desc.Windowed = true;
	m_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	m_desc.Flags = 0;

	ctx->GetDXGI()->GetFactory()->CreateSwapChain(
		ctx->GetDevice(),
		&m_desc,
		m_swapChain.ReleaseAndGetAddressOf()
	);

	m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_renderTarget.ReleaseAndGetAddressOf()));
	// tmp
	m_backBuffers = new D3D11Surface(ctx, this);
	m_backBuffers->AddRef();
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11SwapChain::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11SwapChain::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11SwapChain::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DSwapChain9 methods ***/
HRESULT D3D11::D3D11SwapChain::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_swapChain->Present(1, 0);
	//NOT_IMPLEMENTED
	return D3D_OK;
}
HRESULT D3D11::D3D11SwapChain::GetFrontBufferData(IDirect3DSurface9* pDestSurface)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11SwapChain::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	*ppBackBuffer = m_backBuffers;
	m_backBuffers->AddRef();
	return D3D_OK;
}
HRESULT D3D11::D3D11SwapChain::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11SwapChain::GetDisplayMode(D3DDISPLAYMODE* pMode)
{
	pMode->Width = m_params.BackBufferWidth;
	pMode->Height = m_params.BackBufferHeight;
	pMode->Format = m_params.BackBufferFormat;
	pMode->RefreshRate = m_params.FullScreen_RefreshRateInHz;
	return D3D_OK;
}
HRESULT D3D11::D3D11SwapChain::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11SwapChain::GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	*pPresentationParameters = m_params;
	return D3D_OK;
}
HRESULT D3D11::D3D11SwapChain::TestCooperativeLevel()
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	return m_swapChain->Present(0, DXGI_PRESENT_TEST);
}
void D3D11::D3D11SwapChain::TestBind()
{
	auto m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(m_params.BackBufferWidth),
		static_cast<float>(m_params.BackBufferHeight)
	);
	float color[4] = {1, 0, 1, 1};
	//m_d3dCtx->GetDeviceContext()->ClearRenderTargetView(renderTarget, color);
	//m_d3dCtx->GetDeviceContext()->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_d3dCtx->GetDeviceContext()->RSSetViewports(1, &m_screenViewport);
	//m_d3dCtx->GetDeviceContext()->OMSetRenderTargets(1, &renderTarget, NULL);
}
#pragma endregion

#pragma region D3D11Texture
D3D11::D3D11Texture::D3D11Texture(D3D11Context* ctx, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) : m_refCount(0), m_d3dCtx(ctx)
{
	if (Levels == 0)
		Levels = 1 + std::max(log2(Width), log2(Height));

	DXGI_FORMAT d3d11Format = D3DFORMAT_to_DXGI_FORMAT(Format);
	m_texturesData.resize(Levels);
	m_formatSize = GetFormatSize(d3d11Format);
	uint32_t blockSize = GetBlockSize(d3d11Format);

	for (int i = 0; i < Levels; i++) {
		uint32_t width = std::max(1u, Width / (1 << i));
		uint32_t height = std::max(1u, Height / (1 << i));

		uint32_t textureSize = (AlignUp(width, blockSize) * AlignUp(height, blockSize) * m_formatSize) / (blockSize * blockSize);
		m_texturesData[i].resize(textureSize);
	}

	UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;
	if ((Usage & D3DUSAGE_DEPTHSTENCIL) != 0) {
		bindFlags = D3D11_BIND_DEPTH_STENCIL;
	}
	if ((Usage & D3DUSAGE_RENDERTARGET) != 0) {
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	//todo add dynamic
	m_desc = CD3D11_TEXTURE2D_DESC(
		d3d11Format,
		Width, Height, 1,
		Levels,
		bindFlags,
		D3D11_USAGE_DEFAULT
	);

	m_d3dCtx->GetDevice()->CreateTexture2D(&m_desc, NULL, m_pID3D11Texture2D.ReleaseAndGetAddressOf());

	if ((Usage & D3DUSAGE_DEPTHSTENCIL) == 0) {
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D11_SRV_DIMENSION_TEXTURE2D,
			d3d11Format,
			0, Levels
		);
		m_d3dCtx->GetDevice()->CreateShaderResourceView(m_pID3D11Texture2D.Get(), &srvDesc, m_pID3D11ShaderResourceView.ReleaseAndGetAddressOf());
	}

	if ((Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL)) != 0) {
		m_surface = new D3D11Surface(ctx, this);
		m_surface->AddRef();
	}
}

D3D11::D3D11Texture::~D3D11Texture() {
	if(m_surface)
		m_surface->Release();
	m_surface = nullptr;
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11Texture::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11Texture::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11Texture::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11Texture::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11Texture::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Texture::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Texture::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Texture::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Texture::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11Texture::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11Texture::GetType()
{
	return D3DRTYPE_TEXTURE;
}

/*** IDirect3DBaseTexture9 methods ***/
DWORD D3D11::D3D11Texture::SetLOD(DWORD LODNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Texture::GetLOD()
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11Texture::GetLevelCount()
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Texture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	NOT_IMPLEMENTED_ERROR
}
D3DTEXTUREFILTERTYPE D3D11::D3D11Texture::GetAutoGenFilterType()
{
	NOT_IMPLEMENTED
	return D3DTEXF_NONE;
}
void D3D11::D3D11Texture::GenerateMipSubLevels()
{
	NOT_IMPLEMENTED
}

/*** IDirect3DTexture9 methods ***/
HRESULT D3D11::D3D11Texture::GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11Texture::GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel)
{
	assert(m_surface);
	m_surface->AddRef();
	*ppSurfaceLevel = m_surface;
	return D3D_OK;
}
HRESULT D3D11::D3D11Texture::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	assert(pRect == NULL);

	//implement USAGE_DYNAMIC with maps
	uint32_t width = std::max(1u, m_desc.Width / (1 << Level));
	uint32_t blockSize = GetBlockSize(m_desc.Format);

	*pLockedRect = {
		(INT)(width * m_formatSize / blockSize),
		m_texturesData[Level].data()
	};
	return D3D_OK;
}

HRESULT D3D11::D3D11Texture::UnlockRect(UINT Level)
{
	uint32_t width = std::max(1u, m_desc.Width / (1 << Level));
	uint32_t height = std::max(1u, m_desc.Height / (1 << Level));
	uint32_t blockSize = GetBlockSize(m_desc.Format);

	CD3D11_BOX box(
		0, 0, 0,
		AlignUp(width, blockSize), AlignUp(height, blockSize), 1
	);
	uint32_t rowPitch = (box.right * m_formatSize) / (blockSize);
	uint32_t depthPitch = (box.right * box.bottom * m_formatSize) / (blockSize * blockSize);

	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_d3dCtx->GetDeviceContext()->UpdateSubresource(
		m_pID3D11Texture2D.Get(),
		Level,
		&box,
		m_texturesData[Level].data(),
		(INT)rowPitch,
		(INT)depthPitch
	);
	return D3D_OK;
}
HRESULT D3D11::D3D11Texture::AddDirtyRect(CONST RECT* pDirtyRect)
{
	NOT_IMPLEMENTED_ERROR
}

/*** D3D11Texture methods ***/
ID3D11ShaderResourceView* D3D11::D3D11Texture::GetShaderResourceView()
{
	return m_pID3D11ShaderResourceView.Get();
}
#pragma endregion

#pragma region D3D11VolumeTexture
D3D11::D3D11VolumeTexture::D3D11VolumeTexture(D3D11Context* ctx, UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) : m_refCount(0), m_d3dCtx(ctx)
{
	assert(Levels == 1);
	assert(Usage == 0);

	DXGI_FORMAT d3d11Format = D3DFORMAT_to_DXGI_FORMAT(Format);
	m_desc = CD3D11_TEXTURE3D_DESC(
		d3d11Format,
		Width, Height, Depth,
		Levels,
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_USAGE_DEFAULT
	);

	m_d3dCtx->GetDevice()->CreateTexture3D(&m_desc, NULL, m_pID3D11Texture3D.ReleaseAndGetAddressOf());

	m_formatSize = GetFormatSize(d3d11Format);
	uint32_t blockSize = GetBlockSize(d3d11Format);
	size_t textureSize = (Width * Height * Depth * m_formatSize) / (blockSize * blockSize);
	m_textureData.resize(textureSize);
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11VolumeTexture::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11VolumeTexture::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11VolumeTexture::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11VolumeTexture::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11VolumeTexture::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VolumeTexture::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VolumeTexture::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VolumeTexture::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VolumeTexture::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11VolumeTexture::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11VolumeTexture::GetType()
{
	return D3DRTYPE_VOLUMETEXTURE;
}

/*** IDirect3DBaseTexture9 methods ***/
DWORD D3D11::D3D11VolumeTexture::SetLOD(DWORD LODNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VolumeTexture::GetLOD()
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VolumeTexture::GetLevelCount()
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VolumeTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	NOT_IMPLEMENTED_ERROR
}
D3DTEXTUREFILTERTYPE D3D11::D3D11VolumeTexture::GetAutoGenFilterType()
{
	NOT_IMPLEMENTED
	return D3DTEXF_NONE;
}
void D3D11::D3D11VolumeTexture::GenerateMipSubLevels()
{
	NOT_IMPLEMENTED
}

/*** IDirect3DTexture9 methods ***/
HRESULT D3D11::D3D11VolumeTexture::GetLevelDesc(UINT Level, D3DVOLUME_DESC* pDesc)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VolumeTexture::GetVolumeLevel(UINT Level, IDirect3DVolume9** ppVolumeLevel)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VolumeTexture::LockBox(UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags)
{
	assert(Level == 0);
	assert(pBox == NULL);

	// TODO:
	// - implement USAGE_DYNAMIC textures with Map()
	// - implement pBox if used by the game (need storing for UpdateSubresource)
	// - mipmapped volume texture???? (seems unlikely)
	uint32_t blockSize = GetBlockSize(m_desc.Format);
	*pLockedVolume = {
		(INT)(m_desc.Width * m_formatSize / blockSize),
		(INT)(m_desc.Width * m_desc.Height * m_formatSize / blockSize),
		m_textureData.data()
	};
	return D3D_OK;
}
HRESULT D3D11::D3D11VolumeTexture::UnlockBox(UINT Level)
{
	assert(Level == 0);

	uint32_t width = std::max(1u, m_desc.Width);
	uint32_t height = std::max(1u, m_desc.Height);
	uint32_t depth = std::max(1u, m_desc.Depth);
	uint32_t blockSize = GetBlockSize(m_desc.Format);

	CD3D11_BOX box(
		0, 0, 0,
		AlignUp(width, blockSize), AlignUp(height, blockSize), AlignUp(depth, blockSize)
	);
	uint32_t rowPitch = (box.right * m_formatSize) / (blockSize);
	uint32_t depthPitch = (box.right * box.bottom * m_formatSize) / (blockSize * blockSize);
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_d3dCtx->GetDeviceContext()->UpdateSubresource(
		m_pID3D11Texture3D.Get(),
		Level,
		&box,
		m_textureData.data(),
		(INT)rowPitch,
		(INT)depthPitch
	);
	return D3D_OK;
}
HRESULT D3D11::D3D11VolumeTexture::AddDirtyBox(CONST D3DBOX* pDirtyBox)
{
	//NOT_IMPLEMENTED_ERROR
	return D3D_OK;
}

/*** D3D11VolumeTexture methods ***/
ID3D11ShaderResourceView* D3D11::D3D11VolumeTexture::GetShaderResourceView()
{
	return m_pID3D11ShaderResourceView.Get();
}
#pragma endregion

#pragma region D3D11VolumeTexture
D3D11::D3D11CubeTexture::D3D11CubeTexture(D3D11Context* ctx, UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) : m_refCount(0), m_d3dCtx(ctx)
{
	assert(Usage == 0);
	if (Levels == 0)
		Levels = 1 + log2(EdgeLength);

	DXGI_FORMAT d3d11Format = D3DFORMAT_to_DXGI_FORMAT(Format);
	m_desc = CD3D11_TEXTURE2D_DESC(
		d3d11Format,
		EdgeLength, EdgeLength, 6,
		Levels,
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_USAGE_DEFAULT,
		0,
		1, 0,
		D3D11_RESOURCE_MISC_TEXTURECUBE
	);

	m_d3dCtx->GetDevice()->CreateTexture2D(&m_desc, NULL, m_pID3D11Texture2D.ReleaseAndGetAddressOf());

	m_formatSize = GetFormatSize(d3d11Format);
	uint32_t blockSize = GetBlockSize(d3d11Format);

	for (int side = 0; side < 6; side++) {
		m_texturesData[side].resize(Levels);
		for (int lod = 0; lod < Levels; lod++) {
			uint32_t width = std::max(1u, EdgeLength / (1 << lod));
			uint32_t height = std::max(1u, EdgeLength / (1 << lod));

			uint32_t textureSize = (AlignUp(width, blockSize) * AlignUp(height, blockSize) * m_formatSize) / (blockSize * blockSize);
			m_texturesData[side][lod].resize(textureSize);
		}
	}

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
		D3D11_SRV_DIMENSION_TEXTURECUBE,
		d3d11Format,
		0, Levels
	);
	m_d3dCtx->GetDevice()->CreateShaderResourceView(m_pID3D11Texture2D.Get(), &srvDesc, m_pID3D11ShaderResourceView.ReleaseAndGetAddressOf());
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11CubeTexture::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11CubeTexture::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11CubeTexture::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11CubeTexture::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11CubeTexture::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11CubeTexture::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11CubeTexture::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11CubeTexture::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11CubeTexture::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11CubeTexture::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11CubeTexture::GetType()
{
	return D3DRTYPE_VOLUMETEXTURE;
}

/*** IDirect3DBaseTexture9 methods ***/
DWORD D3D11::D3D11CubeTexture::SetLOD(DWORD LODNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11CubeTexture::GetLOD()
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11CubeTexture::GetLevelCount()
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11CubeTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	NOT_IMPLEMENTED_ERROR
}
D3DTEXTUREFILTERTYPE D3D11::D3D11CubeTexture::GetAutoGenFilterType()
{
	NOT_IMPLEMENTED
	return D3DTEXF_NONE;
}
void D3D11::D3D11CubeTexture::GenerateMipSubLevels()
{
	NOT_IMPLEMENTED
}

/*** IDirect3DTexture9 methods ***/
HRESULT D3D11::D3D11CubeTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11CubeTexture::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11CubeTexture::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	assert(pRect == nullptr);

	//implement USAGE_DYNAMIC with maps
	uint32_t width = std::max(1u, m_desc.Width / (1 << Level));
	uint32_t blockSize = GetBlockSize(m_desc.Format);

	*pLockedRect = {
		(INT)(width * m_formatSize / blockSize),
		m_texturesData[FaceType][Level].data()
	};
	return D3D_OK;
}
HRESULT D3D11::D3D11CubeTexture::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level)
{
	uint32_t width = std::max(1u, m_desc.Width / (1 << Level));
	uint32_t height = std::max(1u, m_desc.Height / (1 << Level));
	uint32_t blockSize = GetBlockSize(m_desc.Format);

	CD3D11_BOX box(
		0, 0, 0,
		AlignUp(width, blockSize), AlignUp(height, blockSize), 1
	);
	uint32_t rowPitch = (box.right * m_formatSize) / (blockSize);
	uint32_t depthPitch = (box.right * box.bottom * m_formatSize) / (blockSize * blockSize);

	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_d3dCtx->GetDeviceContext()->UpdateSubresource(
		m_pID3D11Texture2D.Get(),
		D3D11CalcSubresource(Level, FaceType, m_desc.MipLevels),
		&box,
		m_texturesData[FaceType][Level].data(),
		(INT)rowPitch,
		(INT)depthPitch
	);
	return D3D_OK;
}
HRESULT D3D11::D3D11CubeTexture::AddDirtyRect(THIS_ D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect)
{
	NOT_IMPLEMENTED_ERROR
}

/*** D3D11CubeTexture methods ***/
ID3D11ShaderResourceView* D3D11::D3D11CubeTexture::GetShaderResourceView()
{
	return m_pID3D11ShaderResourceView.Get();
}
#pragma endregion
