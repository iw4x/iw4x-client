namespace Game
{
	typedef void(__cdecl * Cbuf_AddText_t)(int a1, const char* cmd);
	extern Cbuf_AddText_t Cbuf_AddText;

	typedef char*(__cdecl * CL_GetClientName_t)(int localClientNum, int index, char *buf, size_t size);
	extern CL_GetClientName_t CL_GetClientName;

	typedef int(__cdecl * CL_IsCgameInitialized_t)();
	extern CL_IsCgameInitialized_t CL_IsCgameInitialized;

	typedef void(__cdecl * CL_ConnectFromParty_t)(int controller, void*, netadr_t adr, int, int, const char*, const char*);
	extern CL_ConnectFromParty_t CL_ConnectFromParty;

	typedef void(__cdecl * Cmd_AddCommand_t)(const char* name, void(*callback), cmd_function_t* data, char);
	extern Cmd_AddCommand_t Cmd_AddCommand;

	typedef void(__cdecl * Cmd_ExecuteSingleCommand_t)(int controller, int a2, const char* cmd);
	extern Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand;

	typedef void(__cdecl * Com_Error_t)(int type, char* message, ...);
	extern Com_Error_t Com_Error;

	typedef void(__cdecl * Com_Printf_t)(int, const char*, ...);
	extern Com_Printf_t Com_Printf;

	typedef void(__cdecl * Com_PrintMessage_t)(int, const char*, char);
	extern Com_PrintMessage_t Com_PrintMessage;

	typedef int(__cdecl * Com_Milliseconds_t)(void);
	extern Com_Milliseconds_t Com_Milliseconds;

	typedef char* (__cdecl * Com_ParseExt_t)(const char**);
	extern Com_ParseExt_t Com_ParseExt;

	typedef XAssetHeader (__cdecl * DB_FindXAssetHeader_t)(XAssetType type, const char* filename);
	extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

	typedef const char* (__cdecl * DB_GetXAssetNameHandler_t)(Game::XAssetHeader* asset);
	extern DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers;

	typedef int(__cdecl * DB_GetXAssetSizeHandler_t)();
	extern DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers;

	typedef const char *(__cdecl * DB_GetXAssetTypeName_t)(XAssetType type);
	extern DB_GetXAssetTypeName_t DB_GetXAssetTypeName;

	typedef void(*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
	extern DB_LoadXAssets_t DB_LoadXAssets;

	typedef dvar_t* (__cdecl * Dvar_RegisterBool_t)(const char* name, bool default, int flags, const char* description);
	extern Dvar_RegisterBool_t Dvar_RegisterBool;

	typedef dvar_t* (__cdecl * Dvar_RegisterFloat_t)(const char* name, float default, float min, float max, int flags, const char* description);
	extern Dvar_RegisterFloat_t Dvar_RegisterFloat;

	typedef dvar_t* (__cdecl * Dvar_RegisterFloat2_t)(const char* name, float defx, float defy, float min, float max, int flags, const char* description);
	extern Dvar_RegisterFloat2_t Dvar_RegisterFloat2;

	typedef dvar_t* (__cdecl * Dvar_RegisterFloat3_t)(const char* name, float defx, float defy, float defz, float min, float max, int flags, const char* description);
	extern Dvar_RegisterFloat3_t Dvar_RegisterFloat3;

	typedef dvar_t* (__cdecl * Dvar_RegisterFloat4_t)(const char* name, float defx, float defy, float defz, float defw, float min, float max, int flags, const char* description);
	extern Dvar_RegisterFloat4_t Dvar_RegisterFloat4;

	typedef dvar_t* (__cdecl * Dvar_RegisterInt_t)(const char* name, int default, int min, int max, int flags, const char* description);
	extern Dvar_RegisterInt_t Dvar_RegisterInt;

	typedef dvar_t* (__cdecl * Dvar_RegisterEnum_t)(const char* name, char** enumValues, int default, int flags, const char* description);
	extern Dvar_RegisterEnum_t Dvar_RegisterEnum;

	typedef dvar_t* (__cdecl * Dvar_RegisterString_t)(const char* name, const char* default, int, const char*);
	extern Dvar_RegisterString_t Dvar_RegisterString;

	typedef dvar_t* (__cdecl * Dvar_RegisterColor_t)(const char* name, float r, float g, float b, float a, int flags, const char* description);
	extern Dvar_RegisterColor_t Dvar_RegisterColor;

	typedef dvar_t* (__cdecl * Dvar_FindVar_t)(const char *dvarName);
	extern Dvar_FindVar_t Dvar_FindVar;

	typedef char* (__cdecl* Dvar_InfoString_Big_t)(int typeMask);
	extern Dvar_InfoString_Big_t Dvar_InfoString_Big;

	typedef dvar_t* (__cdecl * Dvar_SetCommand_t)(const char* name, const char* value);
	extern Dvar_SetCommand_t Dvar_SetCommand;

	typedef void(__cdecl * Field_Clear_t)(void* field);
	extern Field_Clear_t Field_Clear;

	typedef void(__cdecl * FreeMemory_t)(void* buffer);
	extern FreeMemory_t FreeMemory;

	typedef void(__cdecl * FS_FreeFile_t)(void* buffer);
	extern FS_FreeFile_t FS_FreeFile;

	typedef int(__cdecl * FS_ReadFile_t)(const char* path, char** buffer);
	extern FS_ReadFile_t FS_ReadFile;

	typedef char** (__cdecl * FS_ListFiles_t)(const char *path, const char *extension, FsListBehavior_e behavior, int *numfiles, int allocTrackType);
	extern FS_ListFiles_t FS_ListFiles;

	typedef void(__cdecl * FS_FreeFileList_t)(char** list);
	extern FS_FreeFileList_t FS_FreeFileList;

	typedef int(__cdecl * FS_FOpenFileAppend_t)(char* file);
	extern FS_FOpenFileAppend_t FS_FOpenFileAppend;
	extern FS_FOpenFileAppend_t FS_FOpenFileWrite;

	typedef int(__cdecl * FS_FOpenFileRead_t)(const char* file, int* fh, int uniqueFile);
	extern FS_FOpenFileRead_t FS_FOpenFileRead;

	typedef int(__cdecl * FS_FCloseFile_t)(int fh);
	extern FS_FCloseFile_t FS_FCloseFile;

	typedef bool(__cdecl * FS_FileExists_t)(const char* file);
	extern FS_FileExists_t FS_FileExists;

	typedef bool(__cdecl * FS_WriteFile_t)(char* filename, char* folder, void* buffer, int size);
	extern FS_WriteFile_t FS_WriteFile;

	typedef int(__cdecl * FS_Write_t)(void* buffer, size_t size, int file);
	extern FS_Write_t FS_Write;

	typedef int(__cdecl * FS_Read_t)(void* buffer, size_t size, int file);
	extern FS_Read_t FS_Read;

	typedef int(__cdecl * FS_Seek_t)(int fileHandle, int seekPosition, int seekOrigin);
	extern FS_Seek_t FS_Seek;

	typedef int(__cdecl * FS_FTell_t)(int fileHandle);
	extern FS_FTell_t FS_FTell;

	typedef int(__cdecl * FS_Remove_t)(char *);
	extern FS_Remove_t FS_Remove;

	typedef int(__cdecl * FS_Restart_t)(int a1, int a2);
	extern FS_Restart_t FS_Restart;

	typedef int(__cdecl * FS_BuildPathToFile_t)(const char*, const char*, const char*, char**);
	extern FS_BuildPathToFile_t FS_BuildPathToFile;

	typedef void(__cdecl * Menus_CloseAll_t)(UiContext *dc);
	extern Menus_CloseAll_t Menus_CloseAll;

	typedef int(__cdecl * Menus_OpenByName_t)(UiContext *dc, const char *p);
	extern Menus_OpenByName_t Menus_OpenByName;

	typedef const char* (__cdecl * NET_AdrToString_t)(netadr_t adr);
	extern NET_AdrToString_t NET_AdrToString;

	typedef bool(__cdecl * NET_CompareAdr_t)(netadr_t, netadr_t);
	extern NET_CompareAdr_t NET_CompareAdr;

	typedef bool(__cdecl * NET_IsLocalAddress_t)(netadr_t);
	extern NET_IsLocalAddress_t NET_IsLocalAddress;

	typedef bool(__cdecl * NET_StringToAdr_t)(const char*, netadr_t*);
	extern NET_StringToAdr_t NET_StringToAdr;

	typedef void(__cdecl * Live_MPAcceptInvite_t)(_XSESSION_INFO *hostInfo, const int controllerIndex, bool fromGameInvite);
	extern Live_MPAcceptInvite_t Live_MPAcceptInvite;

	typedef void(__cdecl * Live_ParsePlaylists_t)(const char* data);
	extern Live_ParsePlaylists_t Live_ParsePlaylists;

	typedef void(*LoadInitialFF_t)(void);
	extern LoadInitialFF_t LoadInitialFF;

	typedef void* (__cdecl * LoadModdableRawfile_t)(int a1, const char* filename);
	extern LoadModdableRawfile_t LoadModdableRawfile;

	typedef char* (__cdecl * LocalizeString_t)(char*, char*);
	extern LocalizeString_t LocalizeString;

	typedef char* (__cdecl * LocalizeMapString_t)(char*);
	extern LocalizeMapString_t LocalizeMapString;

	typedef void(__cdecl* sendOOB_t)(int, int, int, int, int, int, const char*);
	extern sendOOB_t OOBPrint;

	typedef void(__cdecl* sendOOBRaw_t)(int, size_t, const char*, int, int, int, int, int);
	extern sendOOBRaw_t OOBPrintRawData;

	typedef char* (__cdecl * SE_Load_t)(char* file, int Unk);
	extern SE_Load_t SE_Load;

	typedef int(__cdecl * PC_ReadToken_t)(source_t*, token_t*);
	extern PC_ReadToken_t PC_ReadToken;

	typedef int(__cdecl * PC_ReadTokenHandle_t)(int handle, pc_token_s *pc_token);
	extern PC_ReadTokenHandle_t PC_ReadTokenHandle;

	typedef void(__cdecl * PC_SourceError_t)(int, const char*, ...);
	extern PC_SourceError_t PC_SourceError;

	typedef int(__cdecl * Party_GetMaxPlayers_t)(party_s* party);
	extern Party_GetMaxPlayers_t Party_GetMaxPlayers;

	typedef int(__cdecl * PartyHost_CountMembers_t)(PartyData_s* party);
	extern PartyHost_CountMembers_t PartyHost_CountMembers;

	typedef netadr_t *(__cdecl * PartyHost_GetMemberAddressBySlot_t)(int unk, void *party, const int slot);
	extern PartyHost_GetMemberAddressBySlot_t PartyHost_GetMemberAddressBySlot;

	typedef const char *(__cdecl * PartyHost_GetMemberName_t)(PartyData_s* party, const int clientNum);
	extern PartyHost_GetMemberName_t PartyHost_GetMemberName;

	typedef script_t* (__cdecl * Script_Alloc_t)(int length);
	extern Script_Alloc_t Script_Alloc;

	typedef void(__cdecl * Script_SetupTokens_t)(script_t* script, void* tokens);
	extern Script_SetupTokens_t Script_SetupTokens;

	typedef int(__cdecl * Script_CleanString_t)(char* buffer);
	extern Script_CleanString_t Script_CleanString;

	typedef void(__cdecl * SetConsole_t)(const char* cvar, const char* value);
	extern SetConsole_t SetConsole;

	typedef char* (__cdecl * SL_ConvertToString_t)(unsigned short);
	extern SL_ConvertToString_t SL_ConvertToString;

	typedef void(__cdecl * Steam_JoinLobby_t)(SteamID, char);
	extern Steam_JoinLobby_t Steam_JoinLobby;

	typedef int(__cdecl* SV_GameClientNum_Score_t)(int clientID);
	extern SV_GameClientNum_Score_t SV_GameClientNum_Score;

	typedef bool(__cdecl * Sys_IsMainThread_t)();
	extern Sys_IsMainThread_t Sys_IsMainThread;

	typedef void(__cdecl * UI_AddMenuList_t)(UiContext *dc, MenuList *menuList, int close);
	extern UI_AddMenuList_t UI_AddMenuList;

	typedef MenuList *(__cdecl * UI_LoadMenus_t)(const char *menuFile, int imageTrack);
	extern UI_LoadMenus_t UI_LoadMenus;

	typedef void(__cdecl * UI_DrawHandlePic_t)(/*ScreenPlacement*/void *scrPlace, float x, float y, float w, float h, int horzAlign, int vertAlign, const float *color, Material *material);
	extern UI_DrawHandlePic_t UI_DrawHandlePic;

	typedef const char * (__cdecl * Win_GetLanguage_t)();
	extern Win_GetLanguage_t Win_GetLanguage;

	extern void** DB_XAssetPool;
	extern unsigned int* g_poolSize;

	extern DWORD* cmd_id;
	extern DWORD* cmd_argc;
	extern char*** cmd_argv;

	extern int* svs_numclients;
	extern client_t* svs_clients;

	extern source_t **sourceFiles;
	extern keywordHash_t **menuParseKeywordHash;

	extern UiContext *uiContext;

	extern int* arenaCount;
	extern mapArena_t* arenas;

	extern int* gameTypeCount;
	extern gameTypeName_t* gameTypes;

	extern XBlock** g_streamBlocks;

	extern bool* g_lobbyCreateInProgress;
	extern party_t** partyIngame;
	extern PartyData_s** partyData;

	extern int* numIP;
	extern netIP_t* localIP;

	void* ReallocateAssetPool(XAssetType type, unsigned int newSize);
	void Menu_FreeItemMemory(Game::itemDef_t* item);
	void OOBPrintT(int type, netadr_t netadr, const char* message);
	void OOBPrintRaw(int type, netadr_t netadr, const char* message, size_t length);
	const char* TabeLookup(StringTable* stringtable, int row, int column);
	const char* UI_LocalizeMapName(const char* mapName);
	const char* UI_LocalizeGameType(const char* gameType);

	const char *DB_GetXAssetName(XAsset *asset);
	XAssetType DB_GetXAssetNameType(const char* name);
}
