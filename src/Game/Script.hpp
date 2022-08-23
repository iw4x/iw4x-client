#pragma once

namespace Game
{
	typedef void(*AddRefToObject_t)(unsigned int id);
	extern AddRefToObject_t AddRefToObject;

	typedef void(*RemoveRefToObject_t)(unsigned int id);
	extern RemoveRefToObject_t RemoveRefToObject;

	typedef unsigned int(*AllocObject_t)();
	extern AllocObject_t AllocObject;

	typedef void(*AddRefToValue_t)(int type, VariableUnion u);
	extern AddRefToValue_t AddRefToValue;

	typedef unsigned int(*AllocThread_t)(unsigned int self);
	extern AllocThread_t AllocThread;

	typedef unsigned int(*VM_Execute_0_t)(unsigned int localId, const char* pos, unsigned int paramcount);
	extern VM_Execute_0_t VM_Execute_0;

	typedef void(*Scr_AddEntity_t)(const gentity_s* ent);
	extern Scr_AddEntity_t Scr_AddEntity;

	typedef void(*Scr_AddString_t)(const char* value);
	extern Scr_AddString_t Scr_AddString;

	typedef void(*Scr_AddConstString_t)(unsigned int value);
	extern Scr_AddConstString_t Scr_AddConstString;

	typedef void(*Scr_AddIString_t)(const char* value);
	extern Scr_AddIString_t Scr_AddIString;

	typedef void(*Scr_AddInt_t)(int value);
	extern Scr_AddInt_t Scr_AddInt;

	typedef void(*Scr_AddFloat_t)(float value);
	extern Scr_AddFloat_t Scr_AddFloat;

	typedef void(*Scr_AddObject_t)(unsigned int id);
	extern Scr_AddObject_t Scr_AddObject;

	typedef void(*Scr_ShutdownAllocNode_t)();
	extern Scr_ShutdownAllocNode_t Scr_ShutdownAllocNode;

	typedef char* (*Scr_GetGameTypeNameForScript_t)(const char* pszGameTypeScript);
	extern Scr_GetGameTypeNameForScript_t Scr_GetGameTypeNameForScript;

	typedef int(*Scr_IsValidGameType_t)(const char* pszGameType);
	extern Scr_IsValidGameType_t Scr_IsValidGameType;

	typedef void(*Scr_LoadGameType_t)();
	extern Scr_LoadGameType_t Scr_LoadGameType;

	typedef void(*Scr_StartupGameType_t)();
	extern Scr_StartupGameType_t Scr_StartupGameType;

	typedef int(*Scr_LoadScript_t)(const char*);
	extern Scr_LoadScript_t Scr_LoadScript;

	typedef const char*(*Scr_GetString_t)(unsigned int index);
	extern Scr_GetString_t Scr_GetString;

	typedef scr_string_t(*Scr_GetConstString_t)(unsigned int index);
	extern Scr_GetConstString_t Scr_GetConstString;

	typedef const char* (*Scr_GetDebugString_t)(unsigned int index);
	extern Scr_GetDebugString_t Scr_GetDebugString;

	typedef float(*Scr_GetFloat_t)(unsigned int index);
	extern Scr_GetFloat_t Scr_GetFloat;

	typedef int(*Scr_GetInt_t)(unsigned int index);
	extern Scr_GetInt_t Scr_GetInt;

	typedef unsigned int(*Scr_GetObject_t)(unsigned int index);
	extern Scr_GetObject_t Scr_GetObject;

	typedef const char* (*Scr_GetTypeName_t)(unsigned int index);
	extern Scr_GetTypeName_t Scr_GetTypeName;

	typedef unsigned int(*Scr_GetNumParam_t)();
	extern Scr_GetNumParam_t Scr_GetNumParam;

	typedef unsigned int(*Scr_GetEntityId_t)(int entnum, unsigned int classnum);
	extern Scr_GetEntityId_t Scr_GetEntityId;

	typedef int(*Scr_GetFunctionHandle_t)(const char* filename, const char* name);
	extern Scr_GetFunctionHandle_t Scr_GetFunctionHandle;

	typedef int(*Scr_ExecThread_t)(int handle, unsigned int paramcount);
	extern Scr_ExecThread_t Scr_ExecThread;

	typedef int(*Scr_ExecEntThread_t)(gentity_s* ent, int handle, unsigned int paramcount);
	extern Scr_ExecEntThread_t Scr_ExecEntThread;

	typedef void(*Scr_FreeThread_t)(unsigned __int16 handle);
	extern Scr_FreeThread_t Scr_FreeThread;

	typedef void(*Scr_Notify_t)(gentity_t* ent, unsigned __int16 stringValue, unsigned int paramcount);
	extern Scr_Notify_t Scr_Notify;

	typedef void(*Scr_NotifyLevel_t)(unsigned __int16 stringValue, unsigned int paramcount);
	extern Scr_NotifyLevel_t Scr_NotifyLevel;

	typedef void(*Scr_ClearOutParams_t)();
	extern Scr_ClearOutParams_t Scr_ClearOutParams;

	typedef void(*Scr_RegisterFunction_t)(int func, const char* name);
	extern Scr_RegisterFunction_t Scr_RegisterFunction;

	typedef bool(*Scr_IsSystemActive_t)();
	extern Scr_IsSystemActive_t Scr_IsSystemActive;

	typedef int(*Scr_GetType_t)(unsigned int index);
	extern Scr_GetType_t Scr_GetType;

	typedef int(*Scr_GetPointerType_t)(unsigned int index);
	extern Scr_GetPointerType_t Scr_GetPointerType;

	typedef void(*Scr_Error_t)(const char*);
	extern Scr_Error_t Scr_Error;

	typedef void(*Scr_ObjectError_t)(const char*);
	extern Scr_ObjectError_t Scr_ObjectError;

	typedef void(*Scr_ParamError_t)(unsigned int paramIndex, const char*);
	extern Scr_ParamError_t Scr_ParamError;

	typedef void(*Scr_GetObjectField_t)(unsigned int classnum, int entnum, int offset);
	extern Scr_GetObjectField_t Scr_GetObjectField;

	typedef int(*Scr_SetObjectField_t)(unsigned int classnum, int entnum, int offset);
	extern Scr_SetObjectField_t Scr_SetObjectField;

	typedef void(*Scr_SetClientField_t)(gclient_s* client, int offset);
	extern Scr_SetClientField_t Scr_SetClientField;

	typedef void(*Scr_GetEntityField_t)(int entnum, int offset);
	extern Scr_GetEntityField_t Scr_GetEntityField;

	typedef void(*Scr_AddClassField_t)(unsigned int classnum, const char* name, unsigned int offset);
	extern Scr_AddClassField_t Scr_AddClassField;

	typedef gentity_s*(*GetPlayerEntity_t)(scr_entref_t entref);
	extern GetPlayerEntity_t GetPlayerEntity;

	typedef gentity_s*(*GetEntity_t)(scr_entref_t entref);
	extern GetEntity_t GetEntity;

	extern void IncInParam();

	extern void Scr_AddBool(int value);

	extern void RuntimeErrorInternal(int channel, const char* codePos, unsigned int index, const char* msg);

	extern void Scr_NotifyId(unsigned int id, unsigned __int16 stringValue, unsigned int paramcount);

	extern scr_const_t* scr_const;

	extern scrVmPub_t* scrVmPub;
	extern scrVarPub_t* scrVarPub;
}
