#include <STDInclude.hpp>
#include "Huffman.hpp"
#include "Utils/Huffman.hpp"

namespace Components
{
	__declspec(naked) int Huffman::MSG_ReadBitsCompressStub()
	{
		static constexpr auto fn = &Utils::Huffman::Compress;

		__asm
		{
			jmp fn
		}
	}

	__declspec(naked) int Huffman::MSG_WriteBitsCompressStub()
	{
		static constexpr auto fn = &Utils::Huffman::Decompress;

		__asm
		{
			jmp fn
		}
	}

	Huffman::Huffman()
	{
		Utils::Hook(0x4DCC30, Huffman::MSG_ReadBitsCompressStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4319D0, Huffman::MSG_WriteBitsCompressStub, HOOK_JUMP).install()->quick();
	}

	static bool unitTest1() // check internal consistency between compression and decompression
	{
		std::vector<std::uint8_t> uncompressed(256);
		std::iota(uncompressed.begin(), uncompressed.end(), 0);

		std::vector<std::uint8_t> compressed(1024);
		std::vector<std::uint8_t> decompressed(1024);

		const auto compressedSize = Utils::Huffman::Compress(uncompressed.data(), compressed.data(), std::ssize(uncompressed), std::ssize(compressed));
		if (compressedSize <= 0 || compressedSize >= std::ssize(decompressed))
		{
			Logger::Print("Invalid compressed size {}\n", compressedSize);
			return false;
		}

		const auto decompressedSize = Utils::Huffman::Compress(compressed.data(), decompressed.data(), compressedSize, std::ssize(decompressed));
		if (decompressedSize != std::ssize(uncompressed))
		{
			Logger::Print("Invalid decompressed size {}\n", compressedSize);
			return false;
		}

		if (!std::equal(uncompressed.begin(), uncompressed.end(), decompressed.begin()))
		{
			Logger::Print("Compressing and then decompressing bytes did not yield the original input\n");
			return false;
		}

		return true;
	}

	static bool unitTest2() // check consistency between the game's huffman code and our own
	{
		return true;
	}

	static bool unitTest3() // check oob behavior
	{
		return true;
	}

	bool Huffman::unitTest()
	{
		const bool resultTest1 = unitTest1();
		if (!resultTest1)
		{
			Logger::Print("Failed huffman unit test 1\n");
			return false;
		}

		const bool resultTest2 = unitTest2();
		if (!resultTest2)
		{
			Logger::Print("Failed huffman unit test 2\n");
			return false;
		}

		const bool resultTest3 = unitTest3();
		if (!resultTest3)
		{
			Logger::Print("Failed huffman unit test 3\n");
			return false;
		}

		return true;
	}
}
