#pragma once

namespace Game
{
	typedef int(*CL_GetClientName_t)(int localClientNum, int index, char* buf, int size);
	extern CL_GetClientName_t CL_GetClientName;

	typedef int(*CL_IsCgameInitialized_t)();
	extern CL_IsCgameInitialized_t CL_IsCgameInitialized;

	typedef void(*CL_ConnectFromParty_t)(int controllerIndex, _XSESSION_INFO* hostInfo, netadr_t addr, int numPublicSlots, int numPrivateSlots, const char* mapname, const char* gametype);
	extern CL_ConnectFromParty_t CL_ConnectFromParty;

	typedef void(*CL_DownloadsComplete_t)(int controller);
	extern CL_DownloadsComplete_t CL_DownloadsComplete;

	typedef void(*CL_DrawStretchPicPhysical_t)(float x, float y, float w, float h, float xScale, float yScale, float xay, float yay, const float* color, Material* material);
	extern CL_DrawStretchPicPhysical_t CL_DrawStretchPicPhysical;

	typedef const char* (*CL_GetConfigString_t)(int index);
	extern CL_GetConfigString_t CL_GetConfigString;

	typedef void(*CL_AddReliableCommand_t)(int localClientNum, const char* cmd);
	extern CL_AddReliableCommand_t CL_AddReliableCommand;

	typedef void(*CL_ParseGamestate_t)(int localClientNum, msg_t* msg);
	extern CL_ParseGamestate_t CL_ParseGamestate;

	typedef void(*CL_ParseSnapshot_t)(int localClientNum, msg_t* msg);
	extern CL_ParseSnapshot_t CL_ParseSnapshot;

	typedef void(*CL_ParseServerMessage_t)(int localClientNum, msg_t* msg);
	extern CL_ParseServerMessage_t CL_ParseServerMessage;

	typedef int(*CL_GetMaxRank_t)();
	extern CL_GetMaxRank_t CL_GetMaxRank;

	typedef int(*CL_GetRankForXP_t)(int xp);
	extern CL_GetRankForXP_t CL_GetRankForXP;

	typedef void(*CL_GetRankIcon_t)(int level, int prestige, Material** material);
	extern CL_GetRankIcon_t CL_GetRankIcon;

	typedef void(*CL_HandleRelayPacket_t)(Game::msg_t* msg, int client);
	extern CL_HandleRelayPacket_t CL_HandleRelayPacket;

	typedef void(*CL_ResetViewport_t)();
	extern CL_ResetViewport_t CL_ResetViewport;

	typedef void(*CL_SelectStringTableEntryInDvar_f_t)();
	extern CL_SelectStringTableEntryInDvar_f_t CL_SelectStringTableEntryInDvar_f;

	typedef void(*CL_DrawStretchPic_t)(const ScreenPlacement* scrPlace, float x, float y, float w, float h, int horzAlign, int vertAlign, float s1, float t1, float s2, float t2, const float* color, Material* material);
	extern CL_DrawStretchPic_t CL_DrawStretchPic;

	typedef void(*CL_ConsoleFixPosition_t)();
	extern CL_ConsoleFixPosition_t CL_ConsoleFixPosition;

	typedef int(*CL_GetLocalClientActiveCount_t)();
	extern CL_GetLocalClientActiveCount_t CL_GetLocalClientActiveCount;

	typedef int(*CL_ControllerIndexFromClientNum_t)(int localActiveClientNum);
	extern CL_ControllerIndexFromClientNum_t CL_ControllerIndexFromClientNum;

	typedef int(*CL_MouseEvent_t)(int x, int y, int dx, int dy);
	extern CL_MouseEvent_t CL_MouseEvent;

	typedef void(*CL_WriteDemoClientArchive_t)(void (*write)(const void*, int, int), const playerState_s* ps, const float* viewangles, const float* selectedLocation, float selectedLocationAngle, int localClientNum, int index);
	extern CL_WriteDemoClientArchive_t CL_WriteDemoClientArchive;

	typedef void(*CL_WriteDemoMessage_t)(void (*write)(const void*, int, int), int serverMessageSequence, unsigned char* data, int len, int localClientNum);
	extern CL_WriteDemoMessage_t CL_WriteDemoMessage;

	typedef void(*CL_AddDebugStarWithText_t)(const float* point, const float* starColor, const float* textColor, const char* string, float fontsize, int duration, int fromServer);
	extern CL_AddDebugStarWithText_t CL_AddDebugStarWithText;

	typedef void(*Key_ClearStates_t)(int localClientNum);
	extern Key_ClearStates_t Key_ClearStates;

	extern float* cl_angles;

	extern clientConnection_t* clientConnections;

	extern clientStatic_t* cls;

	extern clientUIActive_t* clientUIActives;

	extern clientActive_t* clients;

	extern voiceCommunication_t* cl_voiceCommunication;

	extern [[nodiscard]] int CL_GetMaxXP();
	extern [[nodiscard]] clientConnection_t* CL_GetLocalClientConnection(int localClientNum);
	extern [[nodiscard]] connstate_t CL_GetLocalClientConnectionState(int localClientNum);
	extern [[nodiscard]] voiceCommunication_t* CL_GetLocalClientVoiceCommunication(int localClientNum);
	extern [[nodiscard]] clientUIActive_t* CL_GetLocalClientUIGlobals(int localClientNum);
	extern [[nodiscard]] clientActive_t* CL_GetLocalClientGlobals(int localClientNum);

	extern void CL_AddDebugStar(const float* point, const float* color, int duration, int fromServer);

	extern void CL_MouseMove(int localClientNum, Game::usercmd_s* cmd, float frametime_base);

	extern void AdjustViewanglesForKeyboard(int localClientNum);
}
