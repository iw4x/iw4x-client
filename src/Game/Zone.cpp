#include <STDInclude.hpp>

namespace Game
{
	Z_VirtualAlloc_t Z_VirtualAlloc = Z_VirtualAlloc_t(0x4CFBA0);
	Z_Malloc_t Z_Malloc = Z_Malloc_t(0x4F3680);

	Hunk_AllocateTempMemoryHigh_t Hunk_AllocateTempMemoryHigh = Hunk_AllocateTempMemoryHigh_t(0x475B30);
	Hunk_UserAlloc_t Hunk_UserAlloc = Hunk_UserAlloc_t(0x45D1C0);

	TempMalloc_t TempMalloc = TempMalloc_t(0x4613A0);

	int Z_TryVirtualCommitInternal(void* ptr, int size)
	{
		assert((size >= 0));

		return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
	}

	void Z_VirtualDecommitInternal(void* ptr, int size)
	{
		assert((size >= 0));
#pragma warning(push)
#pragma warning(disable: 6250)
		VirtualFree(ptr, size, MEM_DECOMMIT);
#pragma warning(pop)
	}

	void Z_VirtualCommitInternal(void* ptr, int size)
	{
		if (Z_TryVirtualCommitInternal(ptr, size))
		{
			return;
		}

		Sys_OutOfMemError();
	}

	void Z_VirtualFreeInternal(void* ptr)
	{
		VirtualFree(ptr, 0, MEM_RELEASE);
	}

	void Z_VirtualCommit(void* ptr, int size)
	{
		assert(ptr);
		assert(size);

		Z_VirtualCommitInternal(ptr, size);
	}

	void* Z_VirtualReserve(int size)
	{
		assert((size >= 0));

		void* buf = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
		assert(buf);
		return buf;
	}

	void Z_VirtualDecommit(void* ptr, int size)
	{
		assert(ptr);
		assert(size);

		Z_VirtualDecommitInternal(ptr, size);
	}

	void Z_VirtualFree(void* ptr)
	{
		Z_VirtualFreeInternal(ptr);
	}
}
