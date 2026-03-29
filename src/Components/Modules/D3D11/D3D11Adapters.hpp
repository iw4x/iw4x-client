#pragma once

namespace D3D11 {
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
