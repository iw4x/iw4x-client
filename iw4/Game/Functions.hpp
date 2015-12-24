namespace Game
{
	typedef void(__cdecl * Cmd_AddCommand_t)(const char* name, void(*callback), cmd_function_t* data, char);
	extern Cmd_AddCommand_t Cmd_AddCommand;

	typedef void(__cdecl * Com_Error_t)(int type, char* message, ...);
	extern Com_Error_t Com_Error;

	typedef void(__cdecl * Com_Printf_t)(int, const char*, ...);
	extern Com_Printf_t Com_Printf;

	typedef int(__cdecl * Com_Milliseconds_t)(void);
	extern Com_Milliseconds_t Com_Milliseconds;

	typedef XAssetHeader (__cdecl * DB_FindXAssetHeader_t)(XAssetType type, const char* filename);
	extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

	typedef int(__cdecl * DB_GetXAssetSizeHandler_t)();
	extern DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers;

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

	typedef int(__cdecl * Menus_OpenByName_t)(/*UiContext **/int dc, const char *p);
	extern Menus_OpenByName_t Menus_OpenByName;

	typedef void* (__cdecl * LoadModdableRawfile_t)(int a1, const char* filename);
	extern LoadModdableRawfile_t LoadModdableRawfile;

	typedef int(__cdecl * PC_ReadToken_t)(source_t*, token_t*);
	extern PC_ReadToken_t PC_ReadToken;

	typedef void(__cdecl * PC_SourceError_t)(int, const char*, ...);
	extern PC_SourceError_t PC_SourceError;

	typedef script_t* (__cdecl * Script_Alloc_t)(int length);
	extern Script_Alloc_t Script_Alloc;

	typedef void(__cdecl * Script_SetupTokens_t)(script_t* script, void* tokens);
	extern Script_SetupTokens_t Script_SetupTokens;

	typedef int(__cdecl * Script_CleanString_t)(char* buffer);
	extern Script_CleanString_t Script_CleanString;

	typedef const char * (__cdecl * Win_GetLanguage_t)();
	extern Win_GetLanguage_t Win_GetLanguage;

	extern void** DB_XAssetPool;
	extern unsigned int* g_poolSize;

	extern DWORD* cmd_id;
	extern DWORD* cmd_argc;
	extern char*** cmd_argv;

	extern source_t **sourceFiles;
	extern keywordHash_t **menuParseKeywordHash;

	void* ReallocateAssetPool(XAssetType type, unsigned int newSize);
}
