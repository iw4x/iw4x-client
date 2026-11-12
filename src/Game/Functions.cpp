#include "Functions.hpp"

// Unsorted function definitions
namespace Game
{
  __security_init_cookie_t __security_init_cookie = __security_init_cookie_t(0x1402FC6EC);
  __tmainCRTStartup_t __tmainCRTStartup = __tmainCRTStartup_t(0x1402FBF30);

  AngleVectors_t AngleVectors = AngleVectors_t(0x14027C520);

  Cbuf_AddServerText_f_t Cbuf_AddServerText_f = Cbuf_AddServerText_f_t(0x140080E30);
  Cbuf_AddText_t Cbuf_AddText = Cbuf_AddText_t(0x1401E6A10);
  Cbuf_InsertText_t Cbuf_InsertText = Cbuf_InsertText_t(0x1401E6E40);
  Cbuf_Execute_t Cbuf_Execute = Cbuf_Execute_t(0x1401E6AF0);

  CopyStringInternal_t CopyStringInternal = CopyStringInternal_t(0x14027D8C0);

  CG_LocationalTrace_t CG_LocationalTrace = CG_LocationalTrace_t(0x1400CA0A0);
  CG_WorldTrace_t CG_WorldTrace = CG_WorldTrace_t(0x1400CA980);
  CG_ExecuteNewServerCommands_t CG_ExecuteNewServerCommands = CG_ExecuteNewServerCommands_t(0x1400E76D0);
  CG_GetClientNum_t CG_GetClientNum = CG_GetClientNum_t(0x1400D62C0);
  CG_PlayBoltedEffect_t CG_PlayBoltedEffect = CG_PlayBoltedEffect_t(0x1400AF2A0);
  CG_GetBoneIndex_t CG_GetBoneIndex = CG_GetBoneIndex_t(0x1402AA690);
  CG_ScoresDown_f_t CG_ScoresDown_f = CG_ScoresDown_f_t(0x1400D0660);
  CG_GetTeamName_t CG_GetTeamName = CG_GetTeamName_t(0x1400DFD40);
  CG_SetupWeaponConfigString_t CG_SetupWeaponConfigString = CG_SetupWeaponConfigString_t(0x1400C54B0);

  CL_DrawTextWithCursor_t CL_DrawTextWithCursor = CL_DrawTextWithCursor_t(0x1400FA900);
  CL_GetSnapshot_t CL_GetSnapshot = CL_GetSnapshot_t(0x1400F4820);

  Cmd_AddCommand_t Cmd_AddCommand = Cmd_AddCommand_t(0x1401E6F00);
  Cmd_AddServerCommand_t Cmd_AddServerCommand = Cmd_AddServerCommand_t(0x1401E6F70);
  Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = Cmd_ExecuteSingleCommand_t(0x1401E72D0);

  DB_AllocStreamPos_t DB_AllocStreamPos = DB_AllocStreamPos_t(0x140131FB0);
  DB_PushStreamPos_t DB_PushStreamPos = DB_PushStreamPos_t(0x140132120);
  DB_PopStreamPos_t DB_PopStreamPos = DB_PopStreamPos_t(0x1401320E0);

  Encode_Init_t Encode_Init = Encode_Init_t(0x1402A86F0);

  Field_Clear_t Field_Clear = Field_Clear_t(0x1401F6880);

  Free_String_t Free_String = Free_String_t(0x14027D8E0);

  Image_LoadFromFileWithReader_t Image_LoadFromFileWithReader = Image_LoadFromFileWithReader_t(0x14006A160);
  Image_Release_t Image_Release = Image_Release_t(0x140037650);
  Image_Setup_t Image_Setup = Image_Setup_t(0x14006AC40);

  Info_ValueForKey_t Info_ValueForKey = Info_ValueForKey_t(0x14028AF50);
  Info_Validate_t Info_Validate = Info_Validate_t(0x14028AF10);

  Key_SetCatcher_t Key_SetCatcher = Key_SetCatcher_t(0x1400EF9E0);
  Key_RemoveCatcher_t Key_RemoveCatcher = Key_RemoveCatcher_t(0x1400EF930);
  Key_IsCatcherActive_t Key_IsCatcherActive = Key_IsCatcherActive_t(0x1400EF720);
  Key_SetBinding_t Key_SetBinding = Key_SetBinding_t(0x1400EF960);

  LargeLocalInit_t LargeLocalInit = LargeLocalInit_t(0x14027D660);

  Load_Stream_t Load_Stream = Load_Stream_t(0x140132280);
  Load_XString_t Load_XString = Load_XString_t(0x140132250);
  Load_XModelPtr_t Load_XModelPtr = Load_XModelPtr_t(0x1401236E0);
  Load_XStringArray_t Load_XStringArray = Load_XStringArray_t(0x140123A90);
  Load_XStringCustom_t Load_XStringCustom = Load_XStringCustom_t(0x1401322D0);
  Load_FxEffectDefHandle_t Load_FxEffectDefHandle = Load_FxEffectDefHandle_t(0x140118FE0);
  Load_GfxImagePtr_t Load_GfxImagePtr = Load_GfxImagePtr_t(0x14011A840);
  Load_Texture_t Load_Texture = Load_Texture_t(0x140037AE0);
  Load_GfxTextureLoad_t Load_GfxTextureLoad = Load_GfxTextureLoad_t(0x14011AD60);
  Load_SndAliasCustom_t Load_SndAliasCustom = Load_SndAliasCustom_t(0x14027F050);
  Load_MaterialHandle_t Load_MaterialHandle = Load_MaterialHandle_t(0x14011D100);
  Load_PhysCollmapPtr_t Load_PhysCollmapPtr = Load_PhysCollmapPtr_t(0x14011E4A0);
  Load_PhysPresetPtr_t Load_PhysPresetPtr = Load_PhysPresetPtr_t(0x14011E650);
  Load_TracerDefPtr_t Load_TracerDefPtr = Load_TracerDefPtr_t(0x14011FAC0);

  Menus_CloseAll_t Menus_CloseAll = Menus_CloseAll_t(0x1402654E0);
  Menus_CloseRequest_t Menus_CloseRequest = Menus_CloseRequest_t(0x140265700);
  Menus_OpenByName_t Menus_OpenByName = Menus_OpenByName_t(0x1402662D0);
  Menus_FindByName_t Menus_FindByName = Menus_FindByName_t(0x140265850);
  Menu_IsVisible_t Menu_IsVisible = Menu_IsVisible_t(0x1402640D0);
  Menus_MenuIsInStack_t Menus_MenuIsInStack = Menus_MenuIsInStack_t(0x140265F40);
  Menu_HandleKey_t Menu_HandleKey = Menu_HandleKey_t(0x1402634E0);
  Menu_GetFocused_t Menu_GetFocused = Menu_GetFocused_t(0x140263330);
  Menu_Setup_t Menu_Setup = Menu_Setup_t(0x140265470);
  Menu_SetNextCursorItem_t Menu_SetNextCursorItem = Menu_SetNextCursorItem_t(0x140264D60);
  Menu_SetPrevCursorItem_t Menu_SetPrevCursorItem = Menu_SetPrevCursorItem_t(0x1402650E0);

  MSG_Init_t MSG_Init = MSG_Init_t(0x140200250);
  MSG_ReadBit_t MSG_ReadBit = MSG_ReadBit_t(0x1400AFB30);
  MSG_ReadBits_t MSG_ReadBits = MSG_ReadBits_t(0x1400AFBD0);
  MSG_ReadData_t MSG_ReadData = MSG_ReadData_t(0x1402005E0);
  MSG_ReadLong_t MSG_ReadLong = MSG_ReadLong_t(0x1402008A0);
  MSG_ReadShort_t MSG_ReadShort = MSG_ReadShort_t(0x1402009D0);
  MSG_ReadInt64_t MSG_ReadInt64 = MSG_ReadInt64_t(0x1402006B0);
  MSG_ReadBigString_t  MSG_ReadBigString = MSG_ReadBigString_t(0x140200420);
  MSG_ReadStringLine_t MSG_ReadStringLine = MSG_ReadStringLine_t(0x140200B60);
  MSG_WriteByte_t MSG_WriteByte = MSG_WriteByte_t(0x140200F70);
  MSG_WriteData_t MSG_WriteData = MSG_WriteData_t(0x140200F90);
  MSG_WriteLong_t MSG_WriteLong = MSG_WriteLong_t(0x140201030);
  MSG_WriteShort_t MSG_WriteShort = MSG_WriteShort_t(0x140201060);
  MSG_WriteString_t MSG_WriteString = MSG_WriteString_t(0x140201090);
  MSG_Discard_t MSG_Discard = MSG_Discard_t(0x140200210);
  MSG_ReadByte_t MSG_ReadByte = MSG_ReadByte_t(0x140200570);
  MSG_ReadDeltaUsercmdKey_t MSG_ReadDeltaUsercmdKey = MSG_ReadDeltaUsercmdKey_t(0x140203980);
  MSG_ReadBitsCompress_t MSG_ReadBitsCompress = MSG_ReadBitsCompress_t(0x1402004E0);
  MSG_WriteBitsCompress_t MSG_WriteBitsCompress = MSG_WriteBitsCompress_t(0x140200F10);
  Huff_offsetReceive_t Huff_offsetReceive = Huff_offsetReceive_t(0x1401FE180);

  NetadrToSockadr_t NetadrToSockadr = NetadrToSockadr_t(0x14027DE90);

  NET_CompareAdr_t NET_CompareAdr = NET_CompareAdr_t(0x140204A50);
  NET_CompareBaseAdr_t NET_CompareBaseAdr = NET_CompareBaseAdr_t(0x140204B20);
  NET_DeferPacketToClient_t NET_DeferPacketToClient = NET_DeferPacketToClient_t(0x140204BE0);
  NET_Init_t NET_Init = NET_Init_t(0x1402A6350);
  NET_IsLocalAddress_t NET_IsLocalAddress = NET_IsLocalAddress_t(0x140204EF0);
  NET_StringToAdr_t NET_StringToAdr = NET_StringToAdr_t(0x140205250);
  NET_OutOfBandPrint_t NET_OutOfBandPrint = NET_OutOfBandPrint_t(0x140204FC0);
  NET_OutOfBandData_t NET_OutOfBandData = NET_OutOfBandData_t(0x140204F00);
  NET_OutOfBandVoiceData_t NET_OutOfBandVoiceData = NET_OutOfBandVoiceData_t(0x140205080);

  Live_GetMapIndex_t Live_GetMapIndex = Live_GetMapIndex_t(0x14027E740);
  Live_GetPrestige_t Live_GetPrestige = Live_GetPrestige_t(0x1402A73D0);
  Live_GetXp_t Live_GetXp = Live_GetXp_t(0x1402A7450);
  Live_GetLocalClientName_t Live_GetLocalClientName = Live_GetLocalClientName_t(0x14024BE20);
  Live_IsSystemUiActive_t Live_IsSystemUiActive = Live_IsSystemUiActive_t(0x1402A2420);

  LiveStorage_GetStat_t LiveStorage_GetStat = LiveStorage_GetStat_t(0x1401F7970);
  LiveStorage_SetStat_t LiveStorage_SetStat = LiveStorage_SetStat_t(0x1401F8DA0);
  LiveStorage_DoWeHaveStats_t LiveStorage_DoWeHaveStats = LiveStorage_DoWeHaveStats_t(0x1401F71D0);
  LiveStorage_ReadStats_t LiveStorage_ReadStats = LiveStorage_ReadStats_t(0x1402A70F0);

  Scr_AddSourceBuffer_t Scr_AddSourceBuffer = Scr_AddSourceBuffer_t(0x14021D0D0);

  Party_GetMaxPlayers_t Party_GetMaxPlayers = Party_GetMaxPlayers_t(0x1402418B0);
  PartyHost_CountMembers_t PartyHost_CountMembers = PartyHost_CountMembers_t(0x140105770);
  PartyHost_GetMemberAddressBySlot_t PartyHost_GetMemberAddressBySlot = PartyHost_GetMemberAddressBySlot_t(0x1402415D0);
  PartyHost_GetMemberName_t PartyHost_GetMemberName = PartyHost_GetMemberName_t(0x1401108C0);
  Party_InParty_t Party_InParty = Party_InParty_t(0x14010A5F0);

  Playlist_ParsePlaylists_t Playlist_ParsePlaylists = Playlist_ParsePlaylists_t(0x1402597F0);

  PMoveSingle_t PMoveSingle = PMoveSingle_t(0x140094490);

  PM_CheckLadderMove_t PM_CheckLadderMove = PM_CheckLadderMove_t(0x14008F070);
  PM_Weapon_t PM_Weapon = PM_Weapon_t(0x140097CB0);

  R_AddCmdDrawStretchPic_t R_AddCmdDrawStretchPic = R_AddCmdDrawStretchPic_t(0x14001BE70);
  R_AllocStaticIndexBuffer_t R_AllocStaticIndexBuffer = R_AllocStaticIndexBuffer_t(0x140038B20);
  R_Cinematic_StartPlayback_Now_t R_Cinematic_StartPlayback_Now = R_Cinematic_StartPlayback_Now_t(0x140036070);
  R_Cinematic_BinkClose_t R_Cinematic_BinkClose = R_Cinematic_BinkClose_t(0x140035930);
  R_RegisterFont_t R_RegisterFont = R_RegisterFont_t(0x14001A9D0);
  R_AddCmdDrawText_t R_AddCmdDrawText = R_AddCmdDrawText_t(0x14001C3F0);
  R_AddCmdDrawConsoleText_t R_AddCmdDrawConsoleText = R_AddCmdDrawConsoleText_t(0x14001C950);
  R_LoadGraphicsAssets_t R_LoadGraphicsAssets = R_LoadGraphicsAssets_t(0x140030C10);
  R_TextWidth_t R_TextWidth = R_TextWidth_t(0x14001AA00);
  R_TextHeight_t R_TextHeight = R_TextHeight_t(0x14001AB50);
  R_SortWorldSurfaces_t R_SortWorldSurfaces = R_SortWorldSurfaces_t(0x140053DE0);
  R_SetSunFromDvars_t R_SetSunFromDvars = R_SetSunFromDvars_t(0x140056270);
  R_GetCharacterGlyph_t R_GetCharacterGlyph = R_GetCharacterGlyph_t(0x14001A930);

  RMesg_SendMessages_t RMesg_SendMessages = RMesg_SendMessages_t(0x14028BF30);

  SEH_StringEd_GetString_t SEH_StringEd_GetString = SEH_StringEd_GetString_t(0x14024D7F0);
  SEH_ReadCharFromString_t SEH_ReadCharFromString = SEH_ReadCharFromString_t(0x14024D6C0);
  SEH_GetCurrentLanguage_t SEH_GetCurrentLanguage = SEH_GetCurrentLanguage_t(0x1402A2B90);
  SEH_GetLanguageName_t SEH_GetLanguageName = SEH_GetLanguageName_t(0x14024CF80);

  SND_Init_t SND_Init = SND_Init_t(0x1402445A0);

  SockadrToNetadr_t SockadrToNetadr = SockadrToNetadr_t(0x14027DF20);

  Steam_JoinLobby_t Steam_JoinLobby = Steam_JoinLobby_t(0x14024C320);

  TeleportPlayer_t TeleportPlayer = TeleportPlayer_t(0x1401A0280);

  UI_AddMenuList_t UI_AddMenuList = UI_AddMenuList_t(0x140267A30);
  UI_GetActiveMenu_t UI_GetActiveMenu = UI_GetActiveMenu_t(0x14026CE10);
  UI_LoadMenus_t UI_LoadMenus = UI_LoadMenus_t(0x140268BE0);
  UI_UpdateArenas_t UI_UpdateArenas = UI_UpdateArenas_t(0x140269D20);
  UI_SortArenas_t UI_SortArenas = UI_SortArenas_t(0x140269C00);
  UI_DrawHandlePic_t UI_DrawHandlePic = UI_DrawHandlePic_t(0x14024F460);
  ScrPlace_GetActivePlacement_t ScrPlace_GetActivePlacement = ScrPlace_GetActivePlacement_t(0x1400F2B70);
  UI_TextWidth_t UI_TextWidth = UI_TextWidth_t(0x1402715B0);
  UI_TextHeight_t UI_TextHeight = UI_TextHeight_t(0x140271570);
  UI_DrawText_t UI_DrawText = UI_DrawText_t(0x14026B5E0);
  UI_GetFontHandle_t UI_GetFontHandle = UI_GetFontHandle_t(0x14026D330);
  ScrPlace_ApplyRect_t ScrPlace_ApplyRect = ScrPlace_ApplyRect_t(0x1400F2390);
  UI_KeyEvent_t UI_KeyEvent = UI_KeyEvent_t(0x14026DE90);
  UI_SafeTranslateString_t UI_SafeTranslateString = UI_SafeTranslateString_t(0x140270C90);
  UI_ReplaceConversions_t UI_ReplaceConversions = UI_ReplaceConversions_t(0x1402701B0);
  UI_GetMapDisplayName_t UI_GetMapDisplayName = UI_GetMapDisplayName_t(0x14026D760);
  UI_LoadArenas_t UI_LoadArenas = UI_LoadArenas_t(0x140269D20);
  UI_GetGameTypeDisplayName_t UI_GetGameTypeDisplayName = UI_GetGameTypeDisplayName_t(0x14026D3E0);
  ParseConfigStringToStruct_t ParseConfigStringToStruct = ParseConfigStringToStruct_t(0x14028B260);

  Win_GetLanguage_t Win_GetLanguage = Win_GetLanguage_t(0x1402A2BA0);

  Vec3UnpackUnitVec_t Vec3UnpackUnitVec = Vec3UnpackUnitVec_t(0x14027E2C0);
  vectoryaw_t vectoryaw = vectoryaw_t(0x14027C4B0);
  AngleNormalize360_t AngleNormalize360 = AngleNormalize360_t(0x140277770);

  unzClose_t unzClose = unzClose_t(0x1402CA580);

  RB_DrawCursor_t RB_DrawCursor = RB_DrawCursor_t(0x14004DEA0);
  RB_DrawChar_t RB_DrawChar = RB_DrawChar_t(0x14004DDF0);
  RB_DrawStretchPicRotate_t RB_DrawStretchPicRotate = RB_DrawStretchPicRotate_t(0x1400481A0);

  R_NormalizedTextScale_t R_NormalizedTextScale = R_NormalizedTextScale_t(0x14001A9E0);

  Material_Process2DTextureCoordsForAtlasing_t Material_Process2DTextureCoordsForAtlasing = Material_Process2DTextureCoordsForAtlasing_t(0x14001A6A0);

  Byte4PackRgba_t Byte4PackRgba = Byte4PackRgba_t(0x140030390);
  RandWithSeed_t RandWithSeed = RandWithSeed_t(0x14027B110);

  Field_Draw_t Field_Draw = Field_Draw_t(0x1400EEEA0);
  Field_AdjustScroll_t Field_AdjustScroll = Field_AdjustScroll_t(0x1400EEB20);
  ScrPlace_GetViewPlacement_t ScrPlace_GetViewPlacement = ScrPlace_GetViewPlacement_t(0x1400F2BB0);
  vectoangles_t vectoangles = vectoangles_t(0x14027C090);
  DiffTrackAngle_t DiffTrackAngle = DiffTrackAngle_t(0x1402787E0);
  CG_DObjGetWorldTagPos_t CG_DObjGetWorldTagPos = CG_DObjGetWorldTagPos_t(0x1400D4090);
  CG_UsingTurretOverlay_t CG_UsingTurretOverlay = CG_UsingTurretOverlay_t(0x1400C4770);

  Weapon_RocketLauncher_Fire_t Weapon_RocketLauncher_Fire = Weapon_RocketLauncher_Fire_t(0x1401896B0);
  Bullet_Fire_t Bullet_Fire = Bullet_Fire_t(0x140161B20);

  IN_Frame_t IN_Frame = IN_Frame_t(0x1402A1090);
  IN_MouseEvent_t IN_MouseEvent = IN_MouseEvent_t(0x1402A12F0);
  IN_RecenterMouse_t IN_RecenterMouse = IN_RecenterMouse_t(0x1402A1390);

  IN_Init_t IN_Init = IN_Init_t(0x1402A12A0);
  IN_Shutdown_t IN_Shutdown = IN_Shutdown_t(0x1402A1460);

  Touch_Item_t Touch_Item = Touch_Item_t(0x14016DEA0);

  Add_Ammo_t Add_Ammo = Add_Ammo_t(0x14016BDF0);

  ClientUserinfoChanged_t ClientUserinfoChanged = ClientUserinfoChanged_t(0x140196890);

  player_die_t player_die = player_die_t(0x14019C4C0);

  Vec3Normalize_t Vec3Normalize = Vec3Normalize_t(0x14001ED50);
  Vec2Normalize_t Vec2Normalize = Vec2Normalize_t(0x14027B9C0);
  Vec3NormalizeTo_t Vec3NormalizeTo = Vec3NormalizeTo_t(0x14027BB90);

  I_strncpyz_t I_strncpyz = I_strncpyz_t(0x14028A8B0);
  I_CleanStr_t I_CleanStr = I_CleanStr_t(0x14028A4E0);
  I_isdigit_t I_isdigit = I_isdigit_t(0x14028A570);

  XNAddrToString_t XNAddrToString = XNAddrToString_t(0x140205990);

  Voice_IncomingVoiceData_t Voice_IncomingVoiceData = Voice_IncomingVoiceData_t(0x1402A80C0);
  Voice_IsClientTalking_t Voice_IsClientTalking = Voice_IsClientTalking_t(0x1402A8590);

  LargeLocalConstruct_t LargeLocalConstruct = LargeLocalConstruct_t(0x14027CC90);
  LargeLocalReset_t LargeLocalReset = LargeLocalReset_t(0x14027D760);

  StructuredDataDef_GetAsset_t StructuredDataDef_GetAsset = StructuredDataDef_GetAsset_t(0x14027F5A0);

  StringTable_Lookup_t StringTable_Lookup = StringTable_Lookup_t(0x14027F2E0);
  StringTable_HashString_t StringTable_HashString = StringTable_HashString_t(0x14027F4A0);
  StringTable_GetAsset_FastFile_t StringTable_GetAsset_FastFile = StringTable_GetAsset_FastFile_t(0x14027F270);
  StringTable_LookupRowNumForValue_t StringTable_LookupRowNumForValue = StringTable_LookupRowNumForValue_t(0x14027F3E0);
  StringTable_GetColumnValueForRow_t StringTable_GetColumnValueForRow = StringTable_GetColumnValueForRow_t(0x14027F290);

  longjmp_internal_t longjmp_internal = longjmp_internal_t(0x140326CFC);

  CmdArgs* cmd_args = reinterpret_cast<CmdArgs*>(0x141BBA490);
  CmdArgs* sv_cmd_args = reinterpret_cast<CmdArgs*>(0x141BBA540);

  cmd_function_s** cmd_functions = reinterpret_cast<cmd_function_s**>(0x141BBA5E8);

  float* cgameFOVSensitivityScale = reinterpret_cast<float*>(0x1406CFEAC);

  UiContext* uiContext = reinterpret_cast<UiContext*>(0x14663E870);
  UiContext* cgDC = reinterpret_cast<Game::UiContext*>(0x1406BB280);

  int* arenaCount = reinterpret_cast<int*>(0x1465D06D0);
  mapArena_t* arenas = reinterpret_cast<mapArena_t*>(0x1465D06D4);

  int* gameTypeCount = reinterpret_cast<int*>(0x1465CEE40);
  gameTypeName_t* gameTypes = reinterpret_cast<gameTypeName_t*>(0x1465CEE44);

  bool* g_lobbyCreateInProgress = reinterpret_cast<bool*>(0x14651CF02);
  PartyData* g_lobbyData = reinterpret_cast<PartyData*>(0x140D09A00);
  PartyData* g_partyData = reinterpret_cast<PartyData*>(0x140D0BE10);

  SessionData* g_serverSession = reinterpret_cast<SessionData*>(0x14650F630);

  int* numIP = reinterpret_cast<int*>(0x14678A2C0);
  netIP_t* localIP = reinterpret_cast<netIP_t*>(0x14678A2D0);

  netadr_t* connectedHost = reinterpret_cast<netadr_t*>(0x140BF9630);

  SOCKET* ip_socket = reinterpret_cast<SOCKET*>(0x14678A2B0);

  SafeArea* safeArea = reinterpret_cast<SafeArea*>(0x1406C26A8);

  SpawnVar* spawnVars = reinterpret_cast<SpawnVar*>(0x1418652E8);
  MapEnts** marMapEntsPtr = reinterpret_cast<MapEnts**>(0x140DB56A8);
  clipMap_t* cm = reinterpret_cast<clipMap_t*>(0x141BD1000);

  IDirect3D9** d3d9 = reinterpret_cast<IDirect3D9**>(0x148CC4A68);
  IDirect3DDevice9** dx_ptr = reinterpret_cast<IDirect3DDevice9**>(0x148CC4A70);

  mapname_t* mapnames = reinterpret_cast<mapname_t*>(0x1403A3EE0);

  char*** varXString = reinterpret_cast<char***>(0x140DB4CE8);
  TracerDef*** varTracerDefPtr = reinterpret_cast<TracerDef***>(0x140DB54B8);
  XModel*** varXModelPtr = reinterpret_cast<XModel***>(0x140DB56D0);
  XModel** varXModel = reinterpret_cast<XModel**>(0x140DB56D8);
  const char** varConstChar = reinterpret_cast<const char**>(0x140DB4CE8);
  Material*** varMaterialHandle = reinterpret_cast<Material***>(0x140DB61F0);
  FxEffectDef*** varFxEffectDefHandle = reinterpret_cast<FxEffectDef***>(0x140DB5730);
  PhysCollmap*** varPhysCollmapPtr = reinterpret_cast<PhysCollmap***>(0x140DB5138);
  PhysPreset*** varPhysPresetPtr = reinterpret_cast<PhysPreset***>(0x140DB56C0);
  snd_alias_list_t*** varsnd_alias_list_name = reinterpret_cast<snd_alias_list_t***>(0x140DB5988);
  MaterialVertexShader** varMaterialVertexShader = reinterpret_cast<MaterialVertexShader**>(0x140DB47E8);

  visField_t* visionDefFields = reinterpret_cast<visField_t*>(0x14041D9F0); // Count 21

  infoParm_t* infoParams = reinterpret_cast<infoParm_t*>(0x140425E30); // Count 0x1E

  GfxScene* scene = reinterpret_cast<GfxScene*>(0x148C26480);

  int* g_console_field_width = reinterpret_cast<int*>(0x14041E028);
  float* g_console_char_height = reinterpret_cast<float*>(0x14041E028);

  sharedUiInfo_t* sharedUiInfo = reinterpret_cast<sharedUiInfo_t*>(0x1465CE8C0);
  ScreenPlacement* scrPlaceFull = reinterpret_cast<ScreenPlacement*>(0x1406CBC50);
  ScreenPlacement* scrPlaceFullUnsafe = reinterpret_cast<ScreenPlacement*>(0x1406CBBE0);
  ScreenPlacement* scrPlaceView = reinterpret_cast<ScreenPlacement*>(0x1406CBCC0);

  PlayerKeyState* playerKeys = reinterpret_cast<PlayerKeyState*>(0x1406C4F20);
  kbutton_t* playersKb = reinterpret_cast<kbutton_t*>(0x1406CBDF0);
  AimAssistGlobals* aaGlobArray = reinterpret_cast<AimAssistGlobals*>(0x14042F3D0);

  keyname_t* keyNames = reinterpret_cast<keyname_t*>(0x14041E090);
  keyname_t* localizedKeyNames = reinterpret_cast<keyname_t*>(0x14041E690);

  GraphFloat* aaInputGraph = reinterpret_cast<GraphFloat*>(0x140430230);

  const char* MY_CMDS = reinterpret_cast<const char*>(0x14037E2FC); // Count 5

  XModel** cached_models = reinterpret_cast<XModel**>(0x1419F0E40);

  float (*CorrectSolidDeltas)[26][3] = reinterpret_cast<float(*)[26][3]>(0x140369690); // Count 26

  level_locals_t* level = reinterpret_cast<level_locals_t*>(0x141864EA0);

  float (*penetrationDepthTable)[PENETRATE_TYPE_COUNT][SURF_TYPE_COUNT] = reinterpret_cast<float(*)[PENETRATE_TYPE_COUNT][SURF_TYPE_COUNT]>(0x140444930);

  char (*com_consoleLines)[32] = reinterpret_cast<char(*)[32]>(0x141BD8A20);

  WinMouseVars_t* s_wmv = reinterpret_cast<WinMouseVars_t*>(0x146784DD8);

  int* window_center_x = reinterpret_cast<int*>(0x146784DE8);
  int* window_center_y = reinterpret_cast<int*>(0x146784DE8);

  DeferredQueue* deferredQueue = reinterpret_cast<DeferredQueue*>(0x141DEB460);

  int* g_waitingForKey = reinterpret_cast<int*>(0x14657E520);

  Game::cgMedia_t* cgMedia = reinterpret_cast<Game::cgMedia_t*>(0x140469210);

  char* partyTeamCounts = reinterpret_cast<char*>(0x140D0E260);

  int* dobjFreeCount = reinterpret_cast<int*>(0x141BDE454);

  unsigned long* g_dwTlsIndex = reinterpret_cast<unsigned long*>(0x148C24BD8);

  bgs_t* level_bgs = reinterpret_cast<bgs_t*>(0x1417CBDC0);

  unsigned int* playerCardUIStringIndex = reinterpret_cast<unsigned int*>(0x146523C44);
  char (*playerCardUIStringBuf)[PLAYER_CARD_UI_STRING_COUNT][38] = reinterpret_cast<char(*)[PLAYER_CARD_UI_STRING_COUNT][38]>(0x146523C50);

  uiInfo_s* uiInfoArray = reinterpret_cast<uiInfo_s*>(0x14663E870);

  GamerSettingState* gamerSettings = reinterpret_cast<GamerSettingState*>(0x1406CAB40);

  unsigned char** g_largeLocalBuf = reinterpret_cast<unsigned char**>(0x146653980);
  int* g_largeLocalPos = reinterpret_cast<int*>(0x146651954);
  int* g_largeLocalRightPos = reinterpret_cast<int*>(0x146653974);

  char** ui_arenaInfos = reinterpret_cast<char**>(0x1465BE5C0);
  int* ui_numArenas = reinterpret_cast<int*>(0x1465BE5B0);
  int* ui_arenaBufPos = reinterpret_cast<int*>(0x1465BE5B4);

  bool* s_havePlaylists = reinterpret_cast<bool*>(0x141BDDCE4);

  huffman_t* msgHuff = reinterpret_cast<huffman_t*>(0x141DC91F0);

  FxMarksSystem* fx_marksSystemPool = reinterpret_cast<FxMarksSystem*>(0x141658C00);

  DObjGetBoneIndex_t DObjGetBoneIndex = reinterpret_cast<DObjGetBoneIndex_t>(0x1402AA690);
  Com_GetClientDObj_t Com_GetClientDObj = reinterpret_cast<Com_GetClientDObj_t>(0x1401FB900);
  CG_WeaponDObjHandle_t CG_WeaponDObjHandle = reinterpret_cast<CG_WeaponDObjHandle_t>(0x1400C8110);
  CG_StopBoltedEffect_t CG_StopBoltedEffect = reinterpret_cast<CG_StopBoltedEffect_t>(0x1400AF450);
  FX_ImpactMarkCallback_t FX_ImpactMarkCallback = reinterpret_cast<FX_ImpactMarkCallback_t>(0x140152570);

  const char* TableLookup(StringTable* stringtable, int row, int column)
  {
    if (!stringtable || !stringtable->values || row >= stringtable->rowCount || column >= stringtable->columnCount) return "";

    const char* value = stringtable->values[row * stringtable->columnCount + column].string;
    if (!value) value = "";

    return value;
  }

  const char* UI_LocalizeGameType(const char* gameType)
  {
    if (!gameType || !*gameType)
    {
      return "";
    }

    for (int i = 0; i < *gameTypeCount; ++i)
    {
      if (!_stricmp(gameTypes[i].gameType, gameType))
      {
        return SEH_StringEd_GetString(gameTypes[i].uiName);
      }
    }

    return gameType;
  }

  // this cant be MessageBox because windows.h has a define that converts it to MessageBoxW. which is just stupid
  void ShowMessageBox(const std::string& message, const std::string& title)
  {
    if (!CL_IsCgameInitialized())
    {
      Dvar_SetStringByName("com_errorMessage", message.data());
      Dvar_SetStringByName("com_errorTitle", title.data());
      Cbuf_AddText(0, "openmenu error_popmenu_lobby\n");
    }
  }

  unsigned int R_HashString(const char* string)
  {
    unsigned int hash = 0;

    while (*string)
    {
      hash = (*string | 0x20) ^ (33 * hash);
      ++string;
    }

    return hash;
  }

  unsigned int R_HashString(const char* string, size_t maxLen)
  {
    unsigned int hash = 0;

    while (*string && maxLen > 0)
    {
      hash = (*string | 0x20) ^ (33 * hash);
      ++string;
      maxLen--;
    }

    return hash;
  }

  void Load_IndexBuffer(void* data, IDirect3DIndexBuffer9** storeHere, int count)
  {
    if (Dvar_FindVar("r_loadForRenderer")->current.enabled)
    {
      void* buffer = R_AllocStaticIndexBuffer(storeHere, 2 * count);
      std::memcpy(buffer, data, 2 * count);

      if (storeHere && *storeHere)
      {
        (*storeHere)->Unlock();
      }
    }
  }

  void Vec2UnpackTexCoords(const PackedTexCoords in, vec2_t* out)
  {
    unsigned int v3; // xmm1_4

    if (LOWORD(in.packed))
      v3 = ((in.packed & 0x8000) << 16) | (((((in.packed & 0x3FFF) << 14) - (~(LOWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000001) >> 1);
    else
      v3 = 0;

    (*out)[0] = std::bit_cast<float>(v3);

    if (HIWORD(in.packed))
      v3 = ((HIWORD(in.packed) & 0x8000) << 16) | (((((HIWORD(in.packed) & 0x3FFF) << 14)
        - (~(HIWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000001) >> 1);
    else
      v3 = 0;

    (*out)[1] = std::bit_cast<float>(v3);
  }

  void MatrixVecMultiply(const float (& mulMat)[3][3], const vec3_t& mulVec, vec3_t& solution)
  {
    vec3_t res;
    res[0] = mulMat[0][0] * mulVec[0] + mulMat[1][0] * mulVec[1] + mulMat[2][0] * mulVec[2];
    res[1] = mulMat[0][1] * mulVec[0] + mulMat[1][1] * mulVec[1] + mulMat[2][1] * mulVec[2];
    res[2] = mulMat[0][2] * mulVec[0] + mulMat[1][2] * mulVec[1] + mulMat[2][2] * mulVec[2];
    std::memmove(&solution[0], &res[0], sizeof(res));
  }

  void QuatRot(vec3_t* vec, const vec4_t* quat)
  {
    vec4_t q{ (*quat)[3],(*quat)[0],(*quat)[1],(*quat)[2] };

    vec4_t res{ 0, (*vec)[0], (*vec)[1], (*vec)[2] };
    vec4_t res2;
    vec4_t quat_conj{ q[0], -q[1], -q[2], -q[3] };
    QuatMultiply(&q, &res, &res2);
    QuatMultiply(&res2, &quat_conj, &res);

    (*vec)[0] = res[1];
    (*vec)[1] = res[2];
    (*vec)[2] = res[3];
  }

  void QuatMultiply(const vec4_t* q1, const vec4_t* q2, vec4_t* res)
  {
    (*res)[0] = (*q2)[0] * (*q1)[0] - (*q2)[1] * (*q1)[1] - (*q2)[2] * (*q1)[2] - (*q2)[3] * (*q1)[3];
    (*res)[1] = (*q2)[0] * (*q1)[1] + (*q2)[1] * (*q1)[0] - (*q2)[2] * (*q1)[3] + (*q2)[3] * (*q1)[2];
    (*res)[2] = (*q2)[0] * (*q1)[2] + (*q2)[1] * (*q1)[3] + (*q2)[2] * (*q1)[0] - (*q2)[3] * (*q1)[1];
    (*res)[3] = (*q2)[0] * (*q1)[3] - (*q2)[1] * (*q1)[2] + (*q2)[2] * (*q1)[1] + (*q2)[3] * (*q1)[0];
  }

  void SortWorldSurfaces(GfxWorld* world)
  {
    DWORD* specular1 = reinterpret_cast<DWORD*>(0x1493CFDB4);
    DWORD* specular2 = reinterpret_cast<DWORD*>(0x1493CFDB0);
    DWORD saveSpecular1 = *specular1;
    DWORD saveSpecular2 = *specular2;

    GfxWorld** gameWorld = reinterpret_cast<GfxWorld**>(0x148CCA620);
    GfxWorld* saveWorld = *gameWorld;

    *specular1 = 1;
    *specular2 = 1;
    *gameWorld = world;

    R_SortWorldSurfaces();

    *gameWorld = saveWorld;
    *specular1 = saveSpecular1;
    *specular2 = saveSpecular2;
  }

  float GraphGetValueFromFraction(const int knotCount, const float(*knots)[2], const float fraction)
  {
    for (auto knotIndex = 1; knotIndex < knotCount; ++knotIndex)
    {
      if (knots[knotIndex][0] >= fraction)
      {
        const auto adjustedFraction = (fraction - knots[knotIndex - 1][0]) / (knots[knotIndex][0] - knots[knotIndex - 1][0]);

        return (knots[knotIndex][1] - knots[knotIndex - 1][1]) * adjustedFraction + knots[knotIndex - 1][1];
      }
    }

    return -1.0f;
  }

  float GraphFloat_GetValue(const GraphFloat* graph, const float fraction)
  {
    return GraphGetValueFromFraction(graph->knotCount, graph->knots, fraction) * graph->scale;
  }

  ScreenPlacement* ScrPlace_GetFullPlacement()
  {
    return scrPlaceFull;
  }

  ScreenPlacement* ScrPlace_GetUnsafeFullPlacement()
  {
    return scrPlaceFullUnsafe;
  }

  void UI_FilterStringForButtonAnimation(char* str, unsigned int strMaxSize)
  {
    if (SEH_GetCurrentLanguage() == 8)
    {
      return;
    }

    const auto remainder = Sys_Milliseconds() % 1000;
    if (remainder <= 800)
    {
      return;
    }

    for (std::size_t idx = 0; str[idx] && idx < strMaxSize; ++idx)
    {
      if (str[idx] == 16)
      {
        str[idx] = -68;
      }
      else if (str[idx] == 17)
      {
        str[idx] = -67;
      }
    }
  }

  void I_strncpyz_s(char* dest, std::size_t destsize, const char* src, std::size_t count)
  {
    if (!destsize && !dest)
    {
      return;
    }
    if (!src || !count)
    {
      *dest = '\0';
    }
    else
    {
      const auto* p = reinterpret_cast<const unsigned char*>(src - 1);
      auto* q = reinterpret_cast<unsigned char*>(dest - 1);
      auto n = count + 1;
      auto s = count;
      if (destsize <= count)
      {
        n = destsize + 1;
        s = destsize - 1;
      }
      do
      {
        if (!--n)
        {
          dest[s] = '\0';
          return;
        }
        *++q = *++p;
      } while (*q);
    }
  }

  void I_strcpy(char* dest, std::size_t destsize, const char* src)
  {
    I_strncpyz_s(dest, destsize, src, destsize);
  }
}
