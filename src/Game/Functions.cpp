#include "STDInclude.hpp"

namespace Game
{
	Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x404B20;

	CL_GetClientName_t CL_GetClientName = (CL_GetClientName_t)0x4563D0;
	CL_IsCgameInitialized_t CL_IsCgameInitialized = (CL_IsCgameInitialized_t)0x43EB20;
	CL_ConnectFromParty_t CL_ConnectFromParty = (CL_ConnectFromParty_t)0x433D30;

	Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x470090;
	Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = (Cmd_ExecuteSingleCommand_t)0x609540;

	Com_Error_t Com_Error = (Com_Error_t)0x4B22D0;
	Com_Printf_t Com_Printf = (Com_Printf_t)0x402500;
	Com_PrintMessage_t Com_PrintMessage = (Com_PrintMessage_t)0x4AA830;
	Com_Milliseconds_t Com_Milliseconds = (Com_Milliseconds_t)0x42A660;
	Com_ParseExt_t Com_ParseExt = (Com_ParseExt_t)0x474D60;

	DB_FindXAssetHeader_t DB_FindXAssetHeader = (DB_FindXAssetHeader_t)0x407930;
	DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = (DB_GetXAssetNameHandler_t*)0x799328;
	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = (DB_GetXAssetSizeHandler_t*)0x799488;
	DB_GetXAssetTypeName_t DB_GetXAssetTypeName = (DB_GetXAssetTypeName_t)0x4CFCF0;
	DB_LoadXAssets_t DB_LoadXAssets = (DB_LoadXAssets_t)0x4E5930;

	Dvar_RegisterBool_t Dvar_RegisterBool = (Dvar_RegisterBool_t)0x4CE1A0;
	Dvar_RegisterFloat_t Dvar_RegisterFloat = (Dvar_RegisterFloat_t)0x648440;
	Dvar_RegisterFloat2_t Dvar_RegisterFloat2 = (Dvar_RegisterFloat2_t)0x4F6070;
	Dvar_RegisterFloat3_t Dvar_RegisterFloat3 = (Dvar_RegisterFloat3_t)0x4EF8E0;
	Dvar_RegisterFloat4_t Dvar_RegisterFloat4 = (Dvar_RegisterFloat4_t)0x4F28E0;
	Dvar_RegisterInt_t Dvar_RegisterInt = (Dvar_RegisterInt_t)0x479830;
	Dvar_RegisterEnum_t Dvar_RegisterEnum = (Dvar_RegisterEnum_t)0x412E40;
	Dvar_RegisterString_t Dvar_RegisterString = (Dvar_RegisterString_t)0x4FC7E0;
	Dvar_RegisterColor_t Dvar_RegisterColor = (Dvar_RegisterColor_t)0x471500;

	Dvar_FindVar_t Dvar_FindVar = (Dvar_FindVar_t)0x4D5390;
	Dvar_InfoString_Big_t Dvar_InfoString_Big = (Dvar_InfoString_Big_t)0x4D98A0;
	Dvar_SetCommand_t Dvar_SetCommand = (Dvar_SetCommand_t)0x4EE430;

	Field_Clear_t Field_Clear = (Field_Clear_t)0x437EB0;

	FreeMemory_t FreeMemory = (FreeMemory_t)0x4D6640;

	FS_FileExists_t FS_FileExists = (FS_FileExists_t)0x4DEFA0;
	FS_FreeFile_t FS_FreeFile = (FS_FreeFile_t)0x4416B0;
	FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x4F4B90;
	FS_ListFiles_t FS_ListFiles = (FS_ListFiles_t)0x441BB0;
	FS_FreeFileList_t FS_FreeFileList = (FS_FreeFileList_t)0x4A5DE0;
	FS_FOpenFileAppend_t FS_FOpenFileAppend = (FS_FOpenFileAppend_t)0x410BB0;
	FS_FOpenFileAppend_t FS_FOpenFileWrite = (FS_FOpenFileAppend_t)0x4BA530;
	FS_FOpenFileRead_t FS_FOpenFileRead = (FS_FOpenFileRead_t)0x46CBF0;
	FS_FCloseFile_t FS_FCloseFile = (FS_FCloseFile_t)0x462000;
	FS_WriteFile_t FS_WriteFile = (FS_WriteFile_t)0x426450;
	FS_Write_t FS_Write = (FS_Write_t)0x4C06E0;
	FS_Read_t FS_Read = (FS_Read_t)0x4A04C0;
	FS_Seek_t FS_Seek = (FS_Seek_t)0x4A63D0;
	FS_FTell_t FS_FTell = (FS_FTell_t)0x4E6760;
	FS_Remove_t FS_Remove = (FS_Remove_t)0x4660F0;
	FS_Restart_t FS_Restart = (FS_Restart_t)0x461A50;
	FS_BuildPathToFile_t FS_BuildPathToFile = (FS_BuildPathToFile_t)0x4702C0;

	Menus_CloseAll_t Menus_CloseAll = (Menus_CloseAll_t)0x4BA5B0;
	Menus_OpenByName_t Menus_OpenByName = (Menus_OpenByName_t)0x4CCE60;

	NET_AdrToString_t NET_AdrToString = (NET_AdrToString_t)0x469880;
	NET_CompareAdr_t NET_CompareAdr = (NET_CompareAdr_t)0x4D0AA0;
	NET_IsLocalAddress_t NET_IsLocalAddress = (NET_IsLocalAddress_t)0x402BD0;
	NET_StringToAdr_t NET_StringToAdr = (NET_StringToAdr_t)0x409010;

	Live_MPAcceptInvite_t Live_MPAcceptInvite = (Live_MPAcceptInvite_t)0x420A6D;
	Live_ParsePlaylists_t Live_ParsePlaylists = (Live_ParsePlaylists_t)0x4295A0;

	LoadInitialFF_t LoadInitialFF = (LoadInitialFF_t)0x506AC0;
	LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x61ABC0;

	LocalizeString_t LocalizeString = (LocalizeString_t)0x4FB010;
	LocalizeMapString_t LocalizeMapString = (LocalizeMapString_t)0x44BB30;

	sendOOB_t OOBPrint = (sendOOB_t)0x4AEF00;
	sendOOBRaw_t OOBPrintRawData = (sendOOBRaw_t)0x60FDC0;

	SE_Load_t SE_Load = (SE_Load_t)0x502A30;

	PC_ReadToken_t PC_ReadToken = (PC_ReadToken_t)0x4ACCD0;
	PC_ReadTokenHandle_t PC_ReadTokenHandle = (PC_ReadTokenHandle_t)0x4D2060;
	PC_SourceError_t PC_SourceError = (PC_SourceError_t)0x467A00;

	Party_GetMaxPlayers_t Party_GetMaxPlayers = (Party_GetMaxPlayers_t)0x4F5D60;
	PartyHost_CountMembers_t PartyHost_CountMembers = (PartyHost_CountMembers_t)0x497330;
	PartyHost_GetMemberAddressBySlot_t PartyHost_GetMemberAddressBySlot = (PartyHost_GetMemberAddressBySlot_t)0x44E100;
	PartyHost_GetMemberName_t PartyHost_GetMemberName = (PartyHost_GetMemberName_t)0x44BE90;

	Script_Alloc_t Script_Alloc = (Script_Alloc_t)0x422E70;
	Script_SetupTokens_t Script_SetupTokens = (Script_SetupTokens_t)0x4E6950;
	Script_CleanString_t Script_CleanString = (Script_CleanString_t)0x498220;

	SetConsole_t SetConsole = (SetConsole_t)0x44F060;

	SL_ConvertToString_t SL_ConvertToString = (SL_ConvertToString_t)0x4EC1D0;

	Steam_JoinLobby_t Steam_JoinLobby = (Steam_JoinLobby_t)0x49CF70;

	SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x469AC0;

	Sys_IsMainThread_t Sys_IsMainThread = (Sys_IsMainThread_t)0x4C37D0;

	UI_AddMenuList_t UI_AddMenuList = (UI_AddMenuList_t)0x4533C0;
	UI_LoadMenus_t UI_LoadMenus = (UI_LoadMenus_t)0x641460;
	UI_DrawHandlePic_t UI_DrawHandlePic = (UI_DrawHandlePic_t)0x4D0EA0;

	Win_GetLanguage_t Win_GetLanguage = (Win_GetLanguage_t)0x45CBA0;

	void** DB_XAssetPool = (void**)0x7998A8;
	unsigned int* g_poolSize = (unsigned int*)0x7995E8;

	DWORD* cmd_id = (DWORD*)0x1AAC5D0;
	DWORD* cmd_argc = (DWORD*)0x1AAC614;
	char*** cmd_argv = (char***)0x1AAC634;

	source_t **sourceFiles = (source_t **)0x7C4A98;
	keywordHash_t **menuParseKeywordHash = (keywordHash_t **)0x63AE928;

	int* svs_numclients = (int*)0x31D938C;
	client_t* svs_clients = (client_t*)0x31D9390;

	UiContext *uiContext = (UiContext *)0x62E2858;

	int* arenaCount = (int*)0x62E6930;
	mapArena_t* arenas = (mapArena_t*)0x62E6934;

	int* gameTypeCount = (int*)0x62E50A0;
	gameTypeName_t* gameTypes = (gameTypeName_t*)0x62E50A4;

	XBlock** g_streamBlocks = (XBlock**)0x16E554C;

	bool* g_lobbyCreateInProgress = (bool*)0x66C9BC2;
	party_t** partyIngame = (party_t**)0x1081C00;
	PartyData_s** partyData = (PartyData_s**)0x107E500;

	int* numIP = (int*)0x64A1E68;
	netIP_t* localIP = (netIP_t*)0x64A1E28;

	void* ReallocateAssetPool(XAssetType type, unsigned int newSize)
	{
		int elSize = DB_GetXAssetSizeHandlers[type]();
		void* poolEntry = new char[newSize * elSize];
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

	const char* TabeLookup(StringTable* stringtable, int row, int column)
	{
		if (!stringtable || !stringtable->values || row >= stringtable->rowCount || column >= stringtable->columnCount) return "";

		const char* value = stringtable->values[row * stringtable->columnCount + column].string;
		if (!value) value = "";

		return value;
	}

	void OOBPrintT(int type, netadr_t netadr, const char* message)
	{
		int* adr = (int*)&netadr;

		OOBPrint(type, *adr, *(adr + 1), *(adr + 2), 0xFFFFFFFF, *(adr + 4), message);
	}

	void OOBPrintRaw(int type, netadr_t netadr, const char* message, size_t length)
	{
		int* adr = (int*)&netadr;

		OOBPrintRawData(type, length, message, *adr, *(adr + 1), *(adr + 2), 0xFFFFFFFF, *(adr + 4));
	}

	const char* UI_LocalizeMapName(const char* mapName)
	{
		for (int i = 0; i < *arenaCount; i++)
		{
			if (!_stricmp(arenas[i].mapName, mapName))
			{
				char* uiName = &arenas[i].uiName[0];
				if ((uiName[0] == 'M' && uiName[1] == 'P') || (uiName[0] == 'P' && uiName[1] == 'A')) // MPUI/PATCH
				{
					char* name = LocalizeMapString(uiName);
					return name;
				}

				return uiName;
			}
		}

		return mapName;
	}

	const char* UI_LocalizeGameType(const char* gameType)
	{
		if (gameType == 0 || *gameType == '\0')
		{
			return "";
		}

		for (int i = 0; i < *gameTypeCount; i++)
		{
			if (!_stricmp(gameTypes[i].gameType, gameType))
			{
				char* name = LocalizeMapString(gameTypes[i].uiName);
				return name;
			}
		}

		return gameType;
	}

	const char *DB_GetXAssetName(XAsset *asset)
	{
		if (!asset) return "";
		return DB_GetXAssetNameHandlers[asset->type](&asset->header);
	}

	XAssetType DB_GetXAssetNameType(const char* name)
	{
		for (int i = 0; i < ASSET_TYPE_COUNT; i++)
		{
			XAssetType type = (XAssetType)i;
			if (!_stricmp(DB_GetXAssetTypeName(type), name))
			{
				return type;
			}
		}

		return ASSET_TYPE_INVALID;
	}
}