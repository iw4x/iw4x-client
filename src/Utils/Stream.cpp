#include "STDInclude.hpp"

namespace Utils
{
	Stream::Stream() : CriticalSectionState(0)
	{
		memset(Stream::BlockSize, 0, sizeof(Stream::BlockSize));
	}

	Stream::Stream(size_t size) : Stream()
	{
		Stream::Buffer.reserve(size);
	}

	Stream::~Stream()
	{
		Stream::Buffer.clear();

		if (Stream::CriticalSectionState != 0)
		{
			MessageBoxA(0, Utils::VA("Invalid critical section state '%i' for stream destruction!", Stream::CriticalSectionState), "WARNING", MB_ICONEXCLAMATION);
		}
	};

	size_t Stream::Length()
	{
		return Stream::Buffer.length();
	}

	size_t Stream::Capacity()
	{
		return Stream::Buffer.capacity();
	}

	char* Stream::Save(const void* _str, size_t size, size_t count)
	{
		return Stream::Save(Stream::GetCurrentBlock(), _str, size, count);
	}

	char* Stream::Save(Game::XFILE_BLOCK_TYPES stream, const void * _str, size_t size, size_t count)
	{
		//if (stream == XFILE_BLOCK_TEMP || stream == XFILE_BLOCK_VIRTUAL || stream == XFILE_BLOCK_PHYSICAL) // Only those seem to actually write data.
																											 // As I'm not sure though, I'll still write the data
																											 // Use IncreaseStreamSize to fill virtual streams
		auto data = Stream::Data();

		if (Stream::IsCriticalSection() && Stream::Length() + (size * count) > Stream::Capacity())
		{
			MessageBoxA(0, Utils::VA("Potential stream reallocation during critical operation detected! Writing data of the length 0x%X exceeds the allocated stream size of 0x%X\n", (size * count), Stream::Capacity()), "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		Stream::Buffer.append(static_cast<const char*>(_str), size * count);

		if (Stream::Data() != data && Stream::IsCriticalSection())
		{
			MessageBoxA(0, "Stream reallocation during critical operations not permitted!\nPlease increase the initial memory size or reallocate memory during non-critical sections!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		Stream::IncreaseBlockSize(stream, size * count); // stay up to date on those streams

		return Stream::At() - (size * count);
	}

	char* Stream::Save(Game::XFILE_BLOCK_TYPES stream, int value, size_t count)
	{
		auto ret = Stream::Length();

		for (size_t i = 0; i < count; i++)
		{
			Stream::Save(stream, &value, 4, 1);
		}

		return Stream::Data() + ret;
	}

	char* Stream::SaveString(std::string string)
	{
		return Stream::SaveString(string.data()/*, string.size()*/);
	}

	char* Stream::SaveString(const char* string)
	{
		return Stream::SaveString(string, strlen(string));
	}

	char* Stream::SaveString(const char* string, size_t len)
	{
		auto ret = Stream::Length();

		if (string)
		{
			Stream::Save(string, len);
		}

		Stream::SaveNull();

		return Stream::Data() + ret;
	}

	char* Stream::SaveText(std::string string)
	{
		return Stream::Save(string.data(), string.length());
	}

	char* Stream::SaveByte(unsigned char byte, size_t count)
	{
		auto ret = Stream::Length();

		for (size_t i = 0; i < count; i++)
		{
			Stream::Save(&byte, 1);
		}

		return Stream::Data() + ret;
	}

	char* Stream::SaveNull(size_t count)
	{
		return Stream::SaveByte(0, count);
	}

	char* Stream::SaveMax(size_t count)
	{
		return Stream::SaveByte(static_cast<unsigned char>(-1), count);
	}

	void Stream::Align(Stream::Alignment align)
	{
		uint32_t size = 2 << align;

		// Not power of 2!
		if (!size || (size & (size - 1))) return;
		--size;

		Game::XFILE_BLOCK_TYPES stream = Stream::GetCurrentBlock();
		Stream::BlockSize[stream] = ~size & (Stream::GetBlockSize(stream) + size);
	}

	bool Stream::PushBlock(Game::XFILE_BLOCK_TYPES stream)
	{
		Stream::StreamStack.push_back(stream);
		return Stream::IsValidBlock(stream);
	}

	bool Stream::PopBlock()
	{
		if (Stream::StreamStack.size())
		{
			Stream::StreamStack.pop_back();
			return true;
		}

		return false;
	}

	bool Stream::IsValidBlock(Game::XFILE_BLOCK_TYPES stream)
	{
		return (stream < Game::MAX_XFILE_COUNT && stream >= Game::XFILE_BLOCK_TEMP);
	}

	void Stream::IncreaseBlockSize(Game::XFILE_BLOCK_TYPES stream, unsigned int size)
	{
		if (Stream::IsValidBlock(stream))
		{
			Stream::BlockSize[stream] += size;
		}
	}

	void Stream::IncreaseBlockSize(unsigned int size)
	{
		return IncreaseBlockSize(Stream::GetCurrentBlock(), size);
	}

	Game::XFILE_BLOCK_TYPES Stream::GetCurrentBlock()
	{
		if (Stream::StreamStack.size())
		{
			return Stream::StreamStack[Stream::StreamStack.size() - 1];
		}

		return Game::XFILE_BLOCK_INVALID;
	}

	char* Stream::At()
	{
		return reinterpret_cast<char*>(Stream::Data() + Stream::Length());
	}

	char* Stream::Data()
	{
		return const_cast<char*>(Stream::Buffer.data());
	}

	unsigned int Stream::GetBlockSize(Game::XFILE_BLOCK_TYPES stream)
	{
		if (Stream::IsValidBlock(stream))
		{
			return Stream::BlockSize[stream];
		}

		return 0;
	}

	DWORD Stream::GetPackedOffset()
	{
		Game::XFILE_BLOCK_TYPES block = Stream::GetCurrentBlock();

		Stream::Offset offset;
		offset.block = block;
		offset.offset = Stream::GetBlockSize(block);
		return offset.GetPackedOffset();
	}

	void Stream::ToBuffer(std::string& outBuffer)
	{
		outBuffer.clear();
		outBuffer.append(Stream::Data(), Stream::Length());
	}

	std::string Stream::ToBuffer()
	{
		std::string buffer;
		Stream::ToBuffer(buffer);
		return buffer;
	}

	void Stream::EnterCriticalSection()
	{
		++Stream::CriticalSectionState;
	}

	void Stream::LeaveCriticalSection()
	{
		--Stream::CriticalSectionState;
	}

	bool Stream::IsCriticalSection()
	{
		if (Stream::CriticalSectionState < 0)
		{
			MessageBoxA(0, "CriticalSectionState in stream has been overrun!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		return (Stream::CriticalSectionState != 0);
	}
}
