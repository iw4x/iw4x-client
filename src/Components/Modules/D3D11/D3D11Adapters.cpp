#include <STDInclude.hpp>
#include "D3D11Adapters.hpp"
#include "D3D11.hpp"
#include "D3D11Utils.hpp"

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
	uint32_t modeIdx = m_adapters[Adapter].m_modes.size() - 1;
	pMode->Width = m_adapters[Adapter].m_modes[modeIdx].Width;
	pMode->Height = m_adapters[Adapter].m_modes[modeIdx].Height;
	pMode->RefreshRate = static_cast<UINT>(m_adapters[Adapter].m_modes[modeIdx].RefreshRate.Numerator / m_adapters[Adapter].m_modes[modeIdx].RefreshRate.Denominator);
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

	m_pIDXGIFactory->MakeWindowAssociation(pPresentationParameters->hDeviceWindow, DXGI_MWA_NO_ALT_ENTER);

	return D3D_OK;
}
#pragma endregion
