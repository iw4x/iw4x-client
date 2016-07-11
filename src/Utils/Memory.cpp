#include "STDInclude.hpp"

namespace Utils
{
	void* Memory::Allocate(size_t length)
	{
		void* data = new char[length];

		assert(data != nullptr);

		if (data)
		{
			ZeroMemory(data, length);
		}

		return data;
	}

	char* Memory::DuplicateString(std::string string)
	{
		char* newString = Memory::AllocateArray<char>(string.size() + 1);
		std::memcpy(newString, string.data(), string.size());
		return newString;
	}

	void Memory::Free(void* data)
	{
		if (data)
		{
			delete[] data;
		}
	}

	void Memory::Free(const void* data)
	{
		Memory::Free(const_cast<void*>(data));
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
}
