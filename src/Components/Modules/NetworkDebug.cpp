#include <STDInclude.hpp>
#include "NetworkDebug.hpp"

namespace Components
{
	void NetworkDebug::CL_ParseServerMessage_Hk(Game::msg_t* msg)
	{
		auto file = Game::FS_FOpenFileWrite("badpacket.dat");
		if (file)
		{
			Game::FS_Write(msg->data, msg->cursize, file);
			Game::FS_FCloseFile(file);
		}
		Game::MSG_Discard(msg);
	}

	void NetworkDebug::CL_ParseBadPacket_f()
	{
		Game::msg_t msg;
		unsigned char* file;

		auto fileSize = Game::FS_ReadFile("badpacket.dat", reinterpret_cast<char**>(&file));
		if (fileSize < 0)
		{
			return;
		}

		ZeroMemory(&msg, sizeof(msg));
		msg.cursize = fileSize;
		msg.data = file;
		Game::MSG_ReadLong(&msg);
		Game::MSG_ReadLong(&msg);
		assert(0 && "Time to debug this packet, baby!");
		Game::CL_ParseServerMessage(0, &msg);
		Game::FS_FreeFile(file);
	}

	NetworkDebug::NetworkDebug()
	{
#ifdef _DEBUG
		Utils::Hook(0x4AA06A, CL_ParseServerMessage_Hk, HOOK_CALL).install()->quick();
		Command::Add("parseBadPacket", CL_ParseBadPacket_f);
#endif

		// Backport updates from IW5
		Utils::Hook::Set<const char*>(0x45D112, "CL_PacketEvent - ignoring illegible message\n");
	}
}
