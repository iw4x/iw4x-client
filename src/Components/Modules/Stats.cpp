#include "STDInclude.hpp"

namespace Components
{
	bool Stats::IsMaxLevel()
	{
		// obtain statsbuffer (offset 4, first 4 bytes is statVersion.)
		char* statsBuffer = Utils::Hook::Call<char *(int)>(0x4C49F0)(0) + 4;

		// obtain experience from statsbuffer (offset 2056)
		std::uint32_t experience = *reinterpret_cast<std::uint32_t*>(statsBuffer + 2056);

		// 2516000 should be the max experience.
		if (experience >= 2516000) return true;
		return false;
	}

	void Stats::SendStats()
	{
		// check if we're connected to a server...
		if (*reinterpret_cast<std::uint32_t*>(0xB2C540) >= 7)
		{
			for (unsigned char i = 0; i < 7; i++)
			{
				Game::Com_Printf(0, "Sending stat packet %i to server.\n", i);

				// alloc
				Game::msg_t msg;
				memset(&msg, 0, sizeof(Game::msg_t));

				char* buffer = new char[2048];
				memset(buffer, 0, 2048);

				// init
				Game::MSG_Init(&msg, buffer, 2048);
				Game::MSG_WriteString(&msg, "stats");

				// get stat buffer
				char *statbuffer = nullptr;
				if (Utils::Hook::Call<int(int)>(0x444CA0)(0))
				{
					statbuffer = &Utils::Hook::Call<char *(int)>(0x4C49F0)(0)[1240 * i];
				}

				// Client port?
				Game::MSG_WriteShort(&msg, *(short*)0xA1E878);

				// Stat packet index
				Game::MSG_WriteByte(&msg, i);

				// calculate packet size
				size_t size = 8192 - (i * 1240);
				if (size > 1240)
					size = 1240;

				// write stat packet data
				if (statbuffer)
				{
					Game::MSG_WriteData(&msg, statbuffer, size);
				}

				// send statpacket
				Game::NET_OutOfBandData(Game::NS_CLIENT, *(Game::netadr_t*)0xA1E888, msg.data, msg.cursize);

				// free memory
				delete[] buffer;
			}
		}
	}

	void Stats::UpdateClasses(UIScript::Token)
	{
		SendStats();
	}

	Stats::Stats()
	{
		// This UIScript should be added in the onClose code of the cac_popup menu,
		// so everytime the create-a-class window is closed, and a client is connected
		// to a server, the stats data of the client will be reuploaded to the server.
		// allowing the player to change their classes while connected to a server.
		UIScript::Add("UpdateClasses", Stats::UpdateClasses);

		// Allow playerdata to be changed while connected to a server
		Utils::Hook::Set<BYTE>(0x4376FD, 0xEB);

		// ToDo: Allow playerdata changes in setPlayerData UI script.
	}
	Stats::~Stats()
	{

	}
}
