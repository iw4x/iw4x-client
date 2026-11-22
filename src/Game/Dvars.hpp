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

  typedef void(*Dvar_SetFromStringFromSource_t)(const dvar_t* dvar, const char* string, DvarSetSource source);
  extern Dvar_SetFromStringFromSource_t Dvar_SetFromStringFromSource;

  typedef void(*Dvar_SetVariant_t)(dvar_t* dvar, DvarValue value, DvarSetSource source);
  extern Dvar_SetVariant_t Dvar_SetVariant;

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

  typedef void (*Dvar_ClearModified_t) (const dvar_t* dvar);
  extern Dvar_ClearModified_t Dvar_ClearModified;

  // The pool that the game allocates its dvars from, in the order they were
  // registered.
  //
  constexpr int DVAR_COUNT = 0x1000;

  extern dvar_t* dvarPool;

  // Register a new dvar, or register an existing one again, which is what
  // every Dvar_Register*() function comes down to.
  //
  // Note that the type is a byte on x64, and that the value and the domain
  // are passed by pointer.
  //
  typedef dvar_t*(*Dvar_RegisterNew_t)(const char* dvarName, unsigned char type, unsigned int flags, const DvarValue* value, const DvarLimits* domain, const char* description);
  extern Dvar_RegisterNew_t Dvar_RegisterNew;

  typedef void(*Dvar_Reregister_t)(dvar_t* dvar, const char* dvarName, unsigned char type, unsigned int flags, const DvarValue* resetValue, const DvarLimits* domain, const char* description);
  extern Dvar_Reregister_t Dvar_Reregister;

  // Return the description of the dvar, or nullptr if it has none.
  //
  // Note that x64 drops the description field from dvar_t, even though
  // every registration still passes one down to Dvar_RegisterNew() and
  // Dvar_Reregister(), which simply ignore it. So we keep the descriptions
  // ourselves, in a table that parallels the pool, and Components::Dvar
  // fills it in from those two functions (see Dvar_SetDescription()).
  //
  const char* Dvar_GetDescription(const dvar_t* dvar);

  // Set the description of the dvar. Like x86, we keep the pointer rather
  // than a copy, so it has to outlive the dvar, which every caller's string
  // literal does.
  //
  void Dvar_SetDescription(const dvar_t* dvar, const char* description);

  extern const dvar_t** com_developer;
  extern const dvar_t** com_logfile;
  extern const dvar_t** com_developer_script;
  extern const dvar_t** com_timescale;
  extern const dvar_t** com_maxFrameTime;
  extern const dvar_t** sv_running;
  extern const dvar_t** com_masterServerName;
  extern const dvar_t** com_masterPort;

  extern const dvar_t** r_mode;
  extern const dvar_t** vid_xpos;
  extern const dvar_t** vid_ypos;

  extern const dvar_t** dev_timescale;

  // Note that x64 compiles out the registration of sv_cheats, which the
  // Dvar component restores.
  //
  extern const dvar_t* dvar_cheats;

  // Note that x64 also compiles out party_host, along with setting it when
  // we start hosting a party or stop being in one. The Dvar component
  // restores both.
  //
  extern const dvar_t* party_host;

  extern const dvar_t** fs_cdpath;
  extern const dvar_t** fs_basepath;
  extern const dvar_t** fs_gameDirVar;
  extern const dvar_t** fs_homepath;

  extern const dvar_t** sv_privatePassword;
  extern const dvar_t** sv_hostname;
  extern const dvar_t** sv_gametype;
  extern const dvar_t** sv_mapname;
  extern const dvar_t** sv_maxclients;
  extern const dvar_t** onlinegame;
  extern const dvar_t** com_sv_running;
  extern const dvar_t** sv_voiceQuality;

  extern const dvar_t** nextmap;

  extern const dvar_t** perk_diveVelocity;
  extern const dvar_t** perk_diveDistanceCheck;

  extern const dvar_t** cl_voice;
  extern const dvar_t** cl_ingame;
  extern const dvar_t** cl_shownet;

  extern const dvar_t** g_giveAll;
  extern const dvar_t** g_deadChat;
  extern const dvar_t** g_allowVote;
  extern const dvar_t** g_gametype;
  extern const dvar_t** g_password;
  extern const dvar_t** g_log;

  extern const dvar_t** cg_chatHeight;
  extern const dvar_t** cg_drawMaterial;
  extern const dvar_t** cg_nopredict;
  extern const dvar_t** g_synchronousClients;
  extern const dvar_t** drawLagometer;
  extern const dvar_t** drawEntityCount;
  extern const dvar_t** drawEntityCountPos;
  extern const dvar_t** drawEntityCountSize;
  extern const dvar_t** drawServerBandwidth;
  extern const dvar_t** drawServerBandwidthPos;

  // Note that the game registers drawServerBandwidthSize under the name
  // drawEntityCountSize, so the two graphs end up sharing the dvar.
  // Presumably a copy and paste slip, but we leave it be.
  //
  extern const dvar_t** drawServerBandwidthSize;
  extern const dvar_t** drawKillcamData;
  extern const dvar_t** drawKillcamDataPos;
  extern const dvar_t** drawKillcamDataSize;
  extern const dvar_t** cg_hudVotePosition;
  extern const dvar_t** cg_chatTime;
  extern const dvar_t** cg_scoreboardHeight;
  extern const dvar_t** cg_scoreboardWidth;
  extern const dvar_t** cg_drawFPS;
  extern const dvar_t** cg_drawFPSLabels;
  extern const dvar_t** cg_drawSnapshot;
  extern const dvar_t** cg_drawViewpos;

  extern const dvar_t** version;
  /// <summary>
  /// Client Revision
  /// </summary>
  extern const dvar_t** shortversion;

  extern const dvar_t** viewposNow;

  extern const dvar_t** ui_currentMap;
  extern const dvar_t** ui_gametype;
  extern const dvar_t** ui_mapname;
  extern const dvar_t** ui_joinGametype;
  extern const dvar_t** ui_netGameType;
  extern const dvar_t** ui_netSource;
  extern const dvar_t** stringtable_debug;

  extern const dvar_t** loc_warnings;
  extern const dvar_t** loc_warningsAsErrors;

  extern const dvar_t** party_minplayers;
  extern const dvar_t** party_maxplayers;
  extern const dvar_t** party_hostname;

  extern const dvar_t** ip;
  extern const dvar_t** port;
}
