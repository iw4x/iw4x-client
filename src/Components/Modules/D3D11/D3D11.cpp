#include <STDInclude.hpp>
#include "D3D11.hpp"
#include "D3D11Buffers.hpp"
#include "D3D11Shaders.hpp"
#include "D3D11Textures.hpp"
#include "D3D11Utils.hpp"

#pragma region D3D11Query
D3D11::D3D11Query::D3D11Query(D3D11Context* ctx, D3DQUERYTYPE type) : m_refCount(0), m_d3dCtx(ctx), m_type(type)
{
	D3D11_QUERY d3d11Type;
    switch (m_type) {
    case D3DQUERYTYPE_EVENT:				d3d11Type = D3D11_QUERY_EVENT; break;
    case D3DQUERYTYPE_OCCLUSION:			d3d11Type = D3D11_QUERY_OCCLUSION; break;
    case D3DQUERYTYPE_TIMESTAMP:			d3d11Type = D3D11_QUERY_TIMESTAMP; break;
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:	d3d11Type = D3D11_QUERY_TIMESTAMP_DISJOINT; break;
    default:
		NOT_IMPLEMENTED
		return;
    }

	m_desc = CD3D11_QUERY_DESC(d3d11Type);
    ctx->GetDevice()->CreateQuery(&m_desc, &m_query);
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11Query::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11Query::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11Query::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DQuery9 methods ***/
HRESULT D3D11::D3D11Query::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
D3DQUERYTYPE D3D11::D3D11Query::GetType()
{
	return m_type;
}
DWORD D3D11::D3D11Query::GetDataSize()
{
	// cf https://learn.microsoft.com/en-us/windows/win32/direct3d9/queries
	switch (m_type) {
    case D3DQUERYTYPE_EVENT:				return sizeof(BOOL);
    case D3DQUERYTYPE_OCCLUSION:			return sizeof(DWORD);
    case D3DQUERYTYPE_TIMESTAMP:			return sizeof(UINT64);
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:	return sizeof(BOOL);
    case D3DQUERYTYPE_TIMESTAMPFREQ:		return sizeof(UINT64);
    default: return 0;
    }
}
UINT D3D11::D3D11Query::GetDataSizeD3D11()
{
	// cf https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_query
	switch (m_type) {
	case D3DQUERYTYPE_EVENT:				return sizeof(BOOL);
	case D3DQUERYTYPE_OCCLUSION:			return sizeof(UINT64);
	case D3DQUERYTYPE_TIMESTAMP:			return sizeof(UINT64);
	case D3DQUERYTYPE_TIMESTAMPDISJOINT:	return sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT);
	default: return 0;
	}
}
HRESULT D3D11::D3D11Query::Issue(DWORD dwIssueFlags)
{
	if (m_query)
	{
		const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
		if (dwIssueFlags == D3DISSUE_BEGIN)
			m_d3dCtx->GetDeviceContext()->Begin(m_query.Get());
		else
			m_d3dCtx->GetDeviceContext()->End(m_query.Get());
	}
	return D3D_OK;
}
HRESULT D3D11::D3D11Query::GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags)
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	bool flush = dwGetDataFlags & D3DGETDATA_FLUSH;

    if (m_query != nullptr) {
      uint64_t queryData = 0;
      
      HRESULT result = m_d3dCtx->GetDeviceContext()->GetData(m_query.Get(), &queryData, GetDataSizeD3D11(), flush ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH);
      std::memcpy(pData, &queryData, std::min((size_t)GetDataSizeD3D11(), (size_t)dwSize));

      if (FAILED(result)) {
        if (flush)
          return D3DERR_INVALIDCALL;
        else
          return S_FALSE;
      }
      else
        return D3D_OK;
    }

    if (flush)
		m_d3dCtx->GetDeviceContext()->Flush();

    if (m_type == D3DQUERYTYPE_TIMESTAMPFREQ) {
      QueryPerformanceFrequency((LARGE_INTEGER*)pData);
      return D3D_OK;
    }

	NOT_IMPLEMENTED
    return D3DERR_INVALIDCALL;
}
#pragma endregion

#pragma region D3D11Context
D3D11::D3D11Context::D3D11Context(DXGI* dxgi, IDXGIAdapter* adapter, D3DPRESENT_PARAMETERS* pPresentationParameters) : m_dxgi(dxgi), m_refCount(0)
{
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	//creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0
	};

	HRESULT hres = D3D11CreateDevice(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		m_pID3D11Device.ReleaseAndGetAddressOf(), // Returns the Direct3D device created.
		nullptr,
		m_pID3D11DeviceContext.ReleaseAndGetAddressOf() // Returns the device immediate context.
	);

	m_mojoshaderCtx = MOJOSHADER_d3d11CreateContext(GetDevice(), GetDeviceContext(), nullptr, nullptr, nullptr);
	MOJOSHADER_d3d11MapUniformBufferMemory(m_mojoshaderCtx, &m_vsf, &m_vsi, &m_vsb, &m_psf, &m_psi, &m_psb);

	m_swapChain = new D3D11SwapChain(this, pPresentationParameters);
	m_swapChain->AddRef();
}

HRESULT D3D11::D3D11Context::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG D3D11::D3D11Context::AddRef()
{
	return ++m_refCount;
}

ULONG D3D11::D3D11Context::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

HRESULT D3D11::D3D11Context::TestCooperativeLevel()
{
	HRESULT res = m_swapChain->TestCooperativeLevel();
	// can be:
	// D3DERR_DEVICELOST, D3DERR_DEVICENOTRESET, D3DERR_DRIVERINTERNALERROR
	return D3D_OK;
}

UINT D3D11::D3D11Context::GetAvailableTextureMem()
{
	return 4096; // hardcoded 4Go VRAM, need to implement for real
}

HRESULT D3D11::D3D11Context::EvictManagedResources()
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetDirect3D(IDirect3D9** ppD3D9)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::GetDeviceCaps(D3DCAPS9* pCaps)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	NOT_IMPLEMENTED_ERROR
}

void D3D11::D3D11Context::SetCursorPosition(int X, int Y, DWORD Flags)
{
	NOT_IMPLEMENTED;
}

BOOL D3D11::D3D11Context::ShowCursor(BOOL bShow)
{
	NOT_IMPLEMENTED;
	return false;
}

HRESULT D3D11::D3D11Context::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	assert(iSwapChain == 0);
	*pSwapChain = m_swapChain;
	return D3D_OK;
}

UINT D3D11::D3D11Context::GetNumberOfSwapChains()
{
	NOT_IMPLEMENTED;
	return 0;
}

HRESULT D3D11::D3D11Context::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	assert(iSwapChain == 0);
	return m_swapChain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}

HRESULT D3D11::D3D11Context::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetDialogBoxMode(BOOL bEnableDialogs)
{
	NOT_IMPLEMENTED_WARNING
}

void D3D11::D3D11Context::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	NOT_IMPLEMENTED;
}

void D3D11::D3D11Context::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	NOT_IMPLEMENTED;
}

HRESULT D3D11::D3D11Context::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	auto d3d11Tex = new D3D11Texture(this, Width, Height, Levels, Usage, Format, Pool);
	d3d11Tex->AddRef();
	*ppTexture = d3d11Tex;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	auto d3d11Tex = new D3D11VolumeTexture(this, Width, Height, Depth, Levels, Usage, Format, Pool);
	d3d11Tex->AddRef();
	*ppVolumeTexture = d3d11Tex;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	auto d3d11Tex = new D3D11CubeTexture(this, EdgeLength, Levels, Usage, Format, Pool);
	d3d11Tex->AddRef();
	*ppCubeTexture = d3d11Tex;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	auto d3d11VB = new D3D11VertexBuffer(this, Length, Usage, FVF, Pool);
	d3d11VB->AddRef();
	*ppVertexBuffer = d3d11VB;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	auto d3d11IB = new D3D11IndexBuffer(this, Length, Usage, Format, Pool);
	d3d11IB->AddRef();
	*ppIndexBuffer = d3d11IB;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	auto d3d11Texture = new D3D11Texture(this, Width, Height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT);
	d3d11Texture->GetSurfaceLevel(0, ppSurface);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	auto d3d11Texture = new D3D11Texture(this, Width, Height, 1, D3DUSAGE_DEPTHSTENCIL, Format, D3DPOOL_DEFAULT);
	d3d11Texture->GetSurfaceLevel(0, ppSurface);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	m_currentRenderTargets[RenderTargetIndex] = (D3D11Surface*)pRenderTarget;
	m_currentTargetsDirty = true;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	m_currentDepthStencilTarget = (D3D11Surface*)pNewZStencil;
	m_currentTargetsDirty = true;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::BeginScene()
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	m_swapChain->TestBind();

	ID3D11RenderTargetView* rtv[4] = { NULL, NULL, NULL, NULL};
	int numTargets = 4;
	for (int i = 0; i < 4; i++) {
		if (!m_currentRenderTargets[i]) {
			numTargets = i;
			break;
		}

		rtv[i] = m_currentRenderTargets[i]->GetRTV();
	}

	if (m_currentDepthStencilTarget) {
		m_pID3D11DeviceContext->OMSetRenderTargets(numTargets, rtv, m_currentDepthStencilTarget->GetDSV());
	}
	else {
		m_pID3D11DeviceContext->OMSetRenderTargets(numTargets, rtv, NULL);
	}
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::EndScene()
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	assert(pRects == nullptr);

	const std::lock_guard<std::mutex> lock(m_mutex);

	if ((Flags & D3DCLEAR_TARGET) != 0) {
		float color[4] = {
			float((Color >> 24) & 0xFF) / 0xFF,
			float((Color >> 16) & 0xFF) / 0xFF,
			float((Color >> 8) & 0xFF) / 0xFF,
			float((Color) & 0xFF) / 0xFF
		};
		m_pID3D11DeviceContext->ClearRenderTargetView(m_currentRenderTargets[0]->GetRTV(), color);
	}
	if ((Flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)) != 0) {
		m_pID3D11DeviceContext->ClearDepthStencilView(m_currentDepthStencilTarget->GetDSV(), Flags >> 1, Z, Stencil);
	}

	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetViewport(D3DVIEWPORT9* pViewport)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetMaterial(D3DMATERIAL9* pMaterial)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::LightEnable(DWORD Index, BOOL Enable)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetClipPlane(DWORD Index, float* pPlane)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	bool depthStencilDirty;
	bool rasterizerDirty;
	bool blendDirty;
	bool renderTargetsDirty;

#define DEPTH_BREAK m_currentDSStateDirty = true; break;
	switch (State) {
		case D3DRS_ZENABLE:
			m_currentDSDesc.DepthEnable = Value; DEPTH_BREAK
		case D3DRS_ZWRITEENABLE:
			m_currentDSDesc.DepthWriteMask = (Value == true) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO; DEPTH_BREAK
		case D3DRS_ZFUNC:
			m_currentDSDesc.DepthFunc = D3DCMPFUNCToD3D11_COMPARISON_FUNC((D3DCMPFUNC)Value); DEPTH_BREAK
		case D3DRS_STENCILENABLE:
			m_currentDSDesc.StencilEnable = Value; DEPTH_BREAK
		case D3DRS_STENCILFAIL:
			m_currentDSDesc.FrontFace.StencilFailOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_STENCILZFAIL:
			m_currentDSDesc.FrontFace.StencilDepthFailOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_STENCILPASS:
			m_currentDSDesc.FrontFace.StencilPassOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_STENCILFUNC:
			m_currentDSDesc.FrontFace.StencilFunc = D3DCMPFUNCToD3D11_COMPARISON_FUNC((D3DCMPFUNC)Value); DEPTH_BREAK
		case D3DRS_STENCILREF:
			m_currentStencilRef = Value; DEPTH_BREAK
		case D3DRS_STENCILMASK:
			m_currentDSDesc.StencilReadMask = (UINT8)(Value & 0x000000FF); DEPTH_BREAK
		case D3DRS_STENCILWRITEMASK:
			m_currentDSDesc.StencilWriteMask = (UINT8)(Value & 0x000000FF); DEPTH_BREAK
		case D3DRS_TWOSIDEDSTENCILMODE:
			assert(Value); // only two sided stencil implemented
			break;
		case D3DRS_CCW_STENCILFAIL:
			m_currentDSDesc.BackFace.StencilFailOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_CCW_STENCILZFAIL:
			m_currentDSDesc.BackFace.StencilDepthFailOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_CCW_STENCILPASS:
			m_currentDSDesc.BackFace.StencilPassOp = D3DSTENCILOPToD3D11_STENCIL_OP((D3DSTENCILOP)Value); DEPTH_BREAK
		case D3DRS_CCW_STENCILFUNC:
			m_currentDSDesc.BackFace.StencilFunc = D3DCMPFUNCToD3D11_COMPARISON_FUNC((D3DCMPFUNC)Value); DEPTH_BREAK
			break;
		case D3DRS_FILLMODE:
		case D3DRS_CULLMODE:
		case D3DRS_SCISSORTESTENABLE:
		case D3DRS_SLOPESCALEDEPTHBIAS:
		case D3DRS_DEPTHBIAS:
			rasterizerDirty = true;
			break;
		case D3DRS_SRCBLEND:
		case D3DRS_DESTBLEND:
		case D3DRS_ALPHABLENDENABLE:
		case D3DRS_COLORWRITEENABLE:
		case D3DRS_BLENDOP:
		case D3DRS_COLORWRITEENABLE1:
		case D3DRS_COLORWRITEENABLE2:
		case D3DRS_COLORWRITEENABLE3:
		case D3DRS_BLENDFACTOR:
		case D3DRS_SEPARATEALPHABLENDENABLE:
		case D3DRS_SRCBLENDALPHA:
		case D3DRS_DESTBLENDALPHA:
		case D3DRS_BLENDOPALPHA:
			blendDirty = true;
			break;
		case D3DRS_SRGBWRITEENABLE:
			renderTargetsDirty = true;
			break;


		case D3DRS_ALPHAREF:
		case D3DRS_ALPHATESTENABLE:
		case D3DRS_ALPHAFUNC:
			OutputDebugStringA("[WARNING] D3DRS_ALPHAXXX not implemented!\n");
			break;
		default:
			NOT_IMPLEMENTED;
			break;
	}
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::BeginStateBlock()
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	ID3D11ShaderResourceView* srv = nullptr;
	if (pTexture) {
		switch (pTexture->GetType()) {
		case D3DRTYPE_TEXTURE: srv = ((D3D11Texture*)pTexture)->GetShaderResourceView(); break;
		case D3DRTYPE_VOLUMETEXTURE:srv = ((D3D11VolumeTexture*)pTexture)->GetShaderResourceView(); break;
		case D3DRTYPE_CUBETEXTURE:srv = ((D3D11CubeTexture*)pTexture)->GetShaderResourceView(); break;
		}
	}

	const std::lock_guard<std::mutex> lock(m_mutex);
	m_pID3D11DeviceContext->PSSetShaderResources(
		Stage,
		1,
		&srv
	);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	//NOT_IMPLEMENTED_WARNING
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::ValidateDevice(DWORD* pNumPasses)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetCurrentTexturePalette(UINT PaletteNumber)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetCurrentTexturePalette(UINT* PaletteNumber)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetScissorRect(CONST RECT* pRect)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetScissorRect(RECT* pRect)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	NOT_IMPLEMENTED_WARNING
}

BOOL D3D11::D3D11Context::GetSoftwareVertexProcessing()
{
	NOT_IMPLEMENTED;
	return false;
}

HRESULT D3D11::D3D11Context::SetNPatchMode(float nSegments)
{
	NOT_IMPLEMENTED_WARNING
}

float D3D11::D3D11Context::GetNPatchMode()
{
	NOT_IMPLEMENTED;
	return 0;
}

HRESULT D3D11::D3D11Context::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	UINT indexCount = 0;
	switch (PrimitiveType) {
	case D3DPT_POINTLIST:		topology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;		indexCount = primCount; break;
	case D3DPT_LINELIST:		topology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;		indexCount = primCount * 2; break;
	case D3DPT_LINESTRIP:		topology = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;		indexCount = primCount + 1; break;
	case D3DPT_TRIANGLELIST:	topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	indexCount = primCount * 3; break;
	case D3DPT_TRIANGLESTRIP:	topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;	indexCount = primCount + 2; break;
	default: NOT_IMPLEMENTED;
	}
	ApplyInternalState();
	m_pID3D11DeviceContext->IASetPrimitiveTopology(topology);
	m_pID3D11DeviceContext->DrawIndexed(indexCount, startIndex, BaseVertexIndex);

	return D3D_OK;
}

HRESULT D3D11::D3D11Context::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	D3D11InputLayout* d3d11InputLayout = new D3D11InputLayout(this, pVertexElements);
	size_t hash = d3d11InputLayout->GetHash();
	if (m_inputLayoutCache.contains(hash)) {
		delete d3d11InputLayout; // urg
		d3d11InputLayout = m_inputLayoutCache[hash];
	}
	d3d11InputLayout->AddRef();
	*ppDecl = d3d11InputLayout;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	m_currentInputLayout = (D3D11InputLayout*)pDecl;
	m_currentInputLayoutDirty = true;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	*ppDecl = m_currentInputLayout;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetFVF(DWORD FVF)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetFVF(DWORD* pFVF)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	auto d3d11Shader = new D3D11VertexShader(this, pFunction);
	d3d11Shader->AddRef();
	*ppShader = d3d11Shader;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	m_currentVS = (D3D11VertexShader*)pShader;
	m_currentVSDirty = true;
	m_currentInputLayoutDirty = true;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	*ppShader = m_currentVS;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	std::memcpy(m_vsf + StartRegister * 4, pConstantData, sizeof(float) * 4 * Vector4fCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	std::memcpy(m_vsi + StartRegister * 4, pConstantData, sizeof(int) * 4 * Vector4iCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount)
{
	std::memcpy(m_vsb + StartRegister, pConstantData, sizeof(unsigned char) * BoolCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	D3D11VertexBuffer* buffer = (D3D11VertexBuffer*)pStreamData;
	if (buffer) {
		ID3D11Buffer* vbs[] = { buffer->GetBuffer() };
		UINT strides[] = { Stride };
		UINT offsets[] = { OffsetInBytes };
		m_pID3D11DeviceContext->IASetVertexBuffers(
			StreamNumber,
			1,
			vbs,
			strides,
			offsets
		);
	}
	else {
		ID3D11Buffer* vbs[] = { NULL };
		UINT strides[] = { NULL };
		UINT offsets[] = { NULL };
		m_pID3D11DeviceContext->IASetVertexBuffers(
			StreamNumber,
			1,
			vbs,
			strides,
			offsets
		);
	}
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	if (pIndexData) {
		D3D11IndexBuffer* buffer = (D3D11IndexBuffer*)pIndexData;
		m_pID3D11DeviceContext->IASetIndexBuffer(buffer->GetBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}
	else {
		m_pID3D11DeviceContext->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	}

	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	auto d3d11Shader = new D3D11PixelShader(this, pFunction);
	d3d11Shader->AddRef();
	*ppShader = d3d11Shader;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	m_currentPS = (D3D11PixelShader*)pShader;
	m_currentPSDirty = true;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	*ppShader = m_currentPS;
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	std::memcpy(m_psf + StartRegister * 4, pConstantData, sizeof(float) * 4 * Vector4fCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	std::memcpy(m_psi + StartRegister * 4, pConstantData, sizeof(int) * 4 * Vector4iCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount)
{
	std::memcpy(m_psb + StartRegister, pConstantData, sizeof(unsigned char) * BoolCount);
	return D3D_OK;
}

HRESULT D3D11::D3D11Context::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	NOT_IMPLEMENTED_ERROR
}

HRESULT D3D11::D3D11Context::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::DeletePatch(UINT Handle)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT D3D11::D3D11Context::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	auto d3d11Query = new D3D11Query(this, Type);
	d3d11Query->AddRef();
	*ppQuery = d3d11Query;
	return D3D_OK;
}

void D3D11::D3D11Context::ApplyInternalState()
{
	if (m_currentPSDirty) {
		MOJOSHADER_d3d11BindShaders(m_mojoshaderCtx, nullptr, m_currentPS->GetMojoshader());
		m_currentPSDirty = true;
	}

	if (m_currentVSDirty) {
		MOJOSHADER_d3d11BindShaders(m_mojoshaderCtx, m_currentVS->GetMojoshader(), nullptr);
		auto inputLayout = m_currentVS->Compile(m_currentInputLayout);
		m_pID3D11DeviceContext->IASetInputLayout(inputLayout);
		m_currentVSDirty = true;
	}

	MOJOSHADER_d3d11UnmapUniformBufferMemory(m_mojoshaderCtx);
	MOJOSHADER_d3d11ProgramReady(m_mojoshaderCtx, m_currentInputLayout->GetHash());

	if (m_currentTargetsDirty) {
		
		//m_pID3D11DeviceContext->OMSetRenderTargets(1, NULL, NULL);
	}

	if (m_currentDSStateDirty) {
		size_t hash = HashState(&m_currentDSDesc);

		if (!m_depthStencilStateCache.contains(hash)) {
			m_pID3D11Device->CreateDepthStencilState(
				&m_currentDSDesc,
				m_depthStencilStateCache[hash].ReleaseAndGetAddressOf()
			);
		}
		m_pID3D11DeviceContext->OMSetDepthStencilState(m_depthStencilStateCache[hash].Get(), m_currentStencilRef);
		m_currentDSStateDirty = false;
	}
}

#pragma endregion

#pragma region D3D11Adapter

HMONITOR D3D11::D3D11Adapter::GetMonitor()
{
	DXGI_OUTPUT_DESC outputDesc;
	m_output->GetDesc(&outputDesc);
	return outputDesc.Monitor;
}

HRESULT D3D11::D3D11Adapter::GetIdentifier(D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_OUTPUT_DESC outputDesc;
	m_adapter->GetDesc(&adapterDesc);
	m_output->GetDesc(&outputDesc);

	std::strcpy(pIdentifier->Driver, "d3d9.dll");
	pIdentifier->DriverVersion.QuadPart = 1;

	WideCharToMultiByte(CP_UTF8, 0,
		adapterDesc.Description, -1,
		pIdentifier->Description, MAX_DEVICE_IDENTIFIER_STRING,
		nullptr, nullptr);

	WideCharToMultiByte(CP_UTF8, 0,
		outputDesc.DeviceName, -1,
		pIdentifier->DeviceName, 32,
		nullptr, nullptr);

	pIdentifier->VendorId = adapterDesc.VendorId;
	pIdentifier->DeviceId = adapterDesc.DeviceId;
	pIdentifier->SubSysId = adapterDesc.SubSysId;
	pIdentifier->Revision = adapterDesc.Revision;

	pIdentifier->DeviceIdentifier = {};
	std::memcpy(&pIdentifier->DeviceIdentifier.Data1, &adapterDesc.AdapterLuid, sizeof(LUID));
	std::memcpy(&pIdentifier->DeviceIdentifier.Data4[0], &m_index, sizeof(UINT)); // pseudo identifier

	pIdentifier->WHQLLevel = 0;

	return S_OK;
}
#pragma endregion

#pragma region DXGI

D3D11::DXGI::DXGI() : m_refCount(0)
{
	CreateDXGIFactory(IID_PPV_ARGS(m_pIDXGIFactory.ReleaseAndGetAddressOf()));

	UINT globalIdx = 0;
	UINT adapterId = 0;
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	while (SUCCEEDED(m_pIDXGIFactory->EnumAdapters(adapterId, adapter.ReleaseAndGetAddressOf()))) {
		UINT outputId = 0;
		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		while (SUCCEEDED(adapter->EnumOutputs(outputId, &output))) {
			m_adapters.emplace_back(globalIdx++, adapter, std::move(output));
			++outputId;
		}
		++adapterId;
	}
}

HRESULT WINAPI D3D11::DXGI::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	// maybe check for IID_IDirect3D9Ex ???
	return E_NOINTERFACE;
}

ULONG WINAPI D3D11::DXGI::AddRef()
{
	return ++m_refCount;
}

ULONG WINAPI D3D11::DXGI::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

HRESULT WINAPI D3D11::DXGI::RegisterSoftwareDevice(void* pInitializeFunction)
{
	NOT_IMPLEMENTED_WARNING
}

UINT WINAPI D3D11::DXGI::GetAdapterCount()
{
	return static_cast<UINT>(m_adapters.size());
}

HRESULT WINAPI D3D11::DXGI::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return m_adapters[Adapter].GetIdentifier(pIdentifier);
}

UINT WINAPI D3D11::DXGI::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	UINT count = 0;
	m_adapters[Adapter].m_output->GetDisplayModeList(D3DFORMAT_to_DXGI_FORMAT(Format), 0, &count, nullptr);
	m_adapters[Adapter].m_modes.resize(count);
	m_adapters[Adapter].m_output->GetDisplayModeList(D3DFORMAT_to_DXGI_FORMAT(Format), 0, &count, m_adapters[Adapter].m_modes.data());
	return count;
}

HRESULT WINAPI D3D11::DXGI::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	*pMode = {};
	pMode->Width = m_adapters[Adapter].m_modes[Mode].Width;
	pMode->Height = m_adapters[Adapter].m_modes[Mode].Height;
	pMode->RefreshRate = static_cast<UINT>(m_adapters[Adapter].m_modes[Mode].RefreshRate.Numerator / m_adapters[Adapter].m_modes[Mode].RefreshRate.Denominator);
	pMode->Format = Format;
	return D3D_OK;
}

HRESULT WINAPI D3D11::DXGI::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	*pMode = {};
	pMode->Width = m_adapters[Adapter].m_modes[0].Width;
	pMode->Height = m_adapters[Adapter].m_modes[0].Height;
	pMode->RefreshRate = static_cast<UINT>(m_adapters[Adapter].m_modes[0].RefreshRate.Numerator / m_adapters[Adapter].m_modes[0].RefreshRate.Denominator);
	pMode->Format = D3DFMT_X8R8G8B8;
	return D3D_OK;
}

HRESULT WINAPI D3D11::DXGI::CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT WINAPI D3D11::DXGI::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	if (D3DFORMAT_to_DXGI_FORMAT(CheckFormat) == DXGI_FORMAT_UNKNOWN)
		return D3DERR_NOTAVAILABLE;

	NOT_IMPLEMENTED_WARNING
}

HRESULT WINAPI D3D11::DXGI::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT WINAPI D3D11::DXGI::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	if (D3DFORMAT_to_DXGI_FORMAT(RenderTargetFormat) == DXGI_FORMAT_UNKNOWN)
		return D3DERR_NOTAVAILABLE;

	NOT_IMPLEMENTED_WARNING
}

HRESULT WINAPI D3D11::DXGI::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	NOT_IMPLEMENTED_WARNING
}

HRESULT WINAPI D3D11::DXGI::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
	// TODO: cleanup all that crap and check what features are actually used by the game
	*pCaps = {};
	pCaps->DeviceType = D3DDEVTYPE_HAL;
	pCaps->AdapterOrdinal = Adapter;

	pCaps->Caps = 0;
	pCaps->Caps2 = D3DCAPS2_DYNAMICTEXTURES;
	pCaps->Caps3 = D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD |
		D3DCAPS3_COPY_TO_VIDMEM |
		D3DCAPS3_COPY_TO_SYSTEMMEM;
	pCaps->PresentationIntervals = D3DPRESENT_INTERVAL_IMMEDIATE |
		D3DPRESENT_INTERVAL_ONE;
	pCaps->CursorCaps = D3DCURSORCAPS_COLOR |
		D3DCURSORCAPS_LOWRES;
	pCaps->DevCaps = D3DDEVCAPS_EXECUTESYSTEMMEMORY |
		D3DDEVCAPS_EXECUTEVIDEOMEMORY |
		D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
		D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
		D3DDEVCAPS_TEXTURESYSTEMMEMORY |
		D3DDEVCAPS_TEXTUREVIDEOMEMORY |
		D3DDEVCAPS_DRAWPRIMTLVERTEX |
		D3DDEVCAPS_CANRENDERAFTERFLIP |
		D3DDEVCAPS_DRAWPRIMITIVES2 |
		D3DDEVCAPS_DRAWPRIMITIVES2EX |
		D3DDEVCAPS_HWTRANSFORMANDLIGHT |
		D3DDEVCAPS_HWRASTERIZATION |
		D3DDEVCAPS_PUREDEVICE;
	pCaps->PrimitiveMiscCaps = 0x002ecff2;
	D3DPMISCCAPS_MASKZ |
		D3DPMISCCAPS_CULLNONE |
		D3DPMISCCAPS_CULLCCW |
		D3DPMISCCAPS_CULLCW |
		D3DPMISCCAPS_COLORWRITEENABLE |
		D3DPMISCCAPS_CLIPPLANESCALEDPOINTS |
		D3DPMISCCAPS_CLIPTLVERTS |
		D3DPMISCCAPS_TSSARGTEMP |
		D3DPMISCCAPS_BLENDOP |
		D3DPMISCCAPS_INDEPENDENTWRITEMASKS |
		D3DPMISCCAPS_PERSTAGECONSTANT |
		D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING |
		D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS |
		D3DPMISCCAPS_SEPARATEALPHABLEND |
		D3DPMISCCAPS_POSTBLENDSRGBCONVERT;
	pCaps->RasterCaps =
		D3DPRASTERCAPS_DITHER |
		D3DPRASTERCAPS_ZTEST |
		D3DPRASTERCAPS_FOGVERTEX |
		D3DPRASTERCAPS_FOGTABLE |
		D3DPRASTERCAPS_MIPMAPLODBIAS |
		D3DPRASTERCAPS_FOGRANGE |
		D3DPRASTERCAPS_ANISOTROPY |
		D3DPRASTERCAPS_WFOG |
		D3DPRASTERCAPS_ZFOG |
		D3DPRASTERCAPS_SCISSORTEST |
		D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS |
		D3DPRASTERCAPS_DEPTHBIAS;

	pCaps->ZCmpCaps = 0x000000FFL;
	pCaps->SrcBlendCaps = 0x00003FFFL;
	pCaps->DestBlendCaps = 0x00003FFFL;
	pCaps->AlphaCmpCaps = 0x000000FFL;
	pCaps->TextureCaps = D3DPTEXTURECAPS_PERSPECTIVE |
		D3DPTEXTURECAPS_ALPHA |
		D3DPTEXTURECAPS_ALPHAPALETTE |
		D3DPTEXTURECAPS_PROJECTED |
		D3DPTEXTURECAPS_CUBEMAP |
		D3DPTEXTURECAPS_VOLUMEMAP |
		D3DPTEXTURECAPS_MIPMAP |
		D3DPTEXTURECAPS_MIPVOLUMEMAP |
		D3DPTEXTURECAPS_MIPCUBEMAP;
	pCaps->TextureFilterCaps = 0x07030700L;
	pCaps->CubeTextureFilterCaps = 0x07030700L;
	pCaps->VolumeTextureFilterCaps = 0x03030300L;
	pCaps->TextureAddressCaps = 0x0000003FL;
	pCaps->VolumeTextureAddressCaps = 0x0000003FL;

	pCaps->MaxTextureWidth = 0x00008000L;
	pCaps->MaxTextureHeight = 0x00008000L;
	pCaps->MaxVolumeExtent = 0x00004000L;

	pCaps->MaxTextureRepeat = 0x00008000L;
	pCaps->MaxTextureAspectRatio = 0x00008000L;
	pCaps->MaxAnisotropy = 0x00000010L;
	pCaps->MaxVertexW = 1.00000000e+10f;
	pCaps->StencilCaps = 0x000001FFL;
	pCaps->TextureOpCaps = 0x03feffffL;
	pCaps->MaxTextureBlendStages = 9;
	pCaps->MaxSimultaneousTextures = 8;
	pCaps->MaxUserClipPlanes = 8;

	pCaps->DevCaps2 = D3DDEVCAPS2_STREAMOFFSET;
	pCaps->DeclTypes = D3DDTCAPS_UBYTE4 |
		D3DDTCAPS_UBYTE4N |
		D3DDTCAPS_SHORT2N |
		D3DDTCAPS_SHORT4N |
		D3DDTCAPS_FLOAT16_2 |
		D3DDTCAPS_FLOAT16_4;

	pCaps->MaxVertexIndex = 0x00ffffff;
	pCaps->MaxStreams = 0x00000010;
	pCaps->MaxStreamStride = 0x00000400;

	pCaps->VertexShaderVersion = 0xfffe0300L;
	pCaps->PixelShaderVersion = 0xffff0300L;

	pCaps->NumSimultaneousRTs = 4;
	pCaps->StretchRectFilterCaps = 0x03000300L;

	return S_OK;
}

HMONITOR WINAPI D3D11::DXGI::GetAdapterMonitor(UINT Adapter)
{
	return m_adapters[Adapter].GetMonitor();
}

HRESULT WINAPI D3D11::DXGI::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	D3D11Context* D3D11Context = new D3D11::D3D11Context(this, m_adapters[Adapter].m_adapter.Get(), pPresentationParameters);
	D3D11Context->AddRef();
	*ppReturnedDeviceInterface = D3D11Context;

	// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	m_pIDXGIFactory->MakeWindowAssociation(pPresentationParameters->hDeviceWindow, DXGI_MWA_NO_ALT_ENTER);

	return D3D_OK;
}
#pragma endregion

namespace Components {
	IDirect3D9* CALLBACK D3D11::Direct3DCreate9Stub(UINT sdk)
	{
		::D3D11::DXGI* DXGIFactory = new ::D3D11::DXGI(); // we use IUnknown AddRef/Release to manage the lifetime of the object
		DXGIFactory->AddRef();
		return DXGIFactory;
	}

	D3D11::D3D11()
	{
		Utils::Hook::Set(0x6D74D0, Direct3DCreate9Stub);
	}
}
