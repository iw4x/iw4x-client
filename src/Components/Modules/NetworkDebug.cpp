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

	int NetworkDebug::I_stricmp_Stub(const char* s0, const char* s1)
	{
		assert(s0);
		assert(s1);

		if (!s0)
		{
			return -1;
		}

		if (!s1)
		{
			return 1;
		}

		return Utils::Hook::Call<int(const char*, const char*, int)>(0x426080)(s0, s1, std::numeric_limits<int>::max()); // I_strnicmp
	}

	NetworkDebug::NetworkDebug()
	{
#ifdef _DEBUG
		Utils::Hook(0x4AA06A, CL_ParseServerMessage_Hk, HOOK_CALL).install()->quick();
		Command::Add("parseBadPacket", CL_ParseBadPacket_f);
#endif

		// Address "race" condition where commands received from RCon can be null
		Utils::Hook(0x6094DA, I_stricmp_Stub, HOOK_CALL).install()->quick(); // Cmd_ExecuteServerString
		Utils::Hook(0x6095D7, I_stricmp_Stub, HOOK_CALL).install()->quick(); // Cmd_ExecuteSingleCommand

		// Backport updates from IW5
		Utils::Hook::Set<const char*>(0x45D112, "CL_PacketEvent - ignoring illegible message\n");
	}
}
