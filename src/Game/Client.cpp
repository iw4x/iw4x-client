#include <STDInclude.hpp>

namespace Game
{
	CL_GetClientName_t CL_GetClientName = CL_GetClientName_t(0x4563D0);
	CL_IsCgameInitialized_t CL_IsCgameInitialized = CL_IsCgameInitialized_t(0x43EB20);
	CL_ConnectFromParty_t CL_ConnectFromParty = CL_ConnectFromParty_t(0x433D30);
	CL_DownloadsComplete_t CL_DownloadsComplete = CL_DownloadsComplete_t(0x42CE90);
	CL_DrawStretchPicPhysical_t CL_DrawStretchPicPhysical = CL_DrawStretchPicPhysical_t(0x4FC120);
	CL_GetConfigString_t CL_GetConfigString = CL_GetConfigString_t(0x44ADB0);
	CL_AddReliableCommand_t CL_AddReliableCommand = CL_AddReliableCommand_t(0x454F40);
	CL_ParseGamestate_t CL_ParseGamestate = CL_ParseGamestate_t(0x5AC250);
	CL_ParseSnapshot_t CL_ParseSnapshot = CL_ParseSnapshot_t(0x5ABD40);
	CL_ParseServerMessage_t CL_ParseServerMessage = CL_ParseServerMessage_t(0x4A9E90);
	CL_GetMaxRank_t CL_GetMaxRank = CL_GetMaxRank_t(0x44BA30);
	CL_GetRankForXP_t CL_GetRankForXP = CL_GetRankForXP_t(0x4FF8A0);
	CL_GetRankIcon_t CL_GetRankIcon = CL_GetRankIcon_t(0x4A7B30);
	CL_HandleRelayPacket_t CL_HandleRelayPacket = CL_HandleRelayPacket_t(0x5A8C70);
	CL_ResetViewport_t CL_ResetViewport = CL_ResetViewport_t(0x4A8830);
	CL_SelectStringTableEntryInDvar_f_t CL_SelectStringTableEntryInDvar_f = CL_SelectStringTableEntryInDvar_f_t(0x4A4560);
	CL_DrawStretchPic_t CL_DrawStretchPic = CL_DrawStretchPic_t(0x412490);
	CL_ConsoleFixPosition_t CL_ConsoleFixPosition = CL_ConsoleFixPosition_t(0x44A430);
	CL_GetLocalClientActiveCount_t CL_GetLocalClientActiveCount = CL_GetLocalClientActiveCount_t(0x5BAD90);
	CL_ControllerIndexFromClientNum_t CL_ControllerIndexFromClientNum = CL_ControllerIndexFromClientNum_t(0x449E30);
	CL_MouseEvent_t CL_MouseEvent = CL_MouseEvent_t(0x4D7C50);
	CL_WriteDemoClientArchive_t CL_WriteDemoClientArchive = CL_WriteDemoClientArchive_t(0x5A8020);
	CL_WriteDemoMessage_t CL_WriteDemoMessage = CL_WriteDemoMessage_t(0x4707C0);
	CL_AddDebugStarWithText_t CL_AddDebugStarWithText = CL_AddDebugStarWithText_t(0x4D03C0);

	Key_ClearStates_t Key_ClearStates = Key_ClearStates_t(0x5A7F00);

	float* cl_angles = reinterpret_cast<float*>(0xB2F8D0);

	clientConnection_t* clientConnections = reinterpret_cast<clientConnection_t*>(0xA1E878);

	clientStatic_t* cls = reinterpret_cast<clientStatic_t*>(0xA7FE90);

	clientUIActive_t* clientUIActives = reinterpret_cast<clientUIActive_t*>(0xB2BB88);

	clientActive_t* clients = reinterpret_cast<clientActive_t*>(0xB2C698);

	voiceCommunication_t* cl_voiceCommunication = reinterpret_cast<voiceCommunication_t*>(0x1079DA0);

	int CL_GetMaxXP()
	{
		StringTable* rankTable = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, "mp/rankTable.csv").stringTable;
		const char* maxrank = StringTable_Lookup(rankTable, 0, "maxrank", 1);
		return std::atoi(StringTable_Lookup(rankTable, 0, maxrank, 7));
	}

	clientConnection_t* CL_GetLocalClientConnection(const int localClientNum)
	{
		assert(clientConnections);
		AssertIn(localClientNum, MAX_LOCAL_CLIENTS);

		return &clientConnections[localClientNum];
	}

	connstate_t CL_GetLocalClientConnectionState(const int localClientNum)
	{
		AssertOffset(clientUIActive_t, connectionState, 0x9B8);
		AssertIn(localClientNum, STATIC_MAX_LOCAL_CLIENTS);

		return clientUIActives[localClientNum].connectionState;
	}

	voiceCommunication_t* CL_GetLocalClientVoiceCommunication([[maybe_unused]] const int localClientNum)
	{
		AssertIn(localClientNum, STATIC_MAX_LOCAL_CLIENTS);

		return cl_voiceCommunication;
	}

	clientUIActive_t* CL_GetLocalClientUIGlobals(const int localClientNum)
	{
		AssertIn(localClientNum, MAX_LOCAL_CLIENTS);
		return &clientUIActives[localClientNum];
	}

	clientActive_t* CL_GetLocalClientGlobals(const int localClientNum)
	{
		AssertIn(localClientNum, MAX_LOCAL_CLIENTS);
		assert(clients[localClientNum].alwaysFalse == false);
		return &clients[localClientNum];
	}

	void CL_AddDebugStar(const float* point, const float* color, int duration, int fromServer)
	{
		static const float MY_NULLTEXTCOLOR[] = {0.0f, 0.0f, 0.0f, 0.0f};

		CL_AddDebugStarWithText(point, color, MY_NULLTEXTCOLOR, nullptr, 1.0f, duration, fromServer);
	}

	void CL_MouseMove(const int localClientNum, Game::usercmd_s* cmd, const float frametime_base)
	{
		static const DWORD CL_MouseMove_t = 0x5A6240;

		__asm
		{
			pushad
			mov ebx, cmd
			mov eax, localClientNum
			push frametime_base
			call CL_MouseMove_t
			add esp, 0x4
			popad
		}
	}

	void AdjustViewanglesForKeyboard(const int localClientNum)
	{
		static const DWORD AdjustViewanglesForKeyboard_t = 0x5A5D80;

		__asm
		{
			pushad
			mov eax, localClientNum
			call AdjustViewanglesForKeyboard_t
			popad
		}
	}
}
