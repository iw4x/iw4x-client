#pragma once

namespace Components
{
	class Melee : public Component
	{
	public:
		Melee();

		static void Weapon_Melee(Game::gentity_s* attacker, Game::weaponParms* wpParms, float player_meleeRange, float player_meleeWidth, float player_meleeHeight, int gameTime);
		static void FireWeaponMelee(Game::gentity_s* ent, int gameTime);
	};
}
