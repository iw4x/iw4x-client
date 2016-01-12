#include "STDInclude.hpp"

namespace Components
{
	Game::XAssetHeader Weapon::WeaponFileLoad(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { nullptr };

		// Try loading raw weapon
		if (FileSystem::File(Utils::VA("weapons/mp/%s", filename)).Exists())
		{
			header.data = Game::BG_LoadWeaponDef_LoadObj(filename);
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
	}
}
