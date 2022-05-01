#include <STDInclude.hpp>

namespace Utils
{
	Utils::Memory::Allocator Memory::MemAllocator;

	void* Memory::AllocateAlign(size_t length, size_t alignment)
	{
		void* data = _aligned_malloc(length, alignment);
		assert(data != nullptr);
		if (data) ZeroMemory(data, length);
		return data;
	}

	void* Memory::Allocate(size_t length)
	{
		void* data = calloc(length, 1);
		assert(data != nullptr);
		return data;
	}

	char* Memory::DuplicateString(const std::string& string)
	{
		char* newString = Memory::AllocateArray<char>(string.size() + 1);
		std::memcpy(newString, string.data(), string.size());
		return newString;
	}

	void Memory::Free(void* data)
	{
		if (data)
		{
			free(data);
		}
	}

	void Memory::Free(const void* data)
	{
		Memory::Free(const_cast<void*>(data));
	}

	void Memory::FreeAlign(void* data)
	{
		if (data)
		{
			_aligned_free(data);
		}
	}

	void Memory::FreeAlign(const void* data)
	{
		Memory::FreeAlign(const_cast<void*>(data));
	}

	// Complementary function for memset, which checks if memory is filled with a char
	bool Memory::IsSet(void* mem, char chr, size_t length)
	{
		char* memArr = reinterpret_cast<char*>(mem);

		for (size_t i = 0; i < length; ++i)
		{
			if (memArr[i] != chr)
			{
				return false;
			}
		}

		return true;
	}

	bool Memory::IsBadReadPtr(const void* ptr)
	{
		MEMORY_BASIC_INFORMATION mbi = { nullptr };
		if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
		{
			DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
			bool b = !(mbi.Protect & mask);
			// check the page is not a guard page
			if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

			return b;
		}
		return true;
	}

	bool Memory::IsBadCodePtr(const void* ptr)
	{
		MEMORY_BASIC_INFORMATION mbi = { nullptr };
		if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
		{
			DWORD mask = (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
			bool b = !(mbi.Protect & mask);
			// check the page is not a guard page
			if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

			return b;
		}
		return true;
	}

	Utils::Memory::Allocator* Memory::GetAllocator()
	{
		return &Memory::MemAllocator;
	}
}
