#include "Huffman.hpp"

namespace Components
{
  Huffman::Huffman()
  {
    // Note that on x64 MSG_ReadBitsCompress() takes the size of the input
    // after it and the size of the output, which its callers pass (0x800 in
    // SV_ExecuteClientMessage() and 0x20000 in CL_ParseServerMessage()).
    //
    Utils::Hook(0x140236641, +[](const unsigned char* from, int fromSize, unsigned char* to, int toSize) // SV_ExecuteClientMessage
      { return Utils::Huffman::Decompress(from, to, fromSize, toSize); }, HOOK_CALL).install()->quick();
    Utils::Hook(0x140100D19, +[](const unsigned char* from, int fromSize, unsigned char* to, int toSize) // CL_ParseServerMessage
      { return Utils::Huffman::Decompress(from, to, fromSize, toSize); }, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400F7995, +[](bool, const unsigned char* from, unsigned char* to, int fromSize) // CL_WritePacket
      { return Utils::Huffman::Compress(from, to, fromSize, 0x800); }, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400FD1E8, +[](bool, const unsigned char* from, unsigned char* to, int fromSize) // CL_Record_f
      { return Utils::Huffman::Compress(from, to, fromSize, 0x20000); }, HOOK_CALL).install()->quick();
    Utils::Hook(0x1402409D2, +[](bool, const unsigned char* from, unsigned char* to, int fromSize) // SV_SendMessageToClient
      { return Utils::Huffman::Compress(from, to, fromSize, 0x20000); }, HOOK_CALL).install()->quick();

    // Disable original (de)compression functions

    Utils::Hook(Game::MSG_ReadBitsCompress, +[](const unsigned char*, int, unsigned char*, int) // MSG_ReadBitsCompress
    {
      Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Cannot use the original MSG_ReadBitsCompress function!\n");
      return 0;
    }, HOOK_JUMP).install()->quick();

    Utils::Hook(Game::MSG_WriteBitsCompress, +[](bool, const unsigned char*, unsigned char*, int) // MSG_WriteBitsCompress
    {
      Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Cannot use the original MSG_WriteBitsCompress function!\n");
      return 0;
    }, HOOK_JUMP).install()->quick();

    isInitialized = true;
  }
}
