#pragma once

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
	};
}
