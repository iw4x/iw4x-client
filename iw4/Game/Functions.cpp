#include "..\STDInclude.hpp"

namespace Game
{
	Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x470090;

	Com_Error_t Com_Error = (Com_Error_t)0x4B22D0;
	Com_Printf_t Com_Printf = (Com_Printf_t)0x402500;
	Com_Milliseconds_t Com_Milliseconds = (Com_Milliseconds_t)0x42A660;

	DB_FindXAssetHeader_t DB_FindXAssetHeader = (DB_FindXAssetHeader_t)0x407930;
	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = (DB_GetXAssetSizeHandler_t*)0x799488;
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
	Dvar_SetCommand_t Dvar_SetCommand = (Dvar_SetCommand_t)0x4EE430;

	Field_Clear_t Field_Clear = (Field_Clear_t)0x437EB0;

	FreeMemory_t FreeMemory = (FreeMemory_t)0x4D6640;

	FS_FreeFile_t FS_FreeFile = (FS_FreeFile_t)0x4416B0;
	FS_ReadFile_t FS_ReadFile = (FS_ReadFile_t)0x4F4B90;

	Menus_OpenByName_t Menus_OpenByName = (Menus_OpenByName_t)0x4CCE60;

	LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x61ABC0;

	PC_ReadToken_t PC_ReadToken = (PC_ReadToken_t)0x4ACCD0;
	PC_SourceError_t PC_SourceError = (PC_SourceError_t)0x467A00;

	Script_Alloc_t Script_Alloc = (Script_Alloc_t)0x422E70;
	Script_SetupTokens_t Script_SetupTokens = (Script_SetupTokens_t)0x4E6950;
	Script_CleanString_t Script_CleanString = (Script_CleanString_t)0x498220;

	Win_GetLanguage_t Win_GetLanguage = (Win_GetLanguage_t)0x45CBA0;

	void** DB_XAssetPool = (void**)0x7998A8;
	unsigned int* g_poolSize = (unsigned int*)0x7995E8;

	DWORD* cmd_id = (DWORD*)0x1AAC5D0;
	DWORD* cmd_argc = (DWORD*)0x1AAC614;
	char*** cmd_argv = (char***)0x1AAC634;

	source_t **sourceFiles = (source_t **)0x7C4A98;
	keywordHash_t **menuParseKeywordHash = (keywordHash_t **)0x63AE928;

	void* ReallocateAssetPool(XAssetType type, unsigned int newSize)
	{
		int elSize = DB_GetXAssetSizeHandlers[type]();
		void* poolEntry = new char[newSize * elSize];
		DB_XAssetPool[type] = poolEntry;
		g_poolSize[type] = newSize;
		return poolEntry;
	}
}