#pragma once

namespace D3D11
{
	class D3D11Context;
	class D3D11VertexShader;

	class D3D11InputLayout : public IDirect3DVertexDeclaration9
	{
	public:
		D3D11InputLayout(D3D11Context* d3d11Context, CONST D3DVERTEXELEMENT9* pVertexElements);
		virtual ~D3D11InputLayout() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DVertexDeclaration9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements) override;

		/*** D3D11InputLayout methods ***/
		ID3D11InputLayout* GetLayout(D3D11VertexShader* vertexShader);
		size_t GetHash() const;
		D3D11_INPUT_ELEMENT_DESC* GetElements() { return m_inputElements.data(); } // no const because of mojo
		size_t GetNumElements() const { return m_inputElements.size(); }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pID3D11InputLayout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_inputElements;
	};

	class D3D11VertexShader : public IDirect3DVertexShader9
	{
	public:
		D3D11VertexShader(D3D11Context* d3d11Context, const DWORD* pFunction);
		virtual ~D3D11VertexShader() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DVertexShader9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(GetFunction)(THIS_ void*, UINT* pSizeOfData) override;

		/*** D3D11VertexShader methods ***/
		MOJOSHADER_d3d11Shader* GetMojoshader() const { return m_mojoshader; }
		ID3D11InputLayout* Compile(D3D11InputLayout* layout);
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		MOJOSHADER_d3d11Shader* m_mojoshader;
		std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D11InputLayout>> m_layoutCache;
	};

	class D3D11PixelShader : public IDirect3DPixelShader9
	{
	public:
		D3D11PixelShader(D3D11Context* d3d11Context, const DWORD* pFunction);
		virtual ~D3D11PixelShader() = default;

		/*** IUnknown methods ***/
		STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) override;
		STDMETHOD_(ULONG, AddRef)(THIS) override;
		STDMETHOD_(ULONG, Release)(THIS) override;

		/*** IDirect3DVertexShader9 methods ***/
		STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) override;
		STDMETHOD(GetFunction)(THIS_ void*, UINT* pSizeOfData) override;

		/*** D3D11VertexShader methods ***/
		MOJOSHADER_d3d11Shader* GetMojoshader() const { return m_mojoshader; }
	private:
		std::atomic<ULONG> m_refCount;

		D3D11Context* m_d3dCtx;
		MOJOSHADER_d3d11Shader* m_mojoshader;
	};

}
