#pragma once

#include "Weapon.hpp"
#include "ModelCache.hpp"

namespace Components
{
  class ConfigStrings : public Component
  {
  public:
    // Configstrings past the game's, laid out as the x86 client lays them
    // out: the weapons past 1200 from where the game keeps its own past 1200,
    // then the models past the game's and then the rumbles, which start one
    // past the end of the models.
    //
    static const int BASEGAME_MAX_CONFIGSTRINGS = Game::MAX_CONFIGSTRINGS;

    static const int EXTRA_WEAPONS_FIRST = Game::CS_WEAPONFILES_EXTRA;
    static const int EXTRA_WEAPONS_LAST = EXTRA_WEAPONS_FIRST + Weapon::WEAPON_LIMIT - Weapon::EXTRA_CONFIGSTRING_WEAPONS_FIRST - 1;

    static const int EXTRA_MODELCACHE_FIRST = EXTRA_WEAPONS_LAST + 1;
    static const int EXTRA_MODELCACHE_LAST = EXTRA_MODELCACHE_FIRST + ModelCache::ADDITIONAL_GMODELS;

    static const int RUMBLE_CONFIGSTRINGS_COUNT = 32;
    static const int RUMBLE_FIRST = EXTRA_MODELCACHE_LAST + 1;
    static const int RUMBLE_LAST = RUMBLE_FIRST + RUMBLE_CONFIGSTRINGS_COUNT - 1;

    static const int MAX_CONFIGSTRINGS = EXTRA_MODELCACHE_FIRST + ModelCache::ADDITIONAL_GMODELS + RUMBLE_CONFIGSTRINGS_COUNT;

    static_assert(EXTRA_WEAPONS_LAST > Game::CS_WEAPONFILES_EXTRA_LAST);

    // Note that the gamestate sends an index in 13 bits.
    //
    static_assert(MAX_CONFIGSTRINGS <= 1 << 13);

    ConfigStrings();

    // Return true if index addresses a usable rumble configstring slot.
    //
    static bool IsValidRumbleIndex(int index);
    static const char* CL_GetRumbleConfigString(int index);
    static unsigned int SV_GetRumbleConfigStringConst(int index);
    static void SV_SetRumbleConfigString(int index, const char* data);

  private:
    static void PatchConfigStrings();

    // Return or set the configstring of a model or weapon, past the game's
    // in the extra configstrings that follow them.
    //
    static unsigned int SV_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition);
    static void SV_SetConfigString(int index, const char* data, Game::ConfigString basegameLastPosition, int extendedFirstPosition);
    static const char* CL_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition);

    static unsigned int SV_GetCachedModelConfigStringConst(int index);
    static void SV_SetCachedModelConfigString(int index, const char* data);
    static const char* CL_GetCachedModelConfigString(int index);

    // Handle the change of one of the extra configstrings, which the game
    // doesn't know. Return true if we handled it and false to leave it to
    // the game.
    //
    static bool CG_ParseExtraConfigStrings();
    static void CG_ConfigStringModified_Hk(int localClientNum);

    static void* SV_ClearServer(void* dest, int value, size_t size);
  };
}
