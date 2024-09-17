#include <STDInclude.hpp>
#include "Bullet.hpp"

namespace Components
{
	Dvar::Var Bullet::BGSurfacePenetration;
	Game::dvar_t* Bullet::BGBulletRange;

	float Bullet::ContactPointSave[3];
	float Bullet::VCSave[3];
	float Bullet::CalcRicochetSave[3];

	float Bullet::ColorYellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
	float Bullet::ColorBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
	float Bullet::ColorOrange[] = {1.0f, 0.7f, 0.0f, 1.0f};

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

	void Bullet::BG_srand_Hk(unsigned int* pHoldrand)
	{
		*pHoldrand = static_cast<unsigned int>(std::rand());
	}

	void Bullet::BulletRicochet_Save(const float* contactPoint)
	{
		std::memcpy(ContactPointSave, contactPoint, sizeof(float[3]));
	}

	__declspec(naked) void Bullet::BulletRicochet_Stub()
	{
		__asm
		{
			pushad
			push [esp + 0x20 + 0xC]
			call BulletRicochet_Save
			add esp, 0x4
			popad

			// Game's code
			sub esp, 0x4C
			push ebp
			mov ebp, dword ptr [esp + 0x60]

			push 0x5D5B08
			ret
		}
	}

	void Bullet::_VectorMA_Stub(float* va, float scale, float* vb, float* vc)
	{
		vc[0] = va[0] + scale * vb[0];
		vc[1] = va[1] + scale * vb[1];
		vc[2] = va[2] + scale * vb[2];

		std::memcpy(VCSave, vc, sizeof(float[3]));
	}

	void Bullet::CalcRicochet_Stub(const float* incoming, const float* normal, float* result)
	{
		Utils::Hook::Call<void(const float*, const float*, float*)>(0x5D59F0)(incoming, normal, result);
		std::memcpy(CalcRicochetSave, result, sizeof(float[3]));
	}

	int Bullet::Bullet_Fire_Stub(Game::gentity_s* attacker, [[maybe_unused]] float spread, Game::weaponParms* wp, Game::gentity_s* weaponEnt, Game::PlayerHandIndex hand, int gameTime)
	{
		float tmp[3];

		Game::G_DebugStar(ContactPointSave, ColorYellow);

		tmp[0] = (CalcRicochetSave[0] * 100.0f) + VCSave[0];
		tmp[1] = (CalcRicochetSave[1] * 100.0f) + VCSave[1];
		tmp[2] = (CalcRicochetSave[2] * 100.0f) + VCSave[1];

		Game::G_DebugLineWithDuration(VCSave, tmp, ColorOrange, 1, 100);
		Game::G_DebugStar(tmp, ColorBlue);

		// Set the spread to 0 when drawing
		return Game::Bullet_Fire(attacker, 0.0f, wp, weaponEnt, hand, gameTime);
	}

	Bullet::Bullet()
	{
		BGSurfacePenetration = Dvar::Register<float>("bg_surfacePenetration", 0.0f,
			0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO, "Set to a value greater than 0 to override the surface penetration depth");
		BGBulletRange = Game::Dvar_RegisterFloat("bg_bulletRange", 8192.0f,
			0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO, "Max range used when calculating the bullet end position");

		Utils::Hook(0x4F6980, BG_GetSurfacePenetrationDepthStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x440340, Bullet_FireStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x440368, BG_srand_Hk, HOOK_CALL).install()->quick();

		std::memset(ContactPointSave, 0, sizeof(float[3]));
		std::memset(VCSave, 0, sizeof(float[3]));
		std::memset(CalcRicochetSave, 0, sizeof(float[3]));

#ifdef DEBUG_RIOT_SHIELD
		Utils::Hook(0x5D5B00, BulletRicochet_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x5D5B00 + 5, 3);

		Utils::Hook(0x5D5BBA, CalcRicochet_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x5D5BD7, _VectorMA_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x5D5C0B, Bullet_Fire_Stub, HOOK_CALL).install()->quick();
#endif
	}
}
