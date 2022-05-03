#pragma once

namespace Components
{
	class Bullet : public Component
	{
	public:
		Bullet();

	private:
		static Dvar::Var BGSurfacePenetration;
		// Can't use Var class inside assembly stubs
		static Game::dvar_t* BGBulletRange;

		static float BG_GetSurfacePenetrationDepthStub(const Game::WeaponDef* weapDef, int surfaceType);

		static void Bullet_FireStub();
	};
}
