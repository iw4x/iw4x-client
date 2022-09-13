#include <STDInclude.hpp>

namespace Components
{
	Dvar::Var Bullet::BGSurfacePenetration;
	Game::dvar_t* Bullet::BGBulletRange;

	float Bullet::BG_GetSurfacePenetrationDepthStub(const Game::WeaponDef* weapDef, int surfaceType)
	{
		assert(weapDef);
		assert(weapDef->penetrateType != Game::PENETRATE_TYPE_NONE);
		AssertIn(weapDef->penetrateType, Game::PENETRATE_TYPE_COUNT);
		AssertIn(surfaceType, Game::SURF_TYPE_COUNT);

		const auto penetrationDepth = BGSurfacePenetration.get<float>();
		if (penetrationDepth > 0.0f)
		{
			// Custom depth
			return penetrationDepth;
		}

		// Game's code
		if (surfaceType != Game::SURF_TYPE_DEFAULT)
		{
			return (*Game::penetrationDepthTable)[weapDef->penetrateType][surfaceType];
		}

		return 0.0f;
	}

	__declspec(naked) void Bullet::Bullet_FireStub()
	{
		__asm
		{
			push eax
			mov eax, BGBulletRange
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			pop eax

			push 0x440346
			retn
		}
	}

	Bullet::Bullet()
	{
		BGSurfacePenetration = Dvar::Register<float>("bg_surfacePenetration", 0.0f,
			0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO,
			"Set to a value greater than 0 to override the surface penetration depth");
		BGBulletRange = Game::Dvar_RegisterFloat("bg_bulletRange", 8192.0f,
			0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO,
			"Max range used when calculating the bullet end position");

		Utils::Hook(0x4F6980, BG_GetSurfacePenetrationDepthStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x440340, Bullet_FireStub, HOOK_JUMP).install()->quick();
	}
}
