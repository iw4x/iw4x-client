#pragma once

#include "Weapon.hpp"
#include "ModelCache.hpp"
#include "Gamepad.hpp"

namespace Components
{
	class ConfigStrings
	{
	public:
		static const int BASEGAME_MAX_CONFIGSTRINGS = Game::MAX_CONFIGSTRINGS;
		static const int MAX_CONFIGSTRINGS =
			(BASEGAME_MAX_CONFIGSTRINGS
				+ Weapon::ADDED_WEAPONS
				+ ModelCache::ADDITIONAL_GMODELS
				+ Gamepad::RUMBLE_CONFIGSTRINGS_COUNT
			);

		static_assert(MAX_CONFIGSTRINGS < USHRT_MAX);

		ConfigStrings();

		static const char* CL_GetRumbleConfigString(int index);
		static unsigned int SV_GetRumbleConfigStringConst(int index);
		static void SV_SetRumbleConfigString(int index, const char* data);


	private:
		static void PatchConfigStrings();

		static void SV_SetConfigString(int index, const char* data, Game::ConfigString basegameLastPosition, int extendedFirstPosition);
		static unsigned int SV_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition);
		static const char* CL_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition);

		static const char* CL_GetCachedModelConfigString(int index);
		static int SV_GetCachedModelConfigStringConst(int index);
		static void SV_SetCachedModelConfigString(int index, const char* data);

		static const char* CL_GetWeaponConfigString(int index);
		static int SV_GetWeaponConfigStringConst(int index);
		static void SV_SetWeaponConfigString(int index, const char* data);

		static int CG_ParseExtraConfigStrings();
		static void CG_ParseConfigStrings();
		static int SV_ClearConfigStrings(void* dest, int value, int size);
	};
}
