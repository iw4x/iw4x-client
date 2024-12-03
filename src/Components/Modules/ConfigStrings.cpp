#include <STDInclude.hpp>
#include "ConfigStrings.hpp"

namespace Components
{
	// Patch game state
	// The structure below is our own implementation of the gameState_t structure
	static struct ReallocatedGameState_t
	{
		int stringOffsets[ConfigStrings::MAX_CONFIGSTRINGS];
		char stringData[131072]; // MAX_GAMESTATE_CHARS
		int dataCount;
	} cl_gameState{};

	static short sv_configStrings[ConfigStrings::MAX_CONFIGSTRINGS]{};

	// New mapping (extra data)
	constexpr auto EXTRA_WEAPONS_FIRST = ConfigStrings::BASEGAME_MAX_CONFIGSTRINGS;
	constexpr auto EXTRA_WEAPONS_LAST = EXTRA_WEAPONS_FIRST + Weapon::ADDED_WEAPONS - 1;

	constexpr auto EXTRA_MODELCACHE_FIRST = EXTRA_WEAPONS_LAST + 1;
	constexpr auto EXTRA_MODELCACHE_LAST = EXTRA_MODELCACHE_FIRST + ModelCache::ADDITIONAL_GMODELS;

	constexpr auto RUMBLE_FIRST = EXTRA_MODELCACHE_LAST + 1;
	constexpr auto RUMBLE_LAST = RUMBLE_FIRST + Gamepad::RUMBLE_CONFIGSTRINGS_COUNT - 1; // TODO

	void ConfigStrings::PatchConfigStrings()
	{
		// bump clientstate fields
		Utils::Hook::Set<DWORD>(0x4347A7, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x4982F4, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x4F88B6, MAX_CONFIGSTRINGS); // Save file
		Utils::Hook::Set<DWORD>(0x5A1FA7, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x5A210D, MAX_CONFIGSTRINGS); // Game state
		Utils::Hook::Set<DWORD>(0x5A840E, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x5A853C, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x5AC392, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x5AC3F5, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x5AC542, MAX_CONFIGSTRINGS); // Game state
		Utils::Hook::Set<DWORD>(0x5EADF0, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x625388, MAX_CONFIGSTRINGS);
		Utils::Hook::Set<DWORD>(0x625516, MAX_CONFIGSTRINGS);

		Utils::Hook::Set(0x405B72, sv_configStrings);
		Utils::Hook::Set(0x468508, sv_configStrings);
		Utils::Hook::Set(0x47FD7C, sv_configStrings);
		Utils::Hook::Set(0x49830E, sv_configStrings);
		Utils::Hook::Set(0x498371, sv_configStrings);
		Utils::Hook::Set(0x4983D5, sv_configStrings);
		Utils::Hook::Set(0x4A74AD, sv_configStrings);
		Utils::Hook::Set(0x4BAE7C, sv_configStrings);
		Utils::Hook::Set(0x4BAEC3, sv_configStrings);
		Utils::Hook::Set(0x6252F5, sv_configStrings);
		Utils::Hook::Set(0x625372, sv_configStrings);
		Utils::Hook::Set(0x6253D3, sv_configStrings);
		Utils::Hook::Set(0x625480, sv_configStrings);
		Utils::Hook::Set(0x6254CB, sv_configStrings);

		// TODO: Check if all of these actually mark the end of the array
		// Only 2 actually mark the end, the rest is header data or so
		Utils::Hook::Set(0x405B8F, &sv_configStrings[ARRAYSIZE(sv_configStrings)]);
		Utils::Hook::Set(0x4A74C9, &sv_configStrings[ARRAYSIZE(sv_configStrings)]);

		Utils::Hook(0x405BBE, ConfigStrings::SV_ClearConfigStrings, HOOK_CALL).install()->quick();
		Utils::Hook(0x593CA4, ConfigStrings::CG_ParseConfigStrings, HOOK_CALL).install()->quick();

		// Weapon
		Utils::Hook(0x4BD52D, ConfigStrings::CL_GetWeaponConfigString, HOOK_CALL).install()->quick();
		Utils::Hook(0x45D19E , ConfigStrings::SV_SetWeaponConfigString, HOOK_CALL).install()->quick();

		// Cached Models
		Utils::Hook(0x589908, ConfigStrings::CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();
		Utils::Hook(0x450A30, ConfigStrings::CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();
		Utils::Hook(0x4503F6, ConfigStrings::CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();
		Utils::Hook(0x4504A0, ConfigStrings::CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();
		Utils::Hook(0x450A30, ConfigStrings::CL_GetCachedModelConfigString, HOOK_CALL).install()->quick();

		Utils::Hook(0x44F217, ConfigStrings::SV_GetCachedModelConfigStringConst, HOOK_CALL).install()->quick();
		Utils::Hook(0X418F93, ConfigStrings::SV_GetCachedModelConfigStringConst, HOOK_CALL).install()->quick();
		Utils::Hook(0x497B0A, ConfigStrings::SV_GetCachedModelConfigStringConst, HOOK_CALL).install()->quick();
		Utils::Hook(0x4F4493, ConfigStrings::SV_GetCachedModelConfigStringConst, HOOK_CALL).install()->quick();
		Utils::Hook(0x5FC46D, ConfigStrings::SV_GetCachedModelConfigStringConst, HOOK_JUMP).install()->quick();

		Utils::Hook(0x44F282, ConfigStrings::SV_SetCachedModelConfigString, HOOK_CALL).install()->quick();

		Utils::Hook::Set<DWORD>(0x44A333, sizeof(cl_gameState));
		Utils::Hook::Set<DWORD>(0x5A1F56, sizeof(cl_gameState));
		Utils::Hook::Set<DWORD>(0x5A2043, sizeof(cl_gameState));

		Utils::Hook::Set<DWORD>(0x5A2053, sizeof(cl_gameState.stringOffsets));
		Utils::Hook::Set<DWORD>(0x5A2098, sizeof(cl_gameState.stringOffsets));
		Utils::Hook::Set<DWORD>(0x5AC32C, sizeof(cl_gameState.stringOffsets));

		Utils::Hook::Set(0x4235AC, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x434783, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x44A339, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x44ADB7, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A1FE6, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A2048, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A205A, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A2077, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A2091, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A20D7, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A83FF, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A84B0, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5A84E5, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5AC333, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5AC44A, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5AC4F3, &cl_gameState.stringOffsets);
		Utils::Hook::Set(0x5AC57A, &cl_gameState.stringOffsets);

		Utils::Hook::Set(0x4235B7, &cl_gameState.stringData);
		Utils::Hook::Set(0x43478D, &cl_gameState.stringData);
		Utils::Hook::Set(0x44ADBC, &cl_gameState.stringData);
		Utils::Hook::Set(0x5A1FEF, &cl_gameState.stringData);
		Utils::Hook::Set(0x5A20E6, &cl_gameState.stringData);
		Utils::Hook::Set(0x5AC457, &cl_gameState.stringData);
		Utils::Hook::Set(0x5AC502, &cl_gameState.stringData);
		Utils::Hook::Set(0x5AC586, &cl_gameState.stringData);

		Utils::Hook::Set(0x5A2071, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5A20CD, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5A20DC, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5A20F3, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5A2104, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC33F, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC43B, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC450, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC463, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC471, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC4C3, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC4E8, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC4F8, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC50F, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC528, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC56F, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC580, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC592, &cl_gameState.dataCount);
		Utils::Hook::Set(0x5AC59F, &cl_gameState.dataCount);
	}

	void ConfigStrings::SV_SetConfigString(int index, const char* data, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
	{
		if (index > basegameLastPosition)
		{
			// we jump straight to the reallocated part of the array
			const auto relativeIndex = index - (basegameLastPosition + 1);
			index = extendedFirstPosition + relativeIndex;
		}

		// This will go back to our reallocated game state anyway
		return Game::SV_SetConfigstring(index, data);
	}

	void ConfigStrings::SV_SetWeaponConfigString(int index, const char* data)
	{
		SV_SetConfigString(index, data, Game::CS_WEAPONFILES_LAST, EXTRA_WEAPONS_FIRST);
	}

	const char* ConfigStrings::CL_GetRumbleConfigString(int index)
	{
		return CL_GetConfigString(RUMBLE_FIRST + index, Game::CS_LAST, Game::MAX_CONFIGSTRINGS);
	}

	unsigned int ConfigStrings::SV_GetRumbleConfigStringConst(int index)
	{
		return SV_GetConfigString(RUMBLE_FIRST +index, Game::CS_LAST, Game::MAX_CONFIGSTRINGS);
	}

	void ConfigStrings::SV_SetRumbleConfigString(int index, const char* data)
	{
		SV_SetConfigString(RUMBLE_FIRST + index, data, Game::CS_LAST, Game::MAX_CONFIGSTRINGS);
	}

	void ConfigStrings::SV_SetCachedModelConfigString(int index, const char* data)
	{
		SV_SetConfigString(index, data, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
	}

	unsigned int ConfigStrings::SV_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
	{
		if (index > basegameLastPosition)
		{
			// It's out of range, because we're loading more weapons than the basegame has
			// So we jump straight to the reallocated part of the array
			const auto relativeIndex = index - (basegameLastPosition + 1);

			index = extendedFirstPosition + relativeIndex;
		}

		// This will go back to our reallocated game state anyway
		return Game::SV_GetConfigstringConst(index);
	}

	const char* ConfigStrings::CL_GetConfigString(int index, Game::ConfigString basegameLastPosition, int extendedFirstPosition)
	{
		if (index > basegameLastPosition)
		{
			// It's out of range, because we're loading more weapons than the basegame has
			// So we jump straight to the reallocated part of the array
			const auto relativeIndex = index - (basegameLastPosition + 1);

			index = extendedFirstPosition + relativeIndex;
		}

		// This will go back to our reallocated game state anyway
		return Game::CL_GetConfigString(index);
	}

	const char* ConfigStrings::CL_GetCachedModelConfigString(int index)
	{
		return CL_GetConfigString(index, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
	}

	int ConfigStrings::SV_GetCachedModelConfigStringConst(int index)
	{
		return SV_GetConfigString(index, Game::CS_MODELS_LAST, EXTRA_MODELCACHE_FIRST);
	}

	const char* ConfigStrings::CL_GetWeaponConfigString(int index)
	{
		return CL_GetConfigString(index, Game::CS_WEAPONFILES_LAST, EXTRA_WEAPONS_FIRST);
	}

	int ConfigStrings::SV_GetWeaponConfigStringConst(int index)
	{
		return SV_GetConfigString(index, Game::CS_WEAPONFILES_LAST, EXTRA_WEAPONS_FIRST);
	}

	int ConfigStrings::CG_ParseExtraConfigStrings()
	{
		Command::ClientParams params;

		if (params.size() <= 1)
			return 0;

		char* end;
		const auto* input = params.get(1);
		auto index = std::strtol(input, &end, 10);

		if (input == end)
		{
			Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "{} is not a valid input\nUsage: {} <weapon index>\n",
				input, params.get(0));
			return 0;
		}

		// If it's one of our extra data
		// bypass parsing and handle it ourselves!
		if (index >= BASEGAME_MAX_CONFIGSTRINGS)
		{
			// Handle extra weapons
			if (index >= EXTRA_WEAPONS_FIRST && index <= EXTRA_WEAPONS_LAST)
			{
				// Recompute weapon index
				index = index - EXTRA_WEAPONS_FIRST + Weapon::BASEGAME_WEAPON_LIMIT;
				Game::CG_SetupWeaponConfigString(0, index);
			}
			// Handle extra models
			else if (index >= EXTRA_MODELCACHE_FIRST && index <= EXTRA_MODELCACHE_LAST)
			{
				const auto name = Game::CL_GetConfigString(index);

				const auto R_RegisterModel = 0x50FA00;

				index = index - EXTRA_MODELCACHE_FIRST + ModelCache::BASE_GMODEL_COUNT;
				ModelCache::gameModels_reallocated[index] = Utils::Hook::Call<Game::XModel*(const char*)>(R_RegisterModel)(name);
			}
			// Rumble!
			else if (index >= RUMBLE_FIRST && index <= RUMBLE_LAST)
			{
				// Apparently, there is nothing to do here. At least the game doesn't look like
				// it needs anything to be done. If there was to do rumble string replication
				//	between server and client, it would happen here
			}
			else
			{
				// Unknown for now?
				// Pass it to the game
				return 0;
			}

			// We handled it
			return 1;
		}

		// Pass it to the game
		return 0;
	}

	__declspec(naked) void ConfigStrings::CG_ParseConfigStrings()
	{
		__asm
		{
			push eax
			pushad

			call ConfigStrings::CG_ParseExtraConfigStrings

			mov[esp + 20h], eax
			popad

			pop eax

			test eax, eax
			jz continueParsing

			retn

			continueParsing :
			push 592960h
				retn
		}
	}

	int ConfigStrings::SV_ClearConfigStrings(void* dest, int value, int size)
	{
		memset(Utils::Hook::Get<void*>(0x405B72), value, MAX_CONFIGSTRINGS * sizeof(uint16_t));
		return Utils::Hook::Call<int(void*, int, int)>(0x4C98D0)(dest, value, size); // Com_Memset
	}


	ConfigStrings::ConfigStrings()
	{
		PatchConfigStrings();
	}
}
