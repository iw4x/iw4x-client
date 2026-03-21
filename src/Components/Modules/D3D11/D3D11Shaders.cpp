#include <STDInclude.hpp>
#include "D3D11Shaders.hpp"
#include "D3D11.hpp"
#include "D3D11Utils.hpp"

#pragma region D3D11InputLayout
D3D11::D3D11InputLayout::D3D11InputLayout(D3D11Context* ctx, CONST D3DVERTEXELEMENT9* pVertexElements) : m_refCount(0), m_d3dCtx(ctx)
{
	const D3DVERTEXELEMENT9* pCurrentVtx = pVertexElements;
	while (pCurrentVtx->Stream != 255) {
		D3D11_INPUT_ELEMENT_DESC desc;

		if (pCurrentVtx->Method != 0)
			NOT_IMPLEMENTED;

		m_inputElements.emplace_back(
			D3D9UsageToSemantic(pCurrentVtx->Usage), pCurrentVtx->UsageIndex,
			D3D9DeclTypeToDXGI_Format(pCurrentVtx->Type),
			pCurrentVtx->Stream,
			pCurrentVtx->Offset,
			D3D11_INPUT_PER_VERTEX_DATA, 0
		);
		pCurrentVtx++;
	}
	// we do not create the input layout yet because we need a vertex shader to do that
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11InputLayout::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11InputLayout::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11InputLayout::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DVertexDeclaration9 methods ***/
HRESULT D3D11::D3D11InputLayout::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11InputLayout::GetDeclaration(D3DVERTEXELEMENT9* pElement, UINT* pNumElements)
{
	NOT_IMPLEMENTED_ERROR
}
ID3D11InputLayout* D3D11::D3D11InputLayout::GetLayout(D3D11VertexShader* vertexShader)
{
	return nullptr;
}
size_t D3D11::D3D11InputLayout::GetHash() const
{
	return HashState(m_inputElements.data(), m_inputElements.size());
}
#pragma endregion

#include <d3dcompiler.h>
#include <mojoshader.h>

#pragma region D3D11VertexBuffer
D3D11::D3D11VertexShader::D3D11VertexShader(D3D11Context* ctx, const DWORD* pFunction) : m_refCount(0), m_d3dCtx(ctx)
{
	m_mojoshader = MOJOSHADER_d3d11CompileShader(
		ctx->GetMojoshaderCtx(),
		nullptr, (uint8_t*)pFunction, 0,
		nullptr, 0,
		nullptr, 0);
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11VertexShader::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11VertexShader::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11VertexShader::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11VertexShader::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11VertexShader::GetFunction(void* pData, UINT* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
ID3D11InputLayout* D3D11::D3D11VertexShader::Compile(D3D11InputLayout* layout)
{
	size_t hash = layout->GetHash();
	if (m_layoutCache.contains(hash)) {
		return m_layoutCache[hash].Get();
	}

	void* bytecode;
	int32_t bytecodeLength;
	MOJOSHADER_d3d11CompileVertexShader(
		m_d3dCtx->GetMojoshaderCtx(),
		(unsigned long long)hash,
		layout->GetElements(),
		layout->GetNumElements(),
		&bytecode,
		&bytecodeLength
	);
	m_d3dCtx->GetDevice()->CreateInputLayout(
		layout->GetElements(),
		layout->GetNumElements(),
		bytecode,
		bytecodeLength,
		m_layoutCache[hash].ReleaseAndGetAddressOf()
	);
	return m_layoutCache[hash].Get();
}
#pragma endregion

#pragma region D3D11VertexBuffer
D3D11::D3D11PixelShader::D3D11PixelShader(D3D11Context* ctx, const DWORD* pFunction) : m_refCount(0), m_d3dCtx(ctx)
{
	m_mojoshader = MOJOSHADER_d3d11CompileShader(
		ctx->GetMojoshaderCtx(),
		nullptr, (uint8_t*)pFunction, 0,
		nullptr, 0,
		nullptr, 0);
}

/*** IUnknown methods ***/
HRESULT D3D11::D3D11PixelShader::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}
ULONG D3D11::D3D11PixelShader::AddRef()
{
	return ++m_refCount;
}
ULONG D3D11::D3D11PixelShader::Release()
{
	ULONG ret = --m_refCount;
	if (ret == 0) delete this;
	return ret;
}

/*** IDirect3DResource9 methods ***/
HRESULT D3D11::D3D11PixelShader::GetDevice(IDirect3DDevice9** ppDevice)
{
	*ppDevice = m_d3dCtx;
	return D3D_OK;
}
HRESULT D3D11::D3D11PixelShader::GetFunction(void* pData, UINT* pSizeOfData)
{
	NOT_IMPLEMENTED_ERROR
}
#pragma endregion
