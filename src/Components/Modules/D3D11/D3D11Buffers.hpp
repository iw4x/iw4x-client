#pragma once

namespace D3D11
{
	class D3D11Context;

	class D3D11VertexBuffer : public IDirect3DVertexBuffer9
	{
	public:
		D3D11VertexBuffer(D3D11Context* d3d11Context, UINT Length, DWORD Usage);
		virtual ~D3D11VertexBuffer() = default;

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

		/*** IDirect3DVertexBuffer9 methods ***/
		STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override;
		STDMETHOD(Unlock)(THIS) override;
		STDMETHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC* pDesc) override;

		/*** D3D11VertexBuffer methods ***/
		ID3D11Buffer* GetBuffer() const { return m_pID3D11Buffer.Get(); }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		CD3D11_BUFFER_DESC m_desc;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pID3D11Buffer;
		std::vector<uint8_t> m_data;
	};

	class D3D11IndexBuffer : public IDirect3DIndexBuffer9
	{
	public:
		D3D11IndexBuffer(D3D11Context* d3d11Context, UINT Length, DWORD Usage, D3DFORMAT Format);
		virtual ~D3D11IndexBuffer() = default;

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

		/*** IDirect3DIndexBuffer9 methods ***/
		STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override;
		STDMETHOD(Unlock)(THIS) override;
		STDMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC* pDesc) override;

		/*** D3D11VertexBuffer methods ***/
		ID3D11Buffer* GetBuffer() const { return m_pID3D11Buffer.Get(); }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		CD3D11_BUFFER_DESC m_desc;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pID3D11Buffer;
		std::vector<uint8_t> m_data;
	};
}
