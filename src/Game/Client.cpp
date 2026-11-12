#include "Client.hpp"

namespace Game
{
  CL_GetClientName_t CL_GetClientName = CL_GetClientName_t(0x140101D60);
  CL_IsCgameInitialized_t CL_IsCgameInitialized = CL_IsCgameInitialized_t(0x1400F4DA0);
  CL_InitRenderer_t CL_InitRenderer = CL_InitRenderer_t(0x1400FBD40);
  CL_ConnectFromParty_t CL_ConnectFromParty = CL_ConnectFromParty_t(0x1400F8900);
  CL_DownloadsComplete_t CL_DownloadsComplete = CL_DownloadsComplete_t(0x1400FA210);
  CL_DrawStretchPicPhysical_t CL_DrawStretchPicPhysical = CL_DrawStretchPicPhysical_t(0x1400F43D0);
  CL_GetConfigString_t CL_GetConfigString = CL_GetConfigString_t(0x1400F47A0);
  CL_AddReliableCommand_t CL_AddReliableCommand = CL_AddReliableCommand_t(0x1400F80F0);
  CL_ParseGamestate_t CL_ParseGamestate = CL_ParseGamestate_t(0x1400FFC50);
  CL_ParseSnapshot_t CL_ParseSnapshot = CL_ParseSnapshot_t(0x140100E80);
  CL_ParseServerMessage_t CL_ParseServerMessage = CL_ParseServerMessage_t(0x140100C80);
  CL_GetMaxRank_t CL_GetMaxRank = CL_GetMaxRank_t(0x140101590);
  CL_GetRankForXP_t CL_GetRankForXP = CL_GetRankForXP_t(0x140101680);
  CL_GetRankIcon_t CL_GetRankIcon = CL_GetRankIcon_t(0x140101750);
  CL_ResetViewport_t CL_ResetViewport = CL_ResetViewport_t(0x1400F2CC0);
  CL_SelectStringTableEntryInDvar_f_t CL_SelectStringTableEntryInDvar_f = CL_SelectStringTableEntryInDvar_f_t(0x14026A0E0);
  CL_DrawStretchPic_t CL_DrawStretchPic = CL_DrawStretchPic_t(0x1400F4170);
  CL_ConsoleFixPosition_t CL_ConsoleFixPosition = CL_ConsoleFixPosition_t(0x1401F5720);
  CL_ControllerIndexFromClientNum_t CL_ControllerIndexFromClientNum = CL_ControllerIndexFromClientNum_t(0x1400F30F0);
  CL_MouseEvent_t CL_MouseEvent = CL_MouseEvent_t(0x1400F7030);
  CL_WriteDemoClientArchive_t CL_WriteDemoClientArchive = CL_WriteDemoClientArchive_t(0x1400FDFE0);
  CL_MouseMove_t CL_MouseMove = CL_MouseMove_t(0x1400F70D0);

  Key_ClearStates_t Key_ClearStates = Key_ClearStates_t(0x1400EF4E0);

  float* cl_angles = reinterpret_cast<float*>(0x1406CFEF8);

  clientConnection_t* clientConnections = reinterpret_cast<clientConnection_t*>(0x140BF9620);

  clientStatic_t* cls = reinterpret_cast<clientStatic_t*>(0x140C5AC20);

  clientUIActive_t* clientUIActives = reinterpret_cast<clientUIActive_t*>(0x1406CC030);

  bool* cl_serverChangingMap = reinterpret_cast<bool*>(0x1406CC02A);
  char* loadingMapName = reinterpret_cast<char*>(0x1466421C0);

  clientActive_t* clients = reinterpret_cast<clientActive_t*>(0x1406CCCC0);

  GfxConfiguration* gfxCfg = reinterpret_cast<GfxConfiguration*>(0x148CCA7C0);

  cg_s* cgArray = reinterpret_cast<cg_s*>(0x140474820);
  cgs_t* cgsArray = reinterpret_cast<cgs_t*>(0x140585390);
  cgEntity_s* cg_entitiesArray = reinterpret_cast<cgEntity_s*>(0x14059C9A0);
  dvar_t** cl_paused = reinterpret_cast<dvar_t**>(0x1406BB048);

  voiceCommunication_t* cl_voiceCommunication = reinterpret_cast<voiceCommunication_t*>(0x140D07A80);

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

  cg_s* CL_GetLocalClientGlobals(const int localClientNum)
  {
    AssertIn(localClientNum, MAX_LOCAL_CLIENTS);
    assert(clients[localClientNum].alwaysFalse == false);
    return &cgArray[localClientNum];
  }

  centity_s* CG_GetEntity([[maybe_unused]] int localClientNum, int entityIndex)
  {
    AssertIn(entityIndex, MAX_GENTITIES);
    assert(localClientNum == 0);

    return &cg_entitiesArray[entityIndex].entity;
  }
}
