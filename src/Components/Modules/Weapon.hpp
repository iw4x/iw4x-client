#pragma once

// Increase the weapon limit
// Was 1200 before
#define WEAPON_LIMIT 2400

namespace Components
{
	class Weapon : public Component
	{
	public:
		Weapon();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Weapon"; };
#endif

	private:
		static Game::XAssetHeader WeaponFileLoad(Game::XAssetType type, std::string filename);
		static void PatchLimit();

		static const char* GetWeaponConfigString(int index);
		static void SaveRegisteredWeapons();
	};
}
