#pragma once

namespace Game
{
	typedef void*(*Z_VirtualAlloc_t)(int size);
	extern Z_VirtualAlloc_t Z_VirtualAlloc;

	typedef void*(*Z_Malloc_t)(int size);
	extern Z_Malloc_t Z_Malloc;

	typedef void*(*Hunk_AllocateTempMemoryHigh_t)(int size);
	extern Hunk_AllocateTempMemoryHigh_t Hunk_AllocateTempMemoryHigh;

	typedef void*(*Hunk_UserAlloc_t)(HunkUser* user, int size, int alignment);
	extern Hunk_UserAlloc_t Hunk_UserAlloc;

	typedef char*(*TempMalloc_t)(int len);
	extern TempMalloc_t TempMalloc;

	constexpr auto PAGE_SIZE = 4096;

	extern [[nodiscard]] void* Z_VirtualReserve(int size);
	extern void Z_VirtualCommit(void* ptr, int size);
	extern void Z_VirtualDecommit(void* ptr, int size);
	extern void Z_VirtualFree(void* ptr);
}
