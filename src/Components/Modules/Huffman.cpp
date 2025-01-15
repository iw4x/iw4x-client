#include "Huffman.hpp"

namespace Components
{
	Huffman::Huffman()
	{
		Utils::Hook(0x414D92, [](const unsigned char* from, unsigned char* to, int fromSize) // SV_ExecuteClientMessage
			{ return Utils::Huffman::Decompress(from, to, fromSize, 0x800); }, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A9F56, [](const unsigned char* from, unsigned char* to, int fromSize) // CL_ParseServerMessage
			{ return Utils::Huffman::Decompress(from, to, fromSize, 0x20000); }, HOOK_CALL).install()->quick();

		Utils::Hook(0x411C16, [](bool, const unsigned char* from, unsigned char* to, int fromSize) // CL_WritePacket
			{ return Utils::Huffman::Compress(from, to, fromSize, 0x800); }, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A85A1, [](bool, const unsigned char* from, unsigned char* to, int fromSize) // CL_Record_f
			{ return Utils::Huffman::Compress(from, to, fromSize, 0x20000); }, HOOK_CALL).install()->quick();
		Utils::Hook(0x48FEDD, [](bool, const unsigned char* from, unsigned char* to, int fromSize) // SV_SendMessageToClient
			{ return Utils::Huffman::Compress(from, to, fromSize, 0x20000); }, HOOK_CALL).install()->quick();

		// Disable the original (de)compression functions
		// Cannot disable when performing unit tests because they call the original functions
		if (!Components::Loader::IsPerformingUnitTests())
		{
			Utils::Hook(Game::MSG_ReadBitsCompress, [](const unsigned char*, unsigned char*, int) // MSG_ReadBitsCompress
			{
				Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Cannot use the original MSG_ReadBitsCompress function!\n");
				return 0;
			}, HOOK_JUMP).install()->quick();
			Utils::Hook(Game::MSG_WriteBitsCompress, [](bool, const unsigned char*, unsigned char*, int) // MSG_WriteBitsCompress
			{
				Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Cannot use the original MSG_WriteBitsCompress function!\n");
				return 0;
			}, HOOK_JUMP).install()->quick();
		}

		isInitialized = true;
	}

	static bool unitTest1() // check internal consistency between compression and decompression, and consistency between the game's huffman code and our own
	{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		std::array<std::uint8_t, 1024> uncompressed{};
		std::iota(uncompressed.begin(), uncompressed.begin() + 256, static_cast<std::uint8_t>(0));

		for (std::size_t i = 256; i < uncompressed.size(); ++i)
		{
			uncompressed[i] = static_cast<std::uint8_t>(rand());
		}

		std::array<std::uint8_t, uncompressed.size() * 4> compressed{};
		std::array<std::uint8_t, uncompressed.size() * 4> decompressed{};

		const auto compressedSize = Utils::Huffman::Compress(uncompressed.data(), compressed.data(), std::ssize(uncompressed), std::ssize(compressed));
		if (compressedSize <= 0 || compressedSize >= std::ssize(compressed))
		{
			Logger::Print("Invalid compressed size {}\n", compressedSize);
			return false;
		}

		const auto decompressedSize = Utils::Huffman::Decompress(compressed.data(), decompressed.data(), compressedSize, std::ssize(decompressed));
		if (std::abs(decompressedSize - std::ssize(uncompressed)) > 1)
		{
			Logger::Print("Invalid decompressed size {}\n", decompressedSize);
			return false;
		}

		if (std::memcmp(uncompressed.data(), decompressed.data(), std::min<std::size_t>(uncompressed.size(), decompressedSize)))
		{
			Logger::Print("Compressing and then decompressing bytes did not yield the original input\n");
			return false;
		}

		std::array<std::uint8_t, uncompressed.size() * 4> compressedGame{};
		std::array<std::uint8_t, uncompressed.size() * 4> decompressedGame{};

		const auto compressedSizeGame = Game::MSG_WriteBitsCompress(false, uncompressed.data(), compressedGame.data(), std::ssize(uncompressed));
		if (compressedSizeGame <= 0 || compressedSizeGame >= std::ssize(compressedGame))
		{
			Logger::Print("Invalid compressed size {}\n", compressedSizeGame);
			return false;
		}

		const auto decompressedSizeGame = Game::MSG_ReadBitsCompress(compressedGame.data(), decompressedGame.data(), compressedSizeGame);
		if (std::abs(decompressedSizeGame - std::ssize(uncompressed)) > 1)
		{
			Logger::Print("Invalid decompressed size {}\n", decompressedSizeGame);
			return false;
		}

		if (std::memcmp(uncompressed.data(), decompressedGame.data(), std::min<std::size_t>(uncompressed.size(), decompressedSizeGame)))
		{
			Logger::Print("Compressing and then decompressing bytes did not yield the original input\n");
			return false;
		}

		if (std::abs(compressedSizeGame - compressedSize) > 1)
		{
			Logger::Print("Compressed sizes differ too much\n");
			return false;
		}
		if (std::abs(decompressedSizeGame - decompressedSize) > 1)
		{
			Logger::Print("Decompressed sizes differ too much\n");
			return false;
		}
		if (std::memcmp(decompressed.data(), decompressedGame.data(), std::min<std::size_t>(decompressedSize, decompressedSizeGame)))
		{
			Logger::Print("The decompressed output from the game's huffman code is different from our own\n");
			return false;
		}

		return true;
	}

	static bool unitTest2() // check oob behavior
	{
		const std::array<std::uint8_t, 256> uncompressed{};
		std::array<std::uint8_t, uncompressed.size() / 4> compressed{};
		std::array<std::uint8_t, compressed.size() / 2> decompressed{};

		const auto compressedSize = Utils::Huffman::Compress(uncompressed.data(), compressed.data(), std::ssize(uncompressed), std::ssize(compressed));
		if (compressedSize != std::ssize(compressed))
		{
			Logger::Print("Invalid compressed size {}, shouldn't write out of bounds\n", compressedSize);
			return false;
		}

		const auto decompressedSize = Utils::Huffman::Decompress(compressed.data(), decompressed.data(), compressedSize, std::ssize(decompressed));
		if (decompressedSize != std::ssize(decompressed))
		{
			Logger::Print("Invalid decompressed size {}, shouldn't write out of bounds\n", decompressedSize);
			return false;
		}

		return true;
	}

	static bool unitTest3() // check 'special' oob behavior - max 2 bytes oob read in decompression and max 2 bytes oob write in compression
	{
		{
			const std::array<std::uint8_t, 1> uncompressed{ 183 };
			std::array<std::uint8_t, uncompressed.size() * 4> compressed{};

			// this should trigger a debug statement about OOB detection
			if (Utils::Huffman::Compress(uncompressed.data(), compressed.data(), std::ssize(uncompressed), 1) != 1)
			{
				return false;
			}
		}
		{
			const std::array<std::uint8_t, 2> compressed{ 0b11111111 };
			std::array<std::uint8_t, compressed.size() * 4> decompressed{};

			// this should trigger a debug statement about OOB detection
			static_cast<void>(Utils::Huffman::Decompress(compressed.data(), decompressed.data(), 1, std::ssize(decompressed)));
		}

		return true;
	}

	bool Huffman::unitTest()
	{
		if (!unitTest1())
		{
			Logger::Print("Failed huffman unit test 1\n");
			return false;
		}

		if (!unitTest2())
		{
			Logger::Print("Failed huffman unit test 2\n");
			return false;
		}

		if (!unitTest3())
		{
			Logger::Print("Failed huffman unit test 3\n");
			return false;
		}

		return true;
	}
}
