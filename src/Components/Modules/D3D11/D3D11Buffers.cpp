#include <STDInclude.hpp>
#include "D3D11Buffers.hpp"
#include "D3D11.hpp"
#include "D3D11Utils.hpp"

// TODO implement Usage for VB / IB:
// D3DUSAGE_DONOTCLIP
// D3DUSAGE_DYNAMIC
// D3DUSAGE_NPATCHES
// D3DUSAGE_POINTS
// D3DUSAGE_RTPATCHES
// D3DUSAGE_SOFTWAREPROCESSING
// D3DUSAGE_WRITEONLY

#pragma region D3D11VertexBuffer
D3D11::D3D11VertexBuffer::D3D11VertexBuffer(D3D11Context* ctx, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool) : m_refCount(0), m_d3dCtx(ctx), m_usage(Usage)
{
	CD3D11_BUFFER_DESC desc(
		Length,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	m_d3dCtx->GetDevice()->CreateBuffer(&desc, NULL, m_pID3D11Buffer.ReleaseAndGetAddressOf());
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11VertexBuffer::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11VertexBuffer::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11VertexBuffer::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11VertexBuffer::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11VertexBuffer::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VertexBuffer::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11VertexBuffer::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VertexBuffer::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11VertexBuffer::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11VertexBuffer::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11VertexBuffer::GetType()
{
	return D3DRTYPE_VERTEXBUFFER;
}

/*** IDirect3DVertexBuffer9 methods ***/
HRESULT D3D11::D3D11VertexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT res = m_d3dCtx->GetDeviceContext()->Map(
		m_pID3D11Buffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped
	);
	*ppbData = ((uint8_t*)mapped.pData) + OffsetToLock;
	return D3D_OK;
}
HRESULT D3D11::D3D11VertexBuffer::Unlock()
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_d3dCtx->GetDeviceContext()->Unmap(
		m_pID3D11Buffer.Get(),
		0
	);
	return D3D_OK;
}
HRESULT D3D11::D3D11VertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC* pDesc)
{
	NOT_IMPLEMENTED_ERROR
}
#pragma endregion

#pragma region D3D11IndexBuffer
D3D11::D3D11IndexBuffer::D3D11IndexBuffer(D3D11Context* ctx, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) : m_refCount(0), m_d3dCtx(ctx)
{
	CD3D11_BUFFER_DESC desc(
		Length,
		D3D11_BIND_INDEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	m_d3dCtx->GetDevice()->CreateBuffer(&desc, NULL, m_pID3D11Buffer.ReleaseAndGetAddressOf());
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11IndexBuffer::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11IndexBuffer::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11IndexBuffer::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11IndexBuffer::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11IndexBuffer::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11IndexBuffer::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
HRESULT D3D11::D3D11IndexBuffer::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11IndexBuffer::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_ERROR
}
DWORD D3D11::D3D11IndexBuffer::GetPriority()
{
	NOT_IMPLEMENTED_ERROR
}
void D3D11::D3D11IndexBuffer::PreLoad()
{
	NOT_IMPLEMENTED
}
D3DRESOURCETYPE D3D11::D3D11IndexBuffer::GetType()
{
	return D3DRTYPE_INDEXBUFFER;
}

/*** IDirect3DIndexBuffer9 methods ***/
HRESULT D3D11::D3D11IndexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT res = m_d3dCtx->GetDeviceContext()->Map(
		m_pID3D11Buffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped
	);
	*ppbData = ((uint8_t*)mapped.pData) + OffsetToLock;
	return D3D_OK;
}
HRESULT D3D11::D3D11IndexBuffer::Unlock()
{
	const std::lock_guard<std::mutex> lock(m_d3dCtx->m_mutex);
	m_d3dCtx->GetDeviceContext()->Unmap(
		m_pID3D11Buffer.Get(),
		0
	);
	return D3D_OK;
}
HRESULT D3D11::D3D11IndexBuffer::GetDesc(D3DINDEXBUFFER_DESC* pDesc)
{
	NOT_IMPLEMENTED_ERROR
}
#pragma endregion
