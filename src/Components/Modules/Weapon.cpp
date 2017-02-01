#include "STDInclude.hpp"

namespace Components
{
	Game::XAssetHeader Weapon::WeaponFileLoad(Game::XAssetType /*type*/, std::string filename)
	{
		Game::XAssetHeader header = { nullptr };

		// Try loading raw weapon
		if (FileSystem::File(Utils::String::VA("weapons/mp/%s", filename.data())).exists())
		{
			header.data = Game::BG_LoadWeaponDef_LoadObj(filename.data());
		}

		return header;
	}

	void Weapon::PatchLimit()
	{
		static int bg_weaponCompleteDefs[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B35E1, sizeof(bg_weaponCompleteDefs));
		Utils::Hook::Set(0x44CE07, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x45C671, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x45C6A4, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4B35E8, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4B3643, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4CE903, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4CE927, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4E6EC7, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B69A, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B910, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B925, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA22, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA51, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA78, bg_weaponCompleteDefs);

		static int bg_weaponDefs[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B35F2, sizeof(bg_weaponDefs));
		Utils::Hook::Set(0x42F697, bg_weaponDefs);
		Utils::Hook::Set(0x440EB7, bg_weaponDefs);
		Utils::Hook::Set(0x45C67D, bg_weaponDefs);
		Utils::Hook::Set(0x45C685, bg_weaponDefs);
		Utils::Hook::Set(0x45C6B0, bg_weaponDefs);
		Utils::Hook::Set(0x4B35F9, bg_weaponDefs);
		Utils::Hook::Set(0x4B364B, bg_weaponDefs);
		Utils::Hook::Set(0x57B693, bg_weaponDefs);
		Utils::Hook::Set(0x57B75D, bg_weaponDefs);
		Utils::Hook::Set(0x57B809, bg_weaponDefs);
		Utils::Hook::Set(0x57B8D3, bg_weaponDefs);
		Utils::Hook::Set(0x57B96D, bg_weaponDefs);
		Utils::Hook::Set(0x57BA32, bg_weaponDefs);

		static int bg_weapClips[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3603, sizeof(bg_weapClips));
		Utils::Hook::Set(0x4B360A, bg_weapClips);
		Utils::Hook::Set(0x4B3666, bg_weapClips);
		Utils::Hook::Set(0x57B993, bg_weapClips);
		Utils::Hook::Set(0x57B9E1, bg_weapClips);
		Utils::Hook::Set(0x57B9EA, bg_weapClips);

		static int bg_sharedAmmoCaps[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3614, sizeof(bg_sharedAmmoCaps));
		Utils::Hook::Set(0x414D27, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x4B361B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x4B365B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B83B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B870, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B87D, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B89C, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B8E0, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B901, bg_sharedAmmoCaps);

		static int bg_weapAmmoTypes[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3625, sizeof(bg_weapAmmoTypes));
		Utils::Hook::Set(0x4B362C, bg_weapAmmoTypes);
		Utils::Hook::Set(0x4B3650, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B782, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B7D1, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B7DA, bg_weapAmmoTypes);

//		static int weaponStrings[WEAPON_LIMIT * 2]; // string + hash
// 		Utils::Hook::Set<DWORD>(0x504E01, sizeof(weaponStrings));
// 		Utils::Hook::Set<DWORD>(0x4C77DC, sizeof(weaponStrings));
// 		Utils::Hook::Set(0x4B72DC, weaponStrings);
// 		Utils::Hook::Set(0x4C77E2, weaponStrings);
// 		Utils::Hook::Set(0x504E08, weaponStrings);
// 		Utils::Hook::Set(0x795584, weaponStrings);
// 		Utils::Hook::Set(0x4B72E8, &weaponStrings[1]);


		// Patch bg_weaponDefs on the stack
		Utils::Hook::Set<DWORD>(0x40C31D, sizeof(bg_weaponDefs));
		Utils::Hook::Set<DWORD>(0x40C32F, sizeof(bg_weaponDefs));

		Utils::Hook::Set<DWORD>(0x40C311, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C45F, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C478, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C434, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C434, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));

		// Move second buffer pointers
		Utils::Hook::Set<DWORD>(0x40C336, 0x12E4 + ((sizeof(bg_weaponDefs)) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x40C3C6, 0x12DC + ((sizeof(bg_weaponDefs)) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x40C3CE, 0x12DC + ((sizeof(bg_weaponDefs)) - (1200 * 4)));

		// Move arg0 pointers
		Utils::Hook::Set<DWORD>(0x40C365, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C44E, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C467, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));

		// Move arg4 pointers
		Utils::Hook::Set<DWORD>(0x40C344, 0x25B4 + ((sizeof(bg_weaponDefs) * 2) - (1200 * 4 * 2)));


		// Patch bg_sharedAmmoCaps on the stack
		Utils::Hook::Set<DWORD>(0x4F76E6, sizeof(bg_sharedAmmoCaps));

		Utils::Hook::Set<DWORD>(0x4F7621, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x4F76AF, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x4F76DA, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x4F77C5, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));

		// Move arg0 pointers
		Utils::Hook::Set<DWORD>(0x4F766D, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x4F76B7, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));
		Utils::Hook::Set<DWORD>(0x4F76FB, 0x12EC + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));

		// Move arg4 pointers
		Utils::Hook::Set<DWORD>(0x4F7630, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (1200 * 4)));

		// TODO: Check 0x486E51
	}

	Weapon::Weapon()
	{
		//Weapon::PatchLimit();

		// Intercept weapon loading
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_WEAPON, Weapon::WeaponFileLoad);

		// weapon asset existence check
		Utils::Hook::Nop(0x408228, 5); // find asset header
		Utils::Hook::Nop(0x408230, 5); // is asset default
		Utils::Hook::Nop(0x40823A, 2); // jump

		// Skip double loading for fs_game
		Utils::Hook::Set<BYTE>(0x4081FD, 0xEB);
	}
}
