#include "STDInclude.hpp"

namespace Game
{
	// C-Style casts are fine here, that's where we're doing our dirty stuff anyways...
	BG_LoadWeaponDef_LoadObj_t BG_LoadWeaponDef_LoadObj = (BG_LoadWeaponDef_LoadObj_t)0x57B5F0;

	Cbuf_AddServerText_t Cbuf_AddServerText = (Cbuf_AddServerText_t)0x4BB9B0;
	Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x404B20;

	CG_GetClientNum_t CG_GetClientNum = (CG_GetClientNum_t)0x433700;

	CL_GetClientName_t CL_GetClientName = (CL_GetClientName_t)0x4563D0;
	CL_IsCgameInitialized_t CL_IsCgameInitialized = (CL_IsCgameInitialized_t)0x43EB20;
	CL_ConnectFromParty_t CL_ConnectFromParty = (CL_ConnectFromParty_t)0x433D30;
	CL_DownloadsComplete_t CL_DownloadsComplete = (CL_DownloadsComplete_t)0x42CE90;
	CL_DrawStretchPicPhysical_t CL_DrawStretchPicPhysical = (CL_DrawStretchPicPhysical_t)0x4FC120;
	CL_HandleRelayPacket_t CL_HandleRelayPacket = (CL_HandleRelayPacket_t)0x5A8C70;
	CL_ResetViewport_t CL_ResetViewport = (CL_ResetViewport_t)0x4A8830;
	CL_SelectStringTableEntryInDvar_f_t CL_SelectStringTableEntryInDvar_f = (CL_SelectStringTableEntryInDvar_f_t)0x4A4560;

	Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x470090;
	Cmd_AddServerCommand_t Cmd_AddServerCommand = (Cmd_AddServerCommand_t)0x4DCE00;
	Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = (Cmd_ExecuteSingleCommand_t)0x609540;
	Com_ClientPacketEvent_t Com_ClientPacketEvent = (Com_ClientPacketEvent_t)0x49F0B0;

	Com_Error_t Com_Error = (Com_Error_t)0x4B22D0;
	Com_Printf_t Com_Printf = (Com_Printf_t)0x402500;
	Com_PrintMessage_t Com_PrintMessage = (Com_PrintMessage_t)0x4AA830;
	Com_ParseExt_t Com_ParseExt = (Com_ParseExt_t)0x474D60;

	Con_DrawMiniConsole_t Con_DrawMiniConsole = (Con_DrawMiniConsole_t)0x464F30;
	Con_DrawSolidConsole_t Con_DrawSolidConsole = (Con_DrawSolidConsole_t)0x5A5040;

	DB_AllocStreamPos_t DB_AllocStreamPos = (DB_AllocStreamPos_t)0x418380;
	DB_PushStreamPos_t DB_PushStreamPos = (DB_PushStreamPos_t)0x458A20;
	DB_PopStreamPos_t DB_PopStreamPos = (DB_PopStreamPos_t)0x4D1D60;

	DB_BeginRecoverLostDevice_t DB_BeginRecoverLostDevice = (DB_BeginRecoverLostDevice_t)0x4BFF90;
	DB_EndRecoverLostDevice_t DB_EndRecoverLostDevice = (DB_EndRecoverLostDevice_t)0x46B660;
	DB_EnumXAssets_t DB_EnumXAssets = (DB_EnumXAssets_t)0x4B76D0;
	DB_EnumXAssets_Internal_t DB_EnumXAssets_Internal = (DB_EnumXAssets_Internal_t)0x5BB0A0;
	DB_FindXAssetHeader_t DB_FindXAssetHeader = (DB_FindXAssetHeader_t)0x407930;
	DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = (DB_GetXAssetNameHandler_t*)0x799328;
	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = (DB_GetXAssetSizeHandler_t*)0x799488;
	DB_GetXAssetTypeName_t DB_GetXAssetTypeName = (DB_GetXAssetTypeName_t)0x4CFCF0;
	DB_IsXAssetDefault_t DB_IsXAssetDefault = (DB_IsXAssetDefault_t)0x48E6A0;
	DB_LoadXAssets_t DB_LoadXAssets = (DB_LoadXAssets_t)0x4E5930;
	DB_LoadXFileData_t DB_LoadXFileData = (DB_LoadXFileData_t)0x445460;
	DB_ReadXFileUncompressed_t DB_ReadXFileUncompressed = (DB_ReadXFileUncompressed_t)0x4705E0;
	DB_ReleaseXAssetHandler_t* DB_ReleaseXAssetHandlers = (DB_ReleaseXAssetHandler_t*)0x799AB8;
	DB_XModelSurfsFixup_t DB_XModelSurfsFixup = (DB_XModelSurfsFixup_t)0x5BAC50;

	Dvar_RegisterBool_t Dvar_RegisterBool = (Dvar_RegisterBool_t)0x4CE1A0;
	Dvar_RegisterFloat_t Dvar_RegisterFloat = (Dvar_RegisterFloat_t)0x648440;
	Dvar_RegisterVec2_t Dvar_RegisterVec2 = (Dvar_RegisterVec2_t)0x4F6070;
	Dvar_RegisterVec3_t Dvar_RegisterVec3 = (Dvar_RegisterVec3_t)0x4EF8E0;
	Dvar_RegisterVec4_t Dvar_RegisterVec4 = (Dvar_RegisterVec4_t)0x4F28E0;
	Dvar_RegisterInt_t Dvar_RegisterInt = (Dvar_RegisterInt_t)0x479830;
	Dvar_RegisterEnum_t Dvar_RegisterEnum = (Dvar_RegisterEnum_t)0x412E40;
	Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x4FC7E0;
	Dvar_RegisterColor_t Dvar_RegisterColor = (Dvar_RegisterColor_t)0x4F28E0;//0x471500;

	Dvar_GetUnpackedColorByName_t Dvar_GetUnpackedColorByName = (Dvar_GetUnpackedColorByName_t)0x406530;
	Dvar_FindVar_t Dvar_FindVar = (Dvar_FindVar_t)0x4D5390;
	Dvar_InfoString_Big_t Dvar_InfoString_Big = (Dvar_InfoString_Big_t)0x4D98A0;
	Dvar_SetCommand_t Dvar_SetCommand = (Dvar_SetCommand_t)0x4EE430;

	Encode_Init_t Encode_Init = (Encode_Init_t)0x462AB0;

	Field_Clear_t Field_Clear = (Field_Clear_t)0x437EB0;

	FreeMemory_t FreeMemory = (FreeMemory_t)0x4D6640;

	FS_FileExists_t FS_FileExists = (FS_FileExists_t)0x4DEFA0;
	FS_FreeFile_t FS_FreeFile = (FS_FreeFile_t)0x4416B0;
	FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x4F4B90;
	FS_GetFileList_t FS_GetFileList = (FS_GetFileList_t)0x441BB0;
	FS_FreeFileList_t FS_FreeFileList = (FS_FreeFileList_t)0x4A5DE0;
	FS_FOpenFileAppend_t FS_FOpenFileAppend = (FS_FOpenFileAppend_t)0x410BB0;
	FS_FOpenFileAppend_t FS_FOpenFileWrite = (FS_FOpenFileAppend_t)0x4BA530;
	FS_FOpenFileRead_t FS_FOpenFileRead = (FS_FOpenFileRead_t)0x46CBF0;
	FS_FOpenFileReadForThread_t FS_FOpenFileReadForThread = (FS_FOpenFileReadForThread_t)0x643270;
	FS_FCloseFile_t FS_FCloseFile = (FS_FCloseFile_t)0x462000;
	FS_WriteFile_t FS_WriteFile = (FS_WriteFile_t)0x426450;
	FS_Write_t FS_Write = (FS_Write_t)0x4C06E0;
	FS_Read_t FS_Read = (FS_Read_t)0x4A04C0;
	FS_Seek_t FS_Seek = (FS_Seek_t)0x4A63D0;
	FS_FTell_t FS_FTell = (FS_FTell_t)0x4E6760;
	FS_Remove_t FS_Remove = (FS_Remove_t)0x4660F0;
	FS_Restart_t FS_Restart = (FS_Restart_t)0x461A50;
	FS_BuildPathToFile_t FS_BuildPathToFile = (FS_BuildPathToFile_t)0x4702C0;

	G_SpawnEntitiesFromString_t G_SpawnEntitiesFromString = (G_SpawnEntitiesFromString_t)0x4D8840;

	GScr_LoadGameTypeScript_t GScr_LoadGameTypeScript = (GScr_LoadGameTypeScript_t)0x4ED9A0;

	Image_LoadFromFileWithReader_t Image_LoadFromFileWithReader = (Image_LoadFromFileWithReader_t)0x53ABF0;
	Image_Release_t Image_Release = (Image_Release_t)0x51F010;

	LargeLocalInit_t LargeLocalInit = (LargeLocalInit_t)0x4A62A0;

	Load_Stream_t Load_Stream = (Load_Stream_t)0x470E30;
	Load_XString_t Load_XString = (Load_XString_t)0x47FDA0;
	Load_XModelPtr_t Load_XModelPtr = (Load_XModelPtr_t)0x4FCA70;
	Load_XModelSurfsFixup_t Load_XModelSurfsFixup = (Load_XModelSurfsFixup_t)0x40D7A0;
	Load_XStringArray_t Load_XStringArray = (Load_XStringArray_t)0x4977F0;
	Load_XStringCustom_t Load_XStringCustom = (Load_XStringCustom_t)0x4E0DD0;
	Load_FxEffectDefHandle_t Load_FxEffectDefHandle = (Load_FxEffectDefHandle_t)0x4D9B90;
	Load_FxElemDef_t Load_FxElemDef = (Load_FxElemDef_t)0x45AD90;
	Load_SndAliasCustom_t Load_SndAliasCustom = (Load_SndAliasCustom_t)0x49B6B0;
	Load_MaterialHandle_t Load_MaterialHandle = (Load_MaterialHandle_t)0x403960;
	Load_PhysCollmapPtr_t Load_PhysCollmapPtr = (Load_PhysCollmapPtr_t)0x47E990;
	Load_PhysPresetPtr_t Load_PhysPresetPtr = (Load_PhysPresetPtr_t)0x4FAD30;
	Load_TracerDefPtr_t Load_TracerDefPtr = (Load_TracerDefPtr_t)0x493090;
	Load_snd_alias_list_nameArray_t Load_snd_alias_list_nameArray = (Load_snd_alias_list_nameArray_t)0x4499F0;

	Menus_CloseAll_t Menus_CloseAll = (Menus_CloseAll_t)0x4BA5B0;
	Menus_OpenByName_t Menus_OpenByName = (Menus_OpenByName_t)0x4CCE60;
	Menus_FindByName_t Menus_FindByName = (Menus_FindByName_t)0x487240;
	Menu_IsVisible_t Menu_IsVisible = (Menu_IsVisible_t)0x4D77D0;
	Menus_MenuIsInStack_t Menus_MenuIsInStack = (Menus_MenuIsInStack_t)0x47ACB0;

	MSG_Init_t MSG_Init = (MSG_Init_t)0x45FCA0;
	MSG_ReadData_t MSG_ReadData = (MSG_ReadData_t)0x4527C0;
	MSG_ReadLong_t MSG_ReadLong = (MSG_ReadLong_t)0x4C9550;
	MSG_ReadShort_t MSG_ReadShort = (MSG_ReadShort_t)0x40BDD0;
	MSG_ReadInt64_t MSG_ReadInt64 = (MSG_ReadInt64_t)0x4F1850;
	MSG_ReadString_t MSG_ReadString = (MSG_ReadString_t)0x60E2B0;
	MSG_WriteByte_t MSG_WriteByte = (MSG_WriteByte_t)0x48C520;
	MSG_WriteData_t MSG_WriteData = (MSG_WriteData_t)0x4F4120;
	MSG_WriteLong_t MSG_WriteLong = (MSG_WriteLong_t)0x41CA20;
	MSG_WriteBitsCompress_t MSG_WriteBitsCompress = (MSG_WriteBitsCompress_t)0x4319D0;
	MSG_ReadByte_t MSG_ReadByte = (MSG_ReadByte_t)0x4C1C20;
	MSG_ReadBitsCompress_t MSG_ReadBitsCompress = (MSG_ReadBitsCompress_t)0x4DCC30;

	NetadrToSockadr_t NetadrToSockadr = (NetadrToSockadr_t)0x4B4B40;

	NET_AdrToString_t NET_AdrToString = (NET_AdrToString_t)0x469880;
	NET_CompareAdr_t NET_CompareAdr = (NET_CompareAdr_t)0x4D0AA0;
	NET_Init_t NET_Init = (NET_Init_t)0x491860;
	NET_IsLocalAddress_t NET_IsLocalAddress = (NET_IsLocalAddress_t)0x402BD0;
	NET_StringToAdr_t NET_StringToAdr = (NET_StringToAdr_t)0x409010;
	NET_OutOfBandPrint_t NET_OutOfBandPrint = (NET_OutOfBandPrint_t)0x4AEF00;
	NET_OutOfBandData_t NET_OutOfBandData = (NET_OutOfBandData_t)0x49C7E0;

	Live_MPAcceptInvite_t Live_MPAcceptInvite = (Live_MPAcceptInvite_t)0x420A6D;
	Live_ParsePlaylists_t Live_ParsePlaylists = (Live_ParsePlaylists_t)0x4295A0;

	LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x61ABC0;

	PC_ReadToken_t PC_ReadToken = (PC_ReadToken_t)0x4ACCD0;
	PC_ReadTokenHandle_t PC_ReadTokenHandle = (PC_ReadTokenHandle_t)0x4D2060;
	PC_SourceError_t PC_SourceError = (PC_SourceError_t)0x467A00;

	Party_GetMaxPlayers_t Party_GetMaxPlayers = (Party_GetMaxPlayers_t)0x4F5D60;
	PartyHost_CountMembers_t PartyHost_CountMembers = (PartyHost_CountMembers_t)0x497330;
	PartyHost_GetMemberAddressBySlot_t PartyHost_GetMemberAddressBySlot = (PartyHost_GetMemberAddressBySlot_t)0x44E100;
	PartyHost_GetMemberName_t PartyHost_GetMemberName = (PartyHost_GetMemberName_t)0x44BE90;

	R_AddCmdDrawStretchPic_t R_AddCmdDrawStretchPic = (R_AddCmdDrawStretchPic_t)0x509770;
	R_AllocStaticIndexBuffer_t R_AllocStaticIndexBuffer = (R_AllocStaticIndexBuffer_t)0x51E7A0;
	R_Cinematic_StartPlayback_Now_t R_Cinematic_StartPlayback_Now = (R_Cinematic_StartPlayback_Now_t)0x51C5B0;
	R_RegisterFont_t R_RegisterFont = (R_RegisterFont_t)0x505670;
	R_AddCmdDrawText_t R_AddCmdDrawText = (R_AddCmdDrawText_t)0x509D80;
	R_LoadGraphicsAssets_t R_LoadGraphicsAssets = (R_LoadGraphicsAssets_t)0x506AC0;
	R_TextWidth_t R_TextWidth = (R_TextWidth_t)0x5056C0;
	R_TextHeight_t R_TextHeight = (R_TextHeight_t)0x505770;
	
	Scr_LoadGameType_t Scr_LoadGameType = (Scr_LoadGameType_t)0x4D9520;

	Scr_LoadScript_t Scr_LoadScript = (Scr_LoadScript_t)0x45D940;
	Scr_GetFunctionHandle_t Scr_GetFunctionHandle = (Scr_GetFunctionHandle_t)0x4234F0;

	Scr_ExecThread_t Scr_ExecThread = (Scr_ExecThread_t)0x4AD0B0;
	Scr_FreeThread_t Scr_FreeThread = (Scr_FreeThread_t)0x4BD320;

	Scr_ShutdownAllocNode_t Scr_ShutdownAllocNode = (Scr_ShutdownAllocNode_t)0x441650;

	Script_Alloc_t Script_Alloc = (Script_Alloc_t)0x422E70;
	Script_SetupTokens_t Script_SetupTokens = (Script_SetupTokens_t)0x4E6950;
	Script_CleanString_t Script_CleanString = (Script_CleanString_t)0x498220;

	SE_Load_t SE_Load = (SE_Load_t)0x502A30;

	SEH_StringEd_GetString_t SEH_StringEd_GetString = (SEH_StringEd_GetString_t)0x44BB30;

	Dvar_SetStringByName_t Dvar_SetStringByName = (Dvar_SetStringByName_t)0x44F060;

	SL_ConvertToString_t SL_ConvertToString = (SL_ConvertToString_t)0x4EC1D0;
	SL_GetString_t SL_GetString = (SL_GetString_t)0x4CDC10;

	SND_Init_t SND_Init = (SND_Init_t)0x46A630;
	SND_InitDriver_t SND_InitDriver = (SND_InitDriver_t)0x4F5090;

	SockadrToNetadr_t SockadrToNetadr = (SockadrToNetadr_t)0x4F8460;

	Steam_JoinLobby_t Steam_JoinLobby = (Steam_JoinLobby_t)0x49CF70;

	SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x469AC0;
	SV_GameSendServerCommand_t SV_GameSendServerCommand = (SV_GameSendServerCommand_t)0x4BC3A0;
	SV_Cmd_TokenizeString_t SV_Cmd_TokenizeString = (SV_Cmd_TokenizeString_t)0x4B5780;
	SV_Cmd_EndTokenizedString_t SV_Cmd_EndTokenizedString = (SV_Cmd_EndTokenizedString_t)0x464750;
	SV_DirectConnect_t SV_DirectConnect = (SV_DirectConnect_t)0x460480;

	Sys_Error_t Sys_Error = (Sys_Error_t)0x4E0200;
	Sys_FreeFileList_t Sys_FreeFileList = (Sys_FreeFileList_t)0x4D8580;
	Sys_IsDatabaseReady_t Sys_IsDatabaseReady = (Sys_IsDatabaseReady_t)0x4CA4A0;
	Sys_IsDatabaseReady2_t Sys_IsDatabaseReady2 = (Sys_IsDatabaseReady2_t)0x441280;
	Sys_IsMainThread_t Sys_IsMainThread = (Sys_IsMainThread_t)0x4C37D0;
	Sys_SendPacket_t Sys_SendPacket = (Sys_SendPacket_t)0x60FDC0;
	Sys_ShowConsole_t Sys_ShowConsole = (Sys_ShowConsole_t)0x4305E0;
	Sys_ListFiles_t Sys_ListFiles = (Sys_ListFiles_t)0x45A660;
	Sys_Milliseconds_t Sys_Milliseconds = (Sys_Milliseconds_t)0x42A660;

	UI_AddMenuList_t UI_AddMenuList = (UI_AddMenuList_t)0x4533C0;
	UI_CheckStringTranslation_t UI_CheckStringTranslation = (UI_CheckStringTranslation_t)0x4FB010;
	UI_LoadMenus_t UI_LoadMenus = (UI_LoadMenus_t)0x641460;
	UI_DrawHandlePic_t UI_DrawHandlePic = (UI_DrawHandlePic_t)0x4D0EA0;
	UI_GetContext_t UI_GetContext = (UI_GetContext_t)0x4F8940;
	UI_TextWidth_t UI_TextWidth = (UI_TextWidth_t)0x6315C0;
	UI_DrawText_t UI_DrawText = (UI_DrawText_t)0x49C0D0;

	Win_GetLanguage_t Win_GetLanguage = (Win_GetLanguage_t)0x45CBA0;

	XAssetHeader* DB_XAssetPool = (XAssetHeader*)0x7998A8;
	unsigned int* g_poolSize = (unsigned int*)0x7995E8;

	DWORD* cmd_id = (DWORD*)0x1AAC5D0;
	DWORD* cmd_argc = (DWORD*)0x1AAC614;
	char*** cmd_argv = (char***)0x1AAC634;

	DWORD* cmd_id_sv = (DWORD*)0x1ACF8A0;
	DWORD* cmd_argc_sv = (DWORD*)0x1ACF8E4;
	char*** cmd_argv_sv = (char***)0x1ACF904;

	cmd_function_t** cmd_functions = (cmd_function_t**)0x1AAC658;

	source_t **sourceFiles = (source_t **)0x7C4A98;
	keywordHash_t **menuParseKeywordHash = (keywordHash_t **)0x63AE928;

	int* svs_numclients = (int*)0x31D938C;
	client_t* svs_clients = (client_t*)0x31D9390;

	UiContext *uiContext = (UiContext *)0x62E2858;

	int* arenaCount = (int*)0x62E6930;
	mapArena_t* arenas = (mapArena_t*)0x62E6934;

	int* gameTypeCount = (int*)0x62E50A0;
	gameTypeName_t* gameTypes = (gameTypeName_t*)0x62E50A4;

	searchpath_t* fs_searchpaths = (searchpath_t*)0x63D96E0;

	XBlock** g_streamBlocks = (XBlock**)0x16E554C;

	bool* g_lobbyCreateInProgress = (bool*)0x66C9BC2;
	party_t** partyIngame = (party_t**)0x1081C00;
	PartyData_s** partyData = (PartyData_s**)0x107E500;

	int* numIP = (int*)0x64A1E68;
	netIP_t* localIP = (netIP_t*)0x64A1E28;

	int* demoFile = (int*)0xA5EA1C;
	int* demoPlaying = (int*)0xA5EA0C;
	int* demoRecording = (int*)0xA5EA08;
	int* serverMessageSequence = (int*)0xA3E9B4;

	gentity_t* g_entities = (gentity_t*)0x18835D8;

	netadr_t* connectedHost = (netadr_t*)0xA1E888;

	SOCKET* ip_socket = (SOCKET*)0x64A3008;

	SafeArea* safeArea = (SafeArea*)0xA15F3C;

	SpawnVar* spawnVars = (SpawnVar*)0x1A83DE8;
	MapEnts** marMapEntsPtr = (MapEnts**)0x112AD34;

	IDirect3D9** d3d9 = (IDirect3D9**)0x66DEF84;
	IDirect3DDevice9** dx_ptr = (IDirect3DDevice9**)0x66DEF88;

	mapname_t* mapnames = (mapname_t*)0x7471D0;

	char*** varXString = (char***)0x112B340;
	TracerDef*** varTracerDefPtr = (TracerDef***)0x112B3BC;
	XModel*** varXModelPtr = (XModel***)0x112A934;
	XModel** varXModel = (XModel**)0x112AE14;
	PathData** varPathData = (PathData**)0x112AD7C;
	const char** varConstChar = (const char**)0x112A774;
	Material*** varMaterialHandle = (Material***)0x112A878;
	FxEffectDef*** varFxEffectDefHandle = (FxEffectDef***)0x112ACC0;
	PhysCollmap*** varPhysCollmapPtr = (PhysCollmap***)0x112B440;
	PhysPreset*** varPhysPresetPtr = (PhysPreset***)0x112B378;
	Game::MaterialPass** varMaterialPass = (Game::MaterialPass**)0x112A960;
	snd_alias_list_t*** varsnd_alias_list_name = (snd_alias_list_t***)0x112AF38;

	XAssetHeader ReallocateAssetPool(XAssetType type, unsigned int newSize)
	{
		int elSize = DB_GetXAssetSizeHandlers[type]();
		XAssetHeader poolEntry = { Utils::Memory::Allocate(newSize * elSize) };
		DB_XAssetPool[type] = poolEntry;
		g_poolSize[type] = newSize;
		return poolEntry;
	}

	void Menu_FreeItemMemory(Game::itemDef_t* item)
	{
		__asm
		{
			mov edi, item
			mov eax, 63D880h
			call eax
		}
	}

	const char* TableLookup(StringTable* stringtable, int row, int column)
	{
		if (!stringtable || !stringtable->values || row >= stringtable->rowCount || column >= stringtable->columnCount) return "";

		const char* value = stringtable->values[row * stringtable->columnCount + column].string;
		if (!value) value = "";

		return value;
	}

	const char* UI_LocalizeMapName(const char* mapName)
	{
		for (int i = 0; i < *arenaCount; ++i)
		{
			if (!_stricmp(arenas[i].mapName, mapName))
			{
				char* uiName = &arenas[i].uiName[0];
				if ((uiName[0] == 'M' && uiName[1] == 'P') || (uiName[0] == 'P' && uiName[1] == 'A')) // MPUI/PATCH
				{
					return SEH_StringEd_GetString(uiName);
				}

				return uiName;
			}
		}

		return mapName;
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

	float UI_GetScoreboardLeft(void* a1)
	{
		static int func = 0x590390;
		float result = 0;

		__asm
		{
			mov eax, a1
			call func
			mov result, eax
		}

		return result;
	}

	const char *DB_GetXAssetName(XAsset *asset)
	{
		if (!asset) return "";
		return DB_GetXAssetNameHandlers[asset->type](&asset->header);
	}

	XAssetType DB_GetXAssetNameType(const char* name)
	{
		for (int i = 0; i < ASSET_TYPE_COUNT; ++i)
		{
			XAssetType type = static_cast<XAssetType>(i);
			if (!_stricmp(DB_GetXAssetTypeName(type), name))
			{
				return type;
			}
		}

		return ASSET_TYPE_INVALID;
	}

	bool DB_IsZoneLoaded(const char* zone)
	{
		int zoneCount = Utils::Hook::Get<int>(0x1261BCC);
		char* zoneIndices = reinterpret_cast<char*>(0x16B8A34);
		char* zoneData = reinterpret_cast<char*>(0x14C0F80);

		for (int i = 0; i < zoneCount; ++i)
		{
			std::string name = zoneData + 4 + 0xA4 * (zoneIndices[i] & 0xFF);

			if (name == zone)
			{
				return true;
			}
		}

		return false;
	}

	void FS_AddLocalizedGameDirectory(const char *path, const char *dir)
	{
		__asm
		{
			mov ebx, path
			mov eax, dir
			mov ecx, 642EF0h
			call ecx
		}
	}

	void MessageBox(std::string message, std::string title)
	{
		Dvar_SetStringByName("com_errorMessage", message.data());
		Dvar_SetStringByName("com_errorTitle", title.data());
		Cbuf_AddText(0, "openmenu error_popmenu_lobby");
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

	void SV_KickClient(client_t* client, const char* reason)
	{
		__asm
		{
			push edi
			push esi
			mov edi, 0
			mov esi, client
			push reason
			push 0
			push 0
			mov eax, 6249A0h
			call eax
			add esp, 0Ch
			pop esi
			pop edi
		}
	}

	void SV_KickClientError(client_t* client, std::string reason)
	{
		if (client->state < 5)
		{
			Components::Network::Send(client->addr, fmt::sprintf("error\n%s", reason.data()));
		}

		SV_KickClient(client, reason.data());
	}

	void Scr_iPrintLn(int clientNum, std::string message)
	{
		Game::SV_GameSendServerCommand(clientNum, 0, Utils::String::VA("%c \"%s\"", 0x66, message.data()));
	}

	void Scr_iPrintLnBold(int clientNum, std::string message)
	{
		Game::SV_GameSendServerCommand(clientNum, 0, Utils::String::VA("%c \"%s\"", 0x67, message.data()));
	}

	void IN_KeyUp(kbutton_t* button)
	{
		__asm 
		{
			push esi
			mov esi, button
			mov eax, 5A5580h
			call eax
			pop esi
		}
	}

	void IN_KeyDown(kbutton_t* button)
	{
		__asm
		{
			push esi
			mov esi, button
			mov eax, 5A54E0h
			call eax
			pop esi
		}
	}

	void Load_IndexBuffer(void* data, IDirect3DIndexBuffer9** storeHere, int count)
	{
		if (Components::Dvar::Var("r_loadForRenderer").Get<bool>())
		{
			void* buffer = R_AllocStaticIndexBuffer(storeHere, 2 * count);
			std::memcpy(buffer, data, 2 * count);

			if (storeHere && *storeHere)
			{
				(*storeHere)->Unlock();
			}
		}
	}

	void Load_VertexBuffer(void* data, IDirect3DVertexBuffer9** where, int len)
	{
		__asm
		{
			push edi
			push ebx

			mov eax, len
			mov edi, where
			push data

			mov ebx, 5112C0h
			call ebx
			add esp, 4

			pop ebx
			pop edi
		}
	}
}
