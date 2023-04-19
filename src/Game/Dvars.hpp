#pragma once

// Put game dvars here
namespace Game
{
	typedef dvar_t*(*Dvar_RegisterBool_t)(const char* dvarName, bool value, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterBool_t Dvar_RegisterBool;

	typedef dvar_t*(*Dvar_RegisterFloat_t)(const char* dvarName, float value, float min, float max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterFloat_t Dvar_RegisterFloat;

	typedef dvar_t*(*Dvar_RegisterVec2_t)(const char* dvarName, float x, float y, float min, float max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterVec2_t Dvar_RegisterVec2;

	typedef dvar_t*(*Dvar_RegisterVec3_t)(const char* dvarName, float x, float y, float z, float min, float max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterVec3_t Dvar_RegisterVec3;

	typedef dvar_t*(*Dvar_RegisterVec4_t)(const char* dvarName, float x, float y, float z, float w, float min, float max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterVec4_t Dvar_RegisterVec4;

	typedef dvar_t*(*Dvar_RegisterInt_t)(const char* dvarName, int value, int min, int max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterInt_t Dvar_RegisterInt;

	typedef dvar_t*(*Dvar_RegisterEnum_t)(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterEnum_t Dvar_RegisterEnum;

	typedef dvar_t*(*Dvar_RegisterString_t)(const char* dvarName, const char* value, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterString_t Dvar_RegisterString;

	typedef dvar_t*(*Dvar_RegisterColor_t)(const char* dvarName, float r, float g, float b, float a, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterColor_t Dvar_RegisterColor;

	typedef dvar_t*(*Dvar_RegisterVec3Color_t)(const char* dvarName, float x, float y, float z, float max, unsigned __int16 flags, const char* description);
	extern Dvar_RegisterVec3Color_t Dvar_RegisterVec3Color;

	typedef void(*Dvar_SetFromStringByName_t)(const char* dvarName, const char* string);
	extern Dvar_SetFromStringByName_t Dvar_SetFromStringByName;

	typedef const dvar_t*(*Dvar_SetFromStringByNameFromSource_t)(const char* dvarName, const char* string, DvarSetSource source);
	extern Dvar_SetFromStringByNameFromSource_t Dvar_SetFromStringByNameFromSource;

	typedef void(*Dvar_SetStringByName_t)(const char* dvarName, const char* value);
	extern Dvar_SetStringByName_t Dvar_SetStringByName;

	typedef void(*Dvar_SetString_t)(const dvar_t* dvar, const char* value);
	extern Dvar_SetString_t Dvar_SetString;

	typedef void(*Dvar_SetBool_t)(const dvar_t* dvar, bool enabled);
	extern Dvar_SetBool_t Dvar_SetBool;

	typedef void(*Dvar_SetBoolByName_t)(const char* dvarName, bool value);
	extern Dvar_SetBoolByName_t Dvar_SetBoolByName;

	typedef void(*Dvar_SetFloat_t)(const dvar_t* dvar, float value);
	extern Dvar_SetFloat_t Dvar_SetFloat;

	typedef void(*Dvar_SetFloatByName_t)(const char* dvarName, float value);
	extern Dvar_SetFloatByName_t Dvar_SetFloatByName;

	typedef void(*Dvar_SetInt_t)(const dvar_t* dvar, int integer);
	extern Dvar_SetInt_t Dvar_SetInt;

	typedef void(*Dvar_GetUnpackedColorByName_t)(const char* dvarName, float* expandedColor);
	extern Dvar_GetUnpackedColorByName_t Dvar_GetUnpackedColorByName;

	typedef char*(*Dvar_GetString_t)(const char* dvarName);
	extern Dvar_GetString_t Dvar_GetString;

	typedef char*(*Dvar_GetVariantString_t)(const char* dvarName);
	extern Dvar_GetVariantString_t Dvar_GetVariantString;

	typedef dvar_t*(*Dvar_FindVar_t)(const char* dvarName);
	extern Dvar_FindVar_t Dvar_FindVar;

	typedef char*(*Dvar_InfoString_Big_t)(int bit);
	extern Dvar_InfoString_Big_t Dvar_InfoString_Big;

	typedef void(*Dvar_SetCommand_t)(const char* dvarName, const char* string);
	extern Dvar_SetCommand_t Dvar_SetCommand;

	typedef const char*(*Dvar_DisplayableValue_t)(const dvar_t* dvar);
	extern Dvar_DisplayableValue_t Dvar_DisplayableValue;

	typedef void(*Dvar_Reset_t)(const dvar_t* dvar, DvarSetSource setSource);
	extern Dvar_Reset_t Dvar_Reset;

	extern const dvar_t** com_developer;
	extern const dvar_t** com_developer_script;
	extern const dvar_t** com_timescale;
	extern const dvar_t** com_maxFrameTime;
	extern const dvar_t** com_sv_running;
	extern const dvar_t** com_masterServerName;
	extern const dvar_t** com_masterPort;

	extern const dvar_t** dev_timescale;

	extern const dvar_t** dvar_cheats;

	extern const dvar_t** fs_cdpath;
	extern const dvar_t** fs_basepath;
	extern const dvar_t** fs_gameDirVar;
	extern const dvar_t** fs_homepath;

	extern const dvar_t** sv_privatePassword;
	extern const dvar_t** sv_hostname;
	extern const dvar_t** sv_gametype;
	extern const dvar_t** sv_mapname;
	extern const dvar_t** sv_mapRotation;
	extern const dvar_t** sv_mapRotationCurrent;
	extern const dvar_t** sv_maxclients;
	extern const dvar_t** sv_cheats;
	extern const dvar_t** sv_voiceQuality;

	extern const dvar_t** nextmap;

	extern const dvar_t** cl_showSend;
	extern const dvar_t** cl_voice;
	extern const dvar_t** cl_ingame;
	extern const dvar_t** cl_shownet;

	extern const dvar_t** g_cheats;
	extern const dvar_t** g_deadChat;
	extern const dvar_t** g_allowVote;
	extern const dvar_t** g_oldVoting;
	extern const dvar_t** g_gametype;
	extern const dvar_t** g_password;
	extern const dvar_t** g_log;

	extern const dvar_t** cg_chatHeight;
	extern const dvar_t** cg_chatTime;
	extern const dvar_t** cg_scoreboardHeight;
	extern const dvar_t** cg_scoreboardWidth;

	extern const dvar_t** version;

	extern const dvar_t** viewposNow;

	extern const dvar_t** ui_currentMap;
	extern const dvar_t** ui_gametype;
	extern const dvar_t** ui_mapname;
	extern const dvar_t** ui_joinGametype;
	extern const dvar_t** ui_netGameType;
	extern const dvar_t** ui_netSource;

	extern const dvar_t** loc_warnings;
	extern const dvar_t** loc_warningsAsErrors;

	extern const dvar_t** party_minplayers;
	extern const dvar_t** party_maxplayers;

	extern const dvar_t** ip;
	extern const dvar_t** port;

	extern void Dvar_SetVariant(dvar_t* dvar, DvarValue value, DvarSetSource source);
	extern void Dvar_SetFromStringFromSource(const dvar_t* dvar, const char* string, DvarSetSource source);
}
