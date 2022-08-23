#include <STDInclude.hpp>

namespace Game
{
	CL_GetClientName_t CL_GetClientName = CL_GetClientName_t(0x4563D0);
	CL_IsCgameInitialized_t CL_IsCgameInitialized = CL_IsCgameInitialized_t(0x43EB20);
	CL_ConnectFromParty_t CL_ConnectFromParty = CL_ConnectFromParty_t(0x433D30);
	CL_DownloadsComplete_t CL_DownloadsComplete = CL_DownloadsComplete_t(0x42CE90);
	CL_DrawStretchPicPhysical_t CL_DrawStretchPicPhysical = CL_DrawStretchPicPhysical_t(0x4FC120);
	CL_GetConfigString_t CL_GetConfigString = CL_GetConfigString_t(0x44ADB0);
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

	float* cl_angles = reinterpret_cast<float*>(0xB2F8D0);

	clientConnection_t* clientConnections = reinterpret_cast<clientConnection_t*>(0xA1E878);

	clientStatic_t* cls = reinterpret_cast<clientStatic_t*>(0xA7FE90);

	clientUIActive_t* clientUIActives = reinterpret_cast<clientUIActive_t*>(0xB2BB88);

	voiceCommunication_t* cl_voiceCommunication = reinterpret_cast<voiceCommunication_t*>(0x1079DA0);

	int CL_GetMaxXP()
	{
		StringTable* rankTable = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, "mp/rankTable.csv").stringTable;
		const char* maxrank = StringTable_Lookup(rankTable, 0, "maxrank", 1);
		return atoi(StringTable_Lookup(rankTable, 0, maxrank, 7));
	}

	clientConnection_t* CL_GetLocalClientConnection(const int localClientNum)
	{
		assert(clientConnections);
		AssertIn(localClientNum, MAX_LOCAL_CLIENTS);

		return &clientConnections[localClientNum];
	}

	connstate_t CL_GetLocalClientConnectionState(const int localClientNum)
	{
		AssertIn(localClientNum, STATIC_MAX_LOCAL_CLIENTS);

		return clientUIActives[localClientNum].connectionState;
	}

	voiceCommunication_t* CL_GetLocalClientVoiceCommunication([[maybe_unused]] const int localClientNum)
	{
		AssertIn(localClientNum, STATIC_MAX_LOCAL_CLIENTS);

		return cl_voiceCommunication;
	}
}
