#include "Dvars.hpp"

namespace Game
{
  Dvar_RegisterBool_t Dvar_RegisterBool = Dvar_RegisterBool_t(0x140284190);
  Dvar_RegisterFloat_t Dvar_RegisterFloat = Dvar_RegisterFloat_t(0x140284570);
  Dvar_RegisterVec2_t Dvar_RegisterVec2 = Dvar_RegisterVec2_t(0x140284B10);
  Dvar_RegisterVec3_t Dvar_RegisterVec3 = Dvar_RegisterVec3_t(0x140284C30);
  Dvar_RegisterVec4_t Dvar_RegisterVec4 = Dvar_RegisterVec4_t(0x140284E80);
  Dvar_RegisterInt_t Dvar_RegisterInt = Dvar_RegisterInt_t(0x1402846A0);
  Dvar_RegisterEnum_t Dvar_RegisterEnum = Dvar_RegisterEnum_t(0x140284470);
  Dvar_RegisterString_t Dvar_RegisterString = Dvar_RegisterString_t(0x140284A40);
  Dvar_RegisterColor_t Dvar_RegisterColor = Dvar_RegisterColor_t(0x140284270);
  Dvar_RegisterVec3Color_t Dvar_RegisterVec3Color = Dvar_RegisterVec3Color_t(0x140284D60);

  Dvar_GetUnpackedColorByName_t Dvar_GetUnpackedColorByName = Dvar_GetUnpackedColorByName_t(0x140283810);
  Dvar_GetString_t Dvar_GetString = Dvar_GetString_t(0x140283780);
  Dvar_GetVariantString_t Dvar_GetVariantString = Dvar_GetVariantString_t(0x1402838A0);
  Dvar_FindVar_t Dvar_FindVar = Dvar_FindVar_t(0x140283520);
  Dvar_InfoString_Big_t Dvar_InfoString_Big = Dvar_InfoString_Big_t(0x1401FC1C0);
  Dvar_SetCommand_t Dvar_SetCommand = Dvar_SetCommand_t(0x140285750);
  Dvar_DisplayableValue_t Dvar_DisplayableValue = Dvar_DisplayableValue_t(0x140283490);
  Dvar_ClearModified_t Dvar_ClearModified = Dvar_ClearModified_t(0x140283440);
  Dvar_Reset_t Dvar_Reset = Dvar_Reset_t(0x140285260);

  Dvar_SetFromStringByName_t Dvar_SetFromStringByName = Dvar_SetFromStringByName_t(0x140285A20);
  Dvar_SetFromStringByNameFromSource_t Dvar_SetFromStringByNameFromSource = Dvar_SetFromStringByNameFromSource_t(0x140285A80);
  Dvar_SetFromStringFromSource_t Dvar_SetFromStringFromSource = Dvar_SetFromStringFromSource_t(0x140285AF0);
  Dvar_SetVariant_t Dvar_SetVariant = Dvar_SetVariant_t(0x140286030);
  Dvar_SetStringByName_t Dvar_SetStringByName = Dvar_SetStringByName_t(0x140285F90);
  Dvar_SetString_t Dvar_SetString = Dvar_SetString_t(0x140285F30);
  Dvar_SetBool_t Dvar_SetBool = Dvar_SetBool_t(0x140285460);
  Dvar_SetBoolByName_t Dvar_SetBoolByName = Dvar_SetBoolByName_t(0x1402854B0);
  Dvar_SetFloat_t Dvar_SetFloat = Dvar_SetFloat_t(0x140285850);
  Dvar_SetFloatByName_t Dvar_SetFloatByName = Dvar_SetFloatByName_t(0x1402858C0);
  Dvar_SetInt_t Dvar_SetInt = Dvar_SetInt_t(0x140285B90);

  const dvar_t** com_developer = reinterpret_cast<const dvar_t**>(0x141BD78D0);
  const dvar_t** com_logfile = reinterpret_cast<const dvar_t**>(0x141BD78D8);
  const dvar_t** com_developer_script = reinterpret_cast<const dvar_t**>(0x141BD7410);
  const dvar_t** com_timescale = reinterpret_cast<const dvar_t**>(0x141BD7908);
  const dvar_t** com_maxFrameTime = reinterpret_cast<const dvar_t**>(0x141BD7420);
  const dvar_t** com_masterServerName = reinterpret_cast<const dvar_t**>(0x141BD78E0);
  const dvar_t** com_masterPort = reinterpret_cast<const dvar_t**>(0x141BD78F0);

  const dvar_t** r_mode   = reinterpret_cast<const dvar_t**>(0x148CC4820);
  const dvar_t** vid_xpos = reinterpret_cast<const dvar_t**>(0x148CC4A10);
  const dvar_t** vid_ypos = reinterpret_cast<const dvar_t**>(0x148CC47D8);

  const dvar_t** dev_timescale = reinterpret_cast<const dvar_t**>(0x141BD7900);

  const dvar_t** dvar_cheats = reinterpret_cast<const dvar_t**>(0x1466E38E1);

  const dvar_t** fs_cdpath = reinterpret_cast<const dvar_t**>(0x1466429F8);
  const dvar_t** fs_basepath = reinterpret_cast<const dvar_t**>(0x1466429E8);
  const dvar_t** fs_gameDirVar = reinterpret_cast<const dvar_t**>(0x146642A08);
  const dvar_t** fs_homepath = reinterpret_cast<const dvar_t**>(0x1466429E0);

  const dvar_t** sv_privatePassword = reinterpret_cast<const dvar_t**>(0x14650B6C0);
  const dvar_t** sv_running = reinterpret_cast<const dvar_t**>(0x141BD7910);
  const dvar_t** sv_hostname = reinterpret_cast<const dvar_t**>(0x14650B6E0);
  const dvar_t** sv_gametype = reinterpret_cast<const dvar_t**>(0x14650B708);
  const dvar_t** sv_mapname = reinterpret_cast<const dvar_t**>(0x14650B6F8);
  const dvar_t** sv_maxclients = reinterpret_cast<const dvar_t**>(0x14650B6D8);
  const dvar_t** com_sv_running = reinterpret_cast<const dvar_t**>(0x141BD7910);
  const dvar_t** sv_voiceQuality = reinterpret_cast<const dvar_t**>(0x14650B728);

  const dvar_t** nextmap = reinterpret_cast<const dvar_t**>(0x141BD7938);

  const dvar_t** cl_voice = reinterpret_cast<const dvar_t**>(0x140D02F70);
  const dvar_t** cl_ingame = reinterpret_cast<const dvar_t**>(0x140D02F60);
  const dvar_t** cl_shownet = reinterpret_cast<const dvar_t**>(0x1406CBDB0);

  const dvar_t** g_giveAll = reinterpret_cast<const dvar_t**>(0x1418679B8);
  const dvar_t** g_deadChat = reinterpret_cast<const dvar_t**>(0x1418678C0);
  const dvar_t** g_allowVote = reinterpret_cast<const dvar_t**>(0x141867888);
  const dvar_t** g_oldVoting = reinterpret_cast<const dvar_t**>(0x1417CBCA0);
  const dvar_t** g_gametype = reinterpret_cast<const dvar_t**>(0x1417CBDB8);
  const dvar_t** g_password = reinterpret_cast<const dvar_t**>(0x141867848);
  const dvar_t** g_log = reinterpret_cast<const dvar_t**>(0x1417CBC90);

  const dvar_t** cg_chatHeight = reinterpret_cast<const dvar_t**>(0x1406BB130);
  const dvar_t** cg_chatTime = reinterpret_cast<const dvar_t**>(0x1406BB128);
  const dvar_t** cg_scoreboardHeight = reinterpret_cast<const dvar_t**>(0x1406BF958);
  const dvar_t** cg_scoreboardWidth = reinterpret_cast<const dvar_t**>(0x1406BF950);
  const dvar_t** cg_drawFPS = reinterpret_cast<const dvar_t**>(0x140585388);
  const dvar_t** cg_drawFPSLabels = reinterpret_cast<const dvar_t**>(0x1406BACE0);
  const dvar_t** cg_drawSnapshot = reinterpret_cast<const dvar_t**>(0x1406BAD08);
  const dvar_t** cg_drawViewpos = reinterpret_cast<const dvar_t**>(0x140589718);

  const dvar_t** version = reinterpret_cast<const dvar_t**>(0x141BD7920);
  const dvar_t** shortversion = reinterpret_cast<const dvar_t**>(0x141BD7928);

  const dvar_t** viewposNow = reinterpret_cast<const dvar_t**>(0x1404609D0);

  const dvar_t** ui_currentMap = reinterpret_cast<const dvar_t**>(0x1465CE830);
  const dvar_t** ui_gametype = reinterpret_cast<const dvar_t**>(0x14663E7D8);
  const dvar_t** ui_mapname = reinterpret_cast<const dvar_t**>(0x1465CE7C0);
  const dvar_t** ui_joinGametype = reinterpret_cast<const dvar_t**>(0x1465CE880);
  const dvar_t** ui_netGameType = reinterpret_cast<const dvar_t**>(0x1465CE878);
  const dvar_t** ui_netSource = reinterpret_cast<const dvar_t**>(0x1465CE828);
  const dvar_t** stringtable_debug = reinterpret_cast<const dvar_t**>(0x1465CE7D0);

  const dvar_t** loc_warnings = reinterpret_cast<const dvar_t**>(0x14651EFB0);
  const dvar_t** loc_warningsAsErrors = reinterpret_cast<const dvar_t**>(0x14651EFB8);

  const dvar_t** party_minplayers = reinterpret_cast<const dvar_t**>(0x140D09808);
  const dvar_t** party_maxplayers = reinterpret_cast<const dvar_t**>(0x140D09998);
  const dvar_t** party_hostname = reinterpret_cast<const dvar_t**>(0x140D09890);

  const dvar_t** perk_diveVelocity = reinterpret_cast<const dvar_t**>(0x14043EDD8);
  const dvar_t** perk_diveDistanceCheck = reinterpret_cast<const dvar_t**>(0x14043EDE0);

  const dvar_t** ip = reinterpret_cast<const dvar_t**>(0x14678A238);
  const dvar_t** port = reinterpret_cast<const dvar_t**>(0x14678A240);
}
