#pragma once

namespace Game
{
	typedef gentity_s*(*SV_AddTestClient_t)();
	extern SV_AddTestClient_t SV_AddTestClient;

	typedef int(*SV_IsTestClient_t)(int clientNum);
	extern SV_IsTestClient_t SV_IsTestClient;

	typedef int(*SV_GameClientNum_Score_t)(int clientID);
	extern SV_GameClientNum_Score_t SV_GameClientNum_Score;

	typedef void(*SV_GameSendServerCommand_t)(int clientNum, svscmd_type type, const char* text);
	extern SV_GameSendServerCommand_t SV_GameSendServerCommand;

	typedef void(*SV_SendServerCommand_t)(client_s* cl, svscmd_type type, const char* fmt, ...);
	extern SV_SendServerCommand_t SV_SendServerCommand;

	typedef void(*SV_Cmd_TokenizeString_t)(const char* string);
	extern SV_Cmd_TokenizeString_t SV_Cmd_TokenizeString;

	typedef void(*SV_Cmd_EndTokenizedString_t)();
	extern SV_Cmd_EndTokenizedString_t SV_Cmd_EndTokenizedString;

	typedef void(*SV_Cmd_ArgvBuffer_t)(int arg, char* buf, int size);
	extern SV_Cmd_ArgvBuffer_t SV_Cmd_ArgvBuffer;

	typedef void(*SV_SetConfigstring_t)(int index, const char* string);
	extern SV_SetConfigstring_t SV_SetConfigstring;

	typedef unsigned int(*SV_GetConfigstringConst_t)(int index);
	extern SV_GetConfigstringConst_t SV_GetConfigstringConst;

	typedef void(*SV_DirectConnect_t)(netadr_t adr);
	extern SV_DirectConnect_t SV_DirectConnect;

	typedef bool(*SV_Loaded_t)();
	extern SV_Loaded_t SV_Loaded;

	typedef void(*SV_ClientThink_t)(client_s* cl, usercmd_s* cmd);
	extern SV_ClientThink_t SV_ClientThink;

	typedef void(*SV_DropClient_t)(client_s* drop, const char* reason, bool tellThem);
	extern SV_DropClient_t SV_DropClient;

	typedef client_s*(*SV_GetPlayerByName_t)();
	extern SV_GetPlayerByName_t SV_GetPlayerByName;

	typedef client_s*(*SV_GetPlayerByNum_t)();
	extern SV_GetPlayerByNum_t SV_GetPlayerByNum;

	typedef client_s*(*SV_FindClientByAddress_t)(netadr_t from, int qport, int remoteClientIndex);
	extern SV_FindClientByAddress_t SV_FindClientByAddress;

	typedef void(*SV_WaitServer_t)();
	extern SV_WaitServer_t SV_WaitServer;

	typedef char*(*SV_GetClientPersistentDataBuffer_t)(int clientNum);
	extern SV_GetClientPersistentDataBuffer_t SV_GetClientPersistentDataBuffer;

	typedef char*(*SV_GetClientPersistentDataModifiedFlags_t)(int clientNum);
	extern SV_GetClientPersistentDataModifiedFlags_t SV_GetClientPersistentDataModifiedFlags;

	constexpr auto MAX_STATPACKETS = 7;

	extern int* svs_time;
	extern int* sv_timeResidual;
	extern int* sv_serverId_value;
	extern int* svs_clientCount;
	extern client_s* svs_clients;

	extern unsigned short* sv_sconfigstrings;
	extern unsigned short* sv_emptyConfigString;

	extern volatile long* sv_thread_owns_game;

	extern int SV_GetServerThreadOwnsGame();
	extern void SV_GameDropClient(int clientNum, const char* reason);
	extern void SV_DropAllBots();
	extern int SV_GetClientStat(int clientNum, int index);
	extern void SV_SetClientStat(int clientNum, int index, int value);
	extern void SV_BotUserMove(client_s* client);
}
