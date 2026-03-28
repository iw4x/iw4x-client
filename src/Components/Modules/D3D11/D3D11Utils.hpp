#pragma once

#define NOT_IMPLEMENTED assert(!"not implemented");

#define NOT_IMPLEMENTED_WARNING OutputDebugStringA("[WARNING] " __FUNCTION__ " not implemented!\n"); return D3D_OK;
#define NOT_IMPLEMENTED_ERROR OutputDebugStringA("[ERROR] " __FUNCTION__ " not implemented!\n"); assert(!"not implemented"); return D3DERR_INVALIDCALL;

namespace D3D11 {
	void UnpackD3DCOLOR(D3DCOLOR D3D9Color, FLOAT* D3D11Color);
	size_t GetFormatSize(DXGI_FORMAT Format);
	size_t GetBlockSize(DXGI_FORMAT Format);
	DXGI_FORMAT D3DFORMAT_to_DXGI_FORMAT(D3DFORMAT Format);
	DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT Format);
	DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT Format);
	LPCSTR D3D9UsageToSemantic(BYTE Usage);
	DXGI_FORMAT D3D9DeclTypeToDXGI_Format(BYTE Usage);
	D3D11_TEXTURE_ADDRESS_MODE D3DTEXTUREADDRESSToD3D11_TEXTURE_ADDRESS_MODE(D3DTEXTUREADDRESS Mode);

	D3D11_STENCIL_OP D3DSTENCILOPToD3D11_STENCIL_OP(D3DSTENCILOP Op);
	D3D11_COMPARISON_FUNC D3DCMPFUNCToD3D11_COMPARISON_FUNC(D3DCMPFUNC Func);
	D3D11_BLEND D3DBLENDToD3D11_BLEND(D3DBLEND Blend, bool alpha = false);
	D3D11_BLEND_OP D3DBLENDOPToD3D11_BLEND_OP(D3DBLENDOP Op);
	
	D3D11_FILL_MODE D3DFILLMODEToD3D11_FILL_MODE(D3DFILLMODE Mode);
	D3D11_CULL_MODE D3DCULLToD3D11_CULL_MODE(D3DCULL Mode);

	// Taken from Microsoft's MiniEngine https://github.com/microsoft/DirectX-Graphics-Samples/blob/389246/MiniEngine/Core/PipelineState.cpp#L125
#define ENABLE_SSE_CRC32 0
	inline size_t HashRange(const uint32_t* const Begin, const uint32_t* const End, size_t Hash)
	{
#if ENABLE_SSE_CRC32
		const uint64_t* Iter64 = (const uint64_t*)D3D11::AlignUp((uint64_t)Begin, 8);
		const uint64_t* const End64 = (const uint64_t* const)D3D11::AlignDown((uint64_t)End, 8);

		// If not 64-bit aligned, start with a single u32
		if ((uint32_t*)Iter64 > Begin)
			Hash = _mm_crc32_u32((uint32_t)Hash, *Begin);

		// Iterate over consecutive u64 values
		while (Iter64 < End64)
			Hash = _mm_crc32_u64((uint64_t)Hash, *Iter64++);

		// If there is a 32-bit remainder, accumulate that
		if ((uint32_t*)Iter64 < End)
			Hash = _mm_crc32_u32((uint32_t)Hash, *(uint32_t*)Iter64);
#else
		// An inexpensive hash for CPUs lacking SSE4.2
		for (const uint32_t* Iter = Begin; Iter < End; ++Iter)
			Hash = 16777619U * Hash ^ *Iter;
#endif

		return Hash;
	}
	template <typename T> inline size_t HashState(const T* StateDesc, size_t Count = 1, size_t Hash = 2166136261U)
	{
		static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
		return HashRange((uint32_t*)StateDesc, (uint32_t*)(StateDesc + Count), Hash);
	}

	// Helpers for aligning values by a power of 2
	template<typename T>
	inline T AlignDown(T size, size_t alignment) noexcept
	{
		if (alignment > 0)
		{
			assert(((alignment - 1) & alignment) == 0);
			auto mask = static_cast<T>(alignment - 1);
			return size & ~mask;
		}
		return size;
	}

	template<typename T>
	inline T AlignUp(T size, size_t alignment) noexcept
	{
		if (alignment > 0)
		{
			assert(((alignment - 1) & alignment) == 0);
			auto mask = static_cast<T>(alignment - 1);
			return (size + mask) & ~mask;
		}
		return size;
	}
}
