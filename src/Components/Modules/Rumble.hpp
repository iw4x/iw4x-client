#pragma once

namespace Components
{
	class Rumble
	{

	public:
		static constexpr unsigned int MAX_ACTIVE_RUMBLES = 32;

		static Dvar::Var cl_debug_rumbles;

		Rumble();

	private:

		enum rumble_entity_event_t
		{
			EV_PLAY_RUMBLE_ON_ENT = Game::EV_MAX_EVENTS + 1,
			EV_PLAY_RUMBLE_ON_POS,
			EV_PLAY_RUMBLELOOP_ON_ENT,
			EV_PLAY_RUMBLELOOP_ON_POS,
			EV_STOP_RUMBLE,
			EV_STOP_ALL_RUMBLES,
		};

		static Dvar::Var cl_rumbleScale;

		static int GetRumbleInfoIndexFromName(const char* rumbleName);
		static Game::ActiveRumble* GetDuplicateRumbleIfExists(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray, Game::RumbleInfo* info, bool loop, Game::RumbleSourceType type, int entityNum, const float* pos);
		static int FindClosestToDyingActiveRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* activeRumbleArray);
		static Game::ActiveRumble* NextAvailableRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray);
		static void InvalidateActiveRumble(Game::ActiveRumble* ar);
		static void CalcActiveRumbles(int localClientNum, Game::ActiveRumble* activeRumbleArray, const float* rumbleReceiverPos);
		static void PlayRumbleInternal(int localClientNum, const char* rumbleName, bool loop, Game::RumbleSourceType type, int entityNum, const float* pos, double scale, bool updateDuplicates);
		static void Rumble_Strcpy(char* member, char* keyValue);
		static bool ParseRumbleGraph(Game::RumbleGraph* graph, const char* buffer, const char* fileName);
		static void ReadRumbleGraph(Game::RumbleGraph* graph, const char* rumbleFileName);
		static int LoadRumbleGraph(Game::RumbleGraph* rumbleGraphArray, Game::RumbleInfo* info, const char* highRumbleFileName, const char* lowRumbleFileName);
		static void RegisterWeaponRumbles(Game::WeaponDef* weapDef);
		static void RemoveInactiveRumbles(int localClientNum, Game::ActiveRumble* activeRumbleArray);

		static int G_RumbleIndex(const char* name);

		static void DebugRumbles();
		static void LoadConstantRumbleConfigStrings();
		static void SV_InitGameProgs_Hk(int arg);

		static Game::WeaponDef* BG_GetWeaponDef_RegisterRumble_Hk(unsigned int weapIndex);

		static void SCR_UpdateRumble();

		static void Scr_PlayRumbleOnEntity(Game::scr_entref_t entref);
		static void Scr_PlayRumbleLoopOnEntity(Game::scr_entref_t entref);
		static void Scr_PlayRumbleOnEntity_Internal(Game::scr_entref_t entref, rumble_entity_event_t event);
		static void Scr_PlayRumbleOnPosition();
		static void Scr_PlayRumbleLoopOnPosition();
		static void Scr_PlayRumbleOnPosition_Internal(rumble_entity_event_t);

		static void PlayNoteMappedRumbleAliases(int localClientNum, const char* noteName, Game::WeaponDef* weapDef);
		static void PlayNoteMappedSoundAliases_Stub();

		static void MeleeRumble_Hook(Game::gentity_s* targetEntity, Game::WeaponDef* weaponDef);
		static void MeleeRumble_Stub();


		static void CG_Turret_UpdateBarrelSpinRumble(int localClientNum, Game::centity_s* cent);
		static void CG_UpdateRumble(int localClientNum);
		static void CG_GetImpactEffectForWeapon_Hk(int localClientNum, const int sourceEntityNum, const int weaponIndex, const int surfType, const int impactFlags, Game::FxEffectDef** outFx, Game::snd_alias_list_t** outSnd);
		static void CG_ExplosiveImpactOnShieldEvent(int localClientNum);
		static void CG_ExplosiveSplashOnShieldEvent(int localClientNum, int weaponIndex);
		static void CG_SetRumbleReceiver();
		static void CG_UpdateEntInfo_Hk();
		static void CG_FireWeapon_Rumble(int localClientNum, Game::entityState_s* ent, Game::WeaponDef* weaponDef, bool isPlayerView);
		static void CG_FireWeapon_FireSoundHk();
		static int CG_LoadRumble(Game::RumbleGraph* rumbleGraphArray, Game::RumbleInfo* info, const char* rumbleName, int rumbleNameIndex);
		static void CG_RegisterRumbles(int localClientNum);
		static void CG_RegisterGraphics_Hk(int localClientNum, int b);
		static void CG_PlayRumbleOnPosition(int localClientNum, const char* rumbleName, const float* pos);
		static void CG_PlayRumbleOnEntity(int localClientNum, const char* rumbleName, int entityIndex);
		static void CG_PlayRumbleLoopOnPosition(int localClientNum, const char* rumbleName, const float* pos);
		static void CG_PlayRumbleLoopOnEntity(int localClientNum, const char* rumbleName, int entityIndex);
		static void CG_PlayRumbleOnClient(int localClientNum, const char* rumbleName);
		static void CG_PlayRumbleOnClientSafe(int localClientNum, const char* rumbleName);
		static void CG_EntityEvents_Stub();
		static void CG_StopRumble(int localClientNum, int entityNum, const char* rumbleName);
		static bool CG_EntityEvents_Hk(Game::centity_s* entity, rumble_entity_event_t event);
		static void CG_StopAllRumbles();

		static int CCS_GetChecksum_Hk();

		static void InitDvars();


	};
}
