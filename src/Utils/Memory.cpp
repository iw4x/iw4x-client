#include <STDInclude.hpp>

namespace Utils
{
	Memory::Allocator Memory::MemAllocator;

	void* Memory::AllocateAlign(std::size_t length, std::size_t alignment)
	{
		auto* data = _aligned_malloc(length, alignment);
		assert(data);
		if (data) ZeroMemory(data, length);
		return data;
	}

	void* Memory::Allocate(std::size_t length)
	{
		auto* data = std::calloc(length, 1);
		assert(data);
		return data;
	}

	char* Memory::DuplicateString(const std::string& string)
	{
		auto* newString = AllocateArray<char>(string.size() + 1);
		std::memcpy(newString, string.data(), string.size());
		return newString;
	}

	void Memory::Free(void* data)
	{
		std::free(data);
	}

	void Memory::Free(const void* data)
	{
		Free(const_cast<void*>(data));
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
		FreeAlign(const_cast<void*>(data));
	}

	// Complementary function for memset, which checks if memory is filled with a char
	bool Memory::IsSet(void* mem, char chr, std::size_t length)
	{
		auto* memArr = static_cast<char*>(mem);

		for (std::size_t i = 0; i < length; ++i)
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

	Memory::Allocator* Memory::GetAllocator()
	{
		return &Memory::MemAllocator;
	}
}
