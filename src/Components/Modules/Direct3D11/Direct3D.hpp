#pragma once

#include "Object.hpp"
#include "Resources.hpp"

#include <dxgi1_5.h>

#include <vector>

namespace Components
{
  namespace d3d11
  {
    // The IDirect3D9 over DXGI.
    //
    // A D3D9 adapter is a display, that is, a DXGI output, so we number our
    // adapters by output in DXGI's order (adapter by adapter, the primary
    // display first). If no adapter has an output (which happens on some
    // virtual setups), we still offer the first adapter so that the game
    // can create a device and render into a window.
    //
    // D3D9 also answers questions about formats and capabilities before
    // there is a device. We answer the format questions by asking D3D11
    // (through a device we create for just that) and report the
    // capabilities of a D3D9 driver for the kind of hardware that runs
    // D3D11, minus what we don't emulate (user clip planes, say).
    //
    class direct3d: public unknown<IDirect3D9>
    {
    public:
      HRESULT
      create ();

      STDMETHOD (RegisterSoftwareDevice) (void*) override;
      STDMETHOD_ (UINT, GetAdapterCount) () override;
      STDMETHOD (GetAdapterIdentifier) (UINT, DWORD, D3DADAPTER_IDENTIFIER9*) override;
      STDMETHOD_ (UINT, GetAdapterModeCount) (UINT, D3DFORMAT) override;
      STDMETHOD (EnumAdapterModes) (UINT, D3DFORMAT, UINT, D3DDISPLAYMODE*) override;
      STDMETHOD (GetAdapterDisplayMode) (UINT, D3DDISPLAYMODE*) override;
      STDMETHOD (CheckDeviceType) (UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL) override;
      STDMETHOD (CheckDeviceFormat) (UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) override;
      STDMETHOD (CheckDeviceMultiSampleType) (UINT, D3DDEVTYPE, D3DFORMAT, BOOL, D3DMULTISAMPLE_TYPE, DWORD*) override;
      STDMETHOD (CheckDepthStencilMatch) (UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT) override;
      STDMETHOD (CheckDeviceFormatConversion) (UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT) override;
      STDMETHOD (GetDeviceCaps) (UINT, D3DDEVTYPE, D3DCAPS9*) override;
      STDMETHOD_ (HMONITOR, GetAdapterMonitor) (UINT) override;
      STDMETHOD (CreateDevice) (UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**) override;

      // Return the multisample count of a D3D9 multisample type and
      // quality. Return 0 if the adapter doesn't support it for the format.
      //
      UINT
      sample_count (UINT adapter, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD quality);

      IDXGIOutput*
      output (UINT adapter) const noexcept
      {
        return adapter < adapters_.size () ? adapters_[adapter].output.Get () : nullptr;
      }

    private:
      struct adapter_info
      {
        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<IDXGIOutput>   output;
        DXGI_ADAPTER_DESC1    desc;
        ComPtr<ID3D11Device>  probe; // For the format questions, lazily.
        std::vector<D3DDISPLAYMODE> modes;
      };

      ID3D11Device*
      probe (UINT adapter);

      const std::vector<D3DDISPLAYMODE>&
      modes (UINT adapter);

      ComPtr<IDXGIFactory1>     factory_;
      std::vector<adapter_info> adapters_;
    };
  }
}
