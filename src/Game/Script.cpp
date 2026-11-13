#include "Script.hpp"

namespace Game
{
  AddRefToObject_t AddRefToObject = AddRefToObject_t(0x140220C10);
  RemoveRefToObject_t RemoveRefToObject = RemoveRefToObject_t(0x1402233A0);
  RemoveRefToValue_t RemoveRefToValue = RemoveRefToValue_t(0x1402234C0);
  AllocObject_t AllocObject = AllocObject_t(0x140220ED0);
  AddRefToValue_t AddRefToValue = AddRefToValue_t(0x140220C30);
  FindVariable_t FindVariable = FindVariable_t(0x140221D30);
  GetVariable_t GetVariable = GetVariable_t(0x140222DD0);
  RemoveVariable_t RemoveVariable = RemoveVariable_t(0x140223590);
  FindObject_t FindObject = FindObject_t(0x140221C80);
  GetObject_t GetObject = GetObject_t(0x140222B90);
  GetNewVariable_t GetNewVariable = GetNewVariable_t(0x140222410);
  AllocThread_t AllocThread = AllocThread_t(0x140220F00);

  VM_Execute_0_t VM_Execute_0 = VM_Execute_0_t(0x140229CE0);

  Scr_IsValidGameType_t Scr_IsValidGameType = Scr_IsValidGameType_t(0x1401A7580);
  Scr_LoadGameType_t Scr_LoadGameType = Scr_LoadGameType_t(0x1401A7610);
  Scr_StartupGameType_t Scr_StartupGameType = Scr_StartupGameType_t(0x1401A7D30);

  Scr_LoadScript_t Scr_LoadScript = Scr_LoadScript_t(0x14021C140);
  Scr_ReadFile_FastFile_t Scr_ReadFile_FastFile = Scr_ReadFile_FastFile_t(0x14021D0D0);
  Scr_GetFunctionHandle_t Scr_GetFunctionHandle = Scr_GetFunctionHandle_t(0x14021C010);
  Scr_CreateCanonicalFilename_t Scr_CreateCanonicalFilename = Scr_CreateCanonicalFilename_t(0x1402204F0);

  Scr_GetString_t Scr_GetString = Scr_GetString_t(0x1402289A0);
  Scr_GetConstString_t Scr_GetConstString = Scr_GetConstString_t(0x140228250);
  Scr_GetFloat_t Scr_GetFloat = Scr_GetFloat_t(0x1402284F0);
  Scr_GetVector_t Scr_GetVector = Scr_GetVector_t(0x140228B20);
  Scr_GetInt_t Scr_GetInt = Scr_GetInt_t(0x140228630);
  Scr_GetObject_t Scr_GetObject = Scr_GetObject_t(0x1402286F0);
  Scr_GetTypeName_t Scr_GetTypeName = Scr_GetTypeName_t(0x140228A30);
  Scr_GetNumParam_t Scr_GetNumParam = Scr_GetNumParam_t(0x1402286E0);
  Scr_GetEntityId_t Scr_GetEntityId = Scr_GetEntityId_t(0x1402268F0);

  Scr_ExecThread_t Scr_ExecThread = Scr_ExecThread_t(0x140227E90);
  Scr_FreeThread_t Scr_FreeThread = Scr_FreeThread_t(0x140227F20);

  Scr_AddEntity_t Scr_AddEntity = Scr_AddEntity_t(0x1401A96E0);
  Scr_AddString_t Scr_AddString = Scr_AddString_t(0x140227A00);
  Scr_AddConstString_t Scr_AddConstString = Scr_AddConstString_t(0x140227870);
  Scr_AddArray_t Scr_AddArray = Scr_AddArray_t(0x1402277A0);
  Scr_MakeArray_t Scr_MakeArray = Scr_MakeArray_t(0x140228F20);
  Scr_AddIString_t Scr_AddIString = Scr_AddIString_t(0x140227990);
  Scr_AddInt_t Scr_AddInt = Scr_AddInt_t(0x140227840);
  Scr_AddFloat_t Scr_AddFloat = Scr_AddFloat_t(0x140227950);
  Scr_AddObject_t Scr_AddObject = Scr_AddObject_t(0x1402279D0);
  Scr_Notify_t Scr_Notify = Scr_Notify_t(0x1401A9DD0);
  Scr_NotifyId_t Scr_NotifyId = Scr_NotifyId_t(0x140228F50);
  Scr_NotifyLevel_t Scr_NotifyLevel = Scr_NotifyLevel_t(0x140229040);

  Scr_ErrorInternal_t Scr_ErrorInternal = Scr_ErrorInternal_t(0x140227D90);

  Scr_GetType_t Scr_GetType = Scr_GetType_t(0x1402289C0);
  Scr_GetPointerType_t Scr_GetPointerType = Scr_GetPointerType_t(0x1402288F0);

  Scr_ClearOutParams_t Scr_ClearOutParams = Scr_ClearOutParams_t(0x1402276F0);

  Scr_GetObjectField_t Scr_GetObjectField = Scr_GetObjectField_t(0x1401A9DA0);
  Scr_SetObjectField_t Scr_SetObjectField = Scr_SetObjectField_t(0x1401A9F10);
  Scr_GetEntityField_t Scr_GetEntityField = Scr_GetEntityField_t(0x1401A9A70);
  Scr_SetClientField_t Scr_SetClientField = Scr_SetClientField_t(0x140163E60);
  Scr_AddClassField_t Scr_AddClassField = Scr_AddClassField_t(0x140223870);

  Scr_ConstructMessageString_t Scr_ConstructMessageString = Scr_ConstructMessageString_t(0x1401A7110);

  Scr_FreeHudElemConstStrings_t Scr_FreeHudElemConstStrings = Scr_FreeHudElemConstStrings_t(0x14016BD00);

  GScr_LoadGameTypeScript_t GScr_LoadGameTypeScript = GScr_LoadGameTypeScript_t(0x1401A66F0);

  GetEntity_t GetEntity = GetEntity_t(0x140181DD0);

  Scr_RegisterFunction_t Scr_RegisterFunction = Scr_RegisterFunction_t(0x14021B210);
  Scr_ShutdownAllocNode_t Scr_ShutdownAllocNode = Scr_ShutdownAllocNode_t(0x14021D260);
  Scr_IsSystemActive_t Scr_IsSystemActive = Scr_IsSystemActive_t(0x1401A7980);

  SL_ConvertToString_t SL_ConvertToString = SL_ConvertToString_t(0x14021FB90);
  SL_GetString_t SL_GetString = SL_GetString_t(0x14021FEA0);
  SL_GetString__t  SL_GetString_ = SL_GetString__t(0x140220B60);
  SL_FindString_t SL_FindString = SL_FindString_t(0x14021FE40);
  SL_FindLowercaseString_t SL_FindLowercaseString = SL_FindLowercaseString_t(0x14021FC00);
  SL_AddRefToString_t SL_AddRefToString = SL_AddRefToString_t(0x14021FB10);
  SL_RemoveRefToString_t SL_RemoveRefToString = SL_RemoveRefToString_t(0x1402200F0);

  ScriptParse_t ScriptParse = ScriptParse_t(0x14022DF40);
  ScriptCompile_t ScriptCompile = ScriptCompile_t(0x14021B420);

  scr_const_t* scr_const = reinterpret_cast<scr_const_t*>(0x1417CA160);

  scrVmPub_t* scrVmPub = reinterpret_cast<scrVmPub_t*>(0x14227B890);
  scrVarPub_t* scrVarPub = reinterpret_cast<scrVarPub_t*>(0x141F27780);
  scrCompilePub_t* scrCompilePub = reinterpret_cast<scrCompilePub_t*>(0x141E0AFA0);
  scrAnimPub_t* scrAnimPub = reinterpret_cast<scrAnimPub_t*>(0x141E0A550);

  char* g_EndPos = reinterpret_cast<char*>(0x14227B888);
  bool* g_loadedImpureScript = reinterpret_cast<bool*>(0x141EEF584);

  game_hudelem_s* g_hudelems = reinterpret_cast<game_hudelem_s*>(0x14177EFA0);

  void IncInParam()
  {
    Scr_ClearOutParams();

    if (scrVmPub->top == scrVmPub->maxStack)
    {
      Sys_Error("Internal script stack overflow");
    }

    scrVmPub->top++;
    scrVmPub->inparamcount++;
  }

  void Scr_Error(const char* error)
  {
    Com_PrintError(CON_CHANNEL_PARSERSCRIPT, "%s\n", error);
    Scr_ErrorInternal();
  }

  void Scr_ParamError(unsigned int paramIndex, const char* error)
  {
    Com_PrintError(CON_CHANNEL_PARSERSCRIPT, "parameter %u: %s\n", paramIndex + 1, error);
    Scr_ErrorInternal();
  }

  void Scr_AddBool(int value)
  {
    assert(value == 0 || value == 1);

    IncInParam();
    scrVmPub->top->type = VAR_INTEGER;
    scrVmPub->top->u.intValue = value;
  }
}
