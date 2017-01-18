#include "STDInclude.hpp"

namespace Components
{
	Game::XAssetHeader Weapon::WeaponFileLoad(Game::XAssetType /*type*/, std::string filename)
	{
		Game::XAssetHeader header = { 0 };

		// Try loading raw weapon
		if (FileSystem::File(Utils::String::VA("weapons/mp/%s", filename.data())).exists())
		{
			header.data = Game::BG_LoadWeaponDef_LoadObj(filename.data());
		}

		return header;
	}

	Weapon::Weapon()
	{
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
