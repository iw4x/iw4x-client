#pragma once

namespace Components
{
	class Missile : public Component
	{
	public:
		Missile();

	private:
		static bool LiftOutOfSolid(const float* origin, const Game::gentity_s* inflictor, float* out);
		static void G_RadiusDamage_Hk(const float* origin, Game::gentity_s* inflictor, Game::gentity_s* attacker, float damage, float minDamage, float radius, float coneDot, const float* coneDir, Game::gentity_s* ignore, int meansOfDeath, int hitLoc);
	};
}
