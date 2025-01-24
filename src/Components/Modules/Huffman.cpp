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

		// Disable original (de)compression functions

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

		isInitialized = true;
	}
}
