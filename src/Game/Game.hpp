#pragma once

#include "BothGames.hpp"
#include "Client.hpp"
#include "Common.hpp"
#include "Database.hpp"
#include "FileSystem.hpp"
#include "Functions.hpp"
#include "Dvars.hpp"
#include "PlayerMovement.hpp"
#include "PreProcessor.hpp"
#include "Script.hpp"
#include "Server.hpp"
#include "System.hpp"
#include "Zone.hpp"

namespace Game
{
	typedef void(*G_LogPrintf_t)(const char* fmt, ...);
	extern G_LogPrintf_t G_LogPrintf;

	typedef unsigned int(*G_GetWeaponIndexForName_t)(const char*);
	extern G_GetWeaponIndexForName_t G_GetWeaponIndexForName;

	typedef void(*G_SpawnEntitiesFromString_t)();
	extern G_SpawnEntitiesFromString_t G_SpawnEntitiesFromString;

	typedef gentity_s* (*G_Spawn_t)();
	extern G_Spawn_t G_Spawn;

	typedef gentity_s* (*G_TempEntity_t)(float *origin, Game::entity_event_t  entity);
	extern G_TempEntity_t G_TempEntity;

	typedef void (*G_AddEvent_t)(gentity_s *ent, entity_event_t event, unsigned int eventParm);
	extern G_AddEvent_t G_AddEvent;



	typedef void(*G_FreeEntity_t)(gentity_s* ed);
	extern G_FreeEntity_t G_FreeEntity;

	typedef void(*G_SpawnItem_t)(gentity_s* ent, int item);
	extern G_SpawnItem_t G_SpawnItem;

	typedef void(*G_GetItemClassname_t)(int item, gentity_s* ent);
	extern G_GetItemClassname_t G_GetItemClassname;

	typedef void(*G_PrintEntities_t)();
	extern G_PrintEntities_t G_PrintEntities;

	typedef const char*(*G_GetEntityTypeName_t)(const gentity_s* ent);
	extern G_GetEntityTypeName_t G_GetEntityTypeName;

	typedef int(*G_LocalizedStringIndex_t)(const char* string);
	extern G_LocalizedStringIndex_t G_LocalizedStringIndex;

	typedef void(*G_DebugLineWithDuration_t)(const float* start, const float* end, const float* color, int depthTest, int duration);
	extern G_DebugLineWithDuration_t G_DebugLineWithDuration;

	typedef int(*G_PointContents_t)(const float* point, int passEntityNum, int contentMask);
	extern G_PointContents_t G_PointContents;

	typedef void(*G_TraceCapsule_t)(trace_t* results, const float* start, const float* end, const Bounds* bounds, int passEntityNum, int contentMask);
	extern G_TraceCapsule_t G_TraceCapsule;

	typedef void(*G_RadiusDamage_t)(const float* origin, gentity_s* inflictor, gentity_s* attacker, float damage, float minDamage, float radius, float coneDot, const float* coneDir, gentity_s* ignore, int meansOfDeath, int hitLoc);
	extern G_RadiusDamage_t G_RadiusDamage;

	extern gentity_s* g_entities;
	extern bool* g_quitRequested;

	extern Bounds* bounds_origin;

	extern char(*g_cmdlineCopy)[1024];

	// This does not belong anywhere else
	extern NetField* clientStateFields;
	extern size_t clientStateFieldsCount;
	extern MssLocal* milesGlobal;
	extern WinVars_t* g_wv;

	extern const char* origErrorMsg;

	extern XModel* G_GetModel(int index);

	extern void G_DebugStar(const float* point, const float* color);
}
