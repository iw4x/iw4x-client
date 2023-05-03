#include <STDInclude.hpp>

namespace Game
{
	SV_AddTestClient_t SV_AddTestClient = SV_AddTestClient_t(0x48AD30);
	SV_IsTestClient_t SV_IsTestClient = SV_IsTestClient_t(0x4D6E40);
	SV_GameClientNum_Score_t SV_GameClientNum_Score = SV_GameClientNum_Score_t(0x469AC0);
	SV_GameSendServerCommand_t SV_GameSendServerCommand = SV_GameSendServerCommand_t(0x4BC3A0);
	SV_SendServerCommand_t SV_SendServerCommand = SV_SendServerCommand_t(0x4255A0);
	SV_Cmd_TokenizeString_t SV_Cmd_TokenizeString = SV_Cmd_TokenizeString_t(0x4B5780);
	SV_Cmd_EndTokenizedString_t SV_Cmd_EndTokenizedString = SV_Cmd_EndTokenizedString_t(0x464750);
	SV_Cmd_ArgvBuffer_t SV_Cmd_ArgvBuffer = SV_Cmd_ArgvBuffer_t(0x40BB60);
	SV_DirectConnect_t SV_DirectConnect = SV_DirectConnect_t(0x460480);
	SV_SetConfigstring_t SV_SetConfigstring = SV_SetConfigstring_t(0x4982E0);
	SV_GetConfigstringConst_t SV_GetConfigstringConst = SV_GetConfigstringConst_t(0x468500);
	SV_Loaded_t SV_Loaded = SV_Loaded_t(0x4EE3E0);
	SV_ClientThink_t SV_ClientThink = SV_ClientThink_t(0x44ADD0);
	SV_DropClient_t SV_DropClient = SV_DropClient_t(0x4D1600);
	SV_GetPlayerByName_t SV_GetPlayerByName = SV_GetPlayerByName_t(0x6242B0);
	SV_GetPlayerByNum_t SV_GetPlayerByNum = SV_GetPlayerByNum_t(0x624390);
	SV_FindClientByAddress_t SV_FindClientByAddress = SV_FindClientByAddress_t(0x44F450);
	SV_WaitServer_t SV_WaitServer = SV_WaitServer_t(0x4256F0);
	SV_GetClientPersistentDataBuffer_t SV_GetClientPersistentDataBuffer = SV_GetClientPersistentDataBuffer_t(0x4014C0);
	SV_GetClientPersistentDataModifiedFlags_t SV_GetClientPersistentDataModifiedFlags = SV_GetClientPersistentDataModifiedFlags_t(0x4F4AC0);

	int* svs_time = reinterpret_cast<int*>(0x31D9384);
	int* sv_timeResidual = reinterpret_cast<int*>(0x2089E14);
	int* sv_serverId_value = reinterpret_cast<int*>(0x2089DC0);
	int* svs_clientCount = reinterpret_cast<int*>(0x31D938C);
	client_s* svs_clients = reinterpret_cast<client_s*>(0x31D9390);

	unsigned short* sv_sconfigstrings = reinterpret_cast<unsigned short*>(0x208A632);
	unsigned short* sv_emptyConfigString = reinterpret_cast<unsigned short*>(0x208A630);

	volatile long* sv_thread_owns_game = reinterpret_cast<volatile long*>(0x2089DB8);

	int SV_GetServerThreadOwnsGame()
	{
		return *sv_thread_owns_game;
	}

	void SV_GameDropClient(int clientNum, const char* reason)
	{
		assert((*sv_maxclients)->current.integer >= 1 && (*sv_maxclients)->current.integer <= 18);

		if (clientNum >= 0 && clientNum < (*sv_maxclients)->current.integer)
		{
			SV_DropClient(&svs_clients[clientNum], reason, true);
		}
	}

	void SV_DropAllBots()
	{
		for (auto i = 0; i < *svs_clientCount; ++i)
		{
			if (svs_clients[i].header.state != CS_FREE
				&& svs_clients[i].header.netchan.remoteAddress.type == NA_BOT)
			{
				SV_GameDropClient(i, "GAME_GET_TO_COVER");
			}
		}
	}

	int SV_GetClientStat(int clientNum, int index)
	{
		assert(svs_clients[clientNum].statPacketsReceived == (1 << MAX_STATPACKETS) - 1);
		assert(svs_clients[clientNum].header.state >= CS_CONNECTED);

		if (index < 2000)
		{
			// Skip first 4 bytes of the binary blob
			return svs_clients[clientNum].stats.__s0.binary[index];
		}

		if (index < 3498)
		{
			return svs_clients[clientNum].stats.__s0.data[index - 2000];
		}

		assert(0 && "Unhandled stat index");
		return 0;
	}

	void SV_SetClientStat(int clientNum, int index, int value)
	{
		assert(svs_clients[clientNum].statPacketsReceived == (1 << MAX_STATPACKETS) - 1);
		assert(svs_clients[clientNum].header.state >= CS_CONNECTED);

		if (index < 2000)
		{
			assert(value >= 0 && value <= 255);

			if (svs_clients[clientNum].stats.__s0.binary[index] == value)
			{
				return;
			}

			svs_clients[clientNum].stats.__s0.binary[index] = static_cast<unsigned char>(value);
		}
		else if (index < 3498)
		{
			if (svs_clients[clientNum].stats.__s0.data[index - 2000] == value)
			{
				return;
			}

			svs_clients[clientNum].stats.__s0.data[index - 2000] = value;
		}
		else
		{
			assert(0 && "Unhandled stat index");
			return;
		}

		SV_SendServerCommand(&svs_clients[clientNum], SV_CMD_RELIABLE, "%c %i %i", 'M', index, value);
	}

	void SV_BotUserMove(client_s* client)
	{
		static DWORD SV_BotUserMove_t = 0x626E50;

		__asm
		{
			pushad
			mov edi, client
			call SV_BotUserMove_t
			popad
		}
	}
}
