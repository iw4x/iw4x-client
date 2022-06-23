#include <STDInclude.hpp>

namespace Components
{
	Dvar::Var Movement::PlayerSpectateSpeedScale;
	Dvar::Var Movement::CGUfoScaler;
	Dvar::Var Movement::CGNoclipScaler;
	Dvar::Var Movement::BGBouncesAllAngles;
	Dvar::Var Movement::BGRocketJump;
	Dvar::Var Movement::BGPlayerEjection;
	Dvar::Var Movement::BGPlayerCollision;
	Game::dvar_t* Movement::BGBounces;
	Game::dvar_t* Movement::PlayerDuckedSpeedScale;
	Game::dvar_t* Movement::PlayerProneSpeedScale;

	__declspec(naked) void Movement::PM_PlayerDuckedSpeedScaleStub()
	{
		__asm
		{
			push eax
			mov eax, Movement::PlayerDuckedSpeedScale
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			pop eax

			// Game's code
			pop ecx
			ret
		}
	}

	__declspec(naked) void Movement::PM_PlayerProneSpeedScaleStub()
	{
		__asm
		{
			push eax
			mov eax, Movement::PlayerProneSpeedScale
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			pop eax

			// Game's code
			pop ecx
			ret
		}
	}

	float Movement::PM_MoveScale(Game::playerState_s* ps, float fmove,
		float rmove, float umove)
	{
		assert(ps != nullptr);

		auto max = std::fabsf(fmove) < std::fabsf(rmove)
			? std::fabsf(rmove) : std::fabsf(fmove);

		if (std::fabsf(umove) > max)
		{
			max = std::fabsf(umove);
		}

		if (max == 0.0f)
		{
			return 0.0f;
		}   

		auto total = std::sqrtf(fmove * fmove
			+ rmove * rmove + umove * umove);
		auto scale = (static_cast<float>(ps->speed) * max) / (127.0f * total);

		if (ps->pm_flags & Game::PMF_WALKING || ps->leanf != 0.0f)
		{
			scale *= 0.4f;
		}

		switch (ps->pm_type)
		{
		case Game::pmtype_t::PM_NOCLIP:
			scale *= Movement::CGNoclipScaler.get<float>();
			break;
		case Game::pmtype_t::PM_UFO:
			scale *= Movement::CGUfoScaler.get<float>();
			break;
		case Game::pmtype_t::PM_SPECTATOR:
			scale *= Movement::PlayerSpectateSpeedScale.get<float>();
			break;
		default:
			break;
		}

		return scale;
	}

	__declspec(naked) void Movement::PM_MoveScaleStub()
	{
		__asm
		{
			pushad

			push [esp + 0xC + 0x20] // umove
			push [esp + 0xC + 0x20] // rmove
			push [esp + 0xC + 0x20] // fmove
			push esi // ps
			call Movement::PM_MoveScale
			add esp, 0x10

			popad
			ret
		}
	}

	__declspec(naked) void Movement::PM_StepSlideMoveStub()
	{
		__asm
		{
			// Check the value of BGBounces
			push ecx
			push eax

			mov eax, Movement::BGBounces
			mov ecx, dword ptr [eax + 0x10]
			test ecx, ecx

			pop eax
			pop ecx

			// Do not bounce if BGBounces is 0
			jle noBounce

			// Bounce
			push 0x4B1B34
			retn

		noBounce:
			// Original game code
			cmp dword ptr [esp + 0x24], 0
			push 0x4B1B48
			retn
		}
	}

	void Movement::PM_ProjectVelocityStub(const float* velIn, const float* normal, float* velOut)
	{
		const auto lengthSquared2D = velIn[0] * velIn[0] + velIn[1] * velIn[1];

		if (std::fabsf(normal[2]) < 0.001f || lengthSquared2D == 0.0)
		{
			velOut[0] = velIn[0];
			velOut[1] = velIn[1];
			velOut[2] = velIn[2];
			return;
		}

		auto newZ = velIn[0] * normal[0] + velIn[1] * normal[1];
		newZ = -newZ / normal[2];
		const auto lengthScale = std::sqrtf((velIn[2] * velIn[2] + lengthSquared2D)
				/ (newZ * newZ + lengthSquared2D));

		if (Movement::BGBouncesAllAngles.get<bool>()
			|| (lengthScale < 1.f || newZ < 0.f || velIn[2] > 0.f))
		{
			velOut[0] = velIn[0] * lengthScale;
			velOut[1] = velIn[1] * lengthScale;
			velOut[2] = newZ * lengthScale;
		}
	}

	// Double bounces
	void Movement::Jump_ClearState_Hk(Game::playerState_s* ps)
	{
		if (Movement::BGBounces->current.integer != Movement::DOUBLE)
		{
			Game::Jump_ClearState(ps);
		}
	}

	Game::gentity_s* Movement::Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex,
		float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool a7)
	{
		auto* result = Game::Weapon_RocketLauncher_Fire(ent, weaponIndex, spread, wp, gunVel, lockParms, a7);

		if (ent->client != nullptr && BGRocketJump.get<bool>())
		{
			ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * 64.0f;
			ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * 64.0f;
			ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * 64.0f;
		}

		return result;
	}

	int Movement::StuckInClient_Hk(Game::gentity_s* self)
	{
		if (Movement::BGPlayerEjection.get<bool>())
		{
			return Utils::Hook::Call<int(Game::gentity_s*)>(0x402D30)(self); // StuckInClient
		}

		return 0;
	}

	void Movement::CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end,
		const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles)
	{
		if (Movement::BGPlayerCollision.get<bool>())
		{
			Utils::Hook::Call<void(Game::trace_t*, const float*, const float*,
				const Game::Bounds*, const Game::Bounds*, int, const float*, const float*)>
				(0x478300)
				(results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
		}
	}

	Game::dvar_t* Movement::Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value,
		float min, float max, unsigned __int16 /*flags*/, const char* description)
	{
		Movement::PlayerSpectateSpeedScale = Dvar::Register<float>(dvarName, value,
			min, max, Game::DVAR_CHEAT | Game::DVAR_CODINFO, description);

		return Movement::PlayerSpectateSpeedScale.get<Game::dvar_t*>();
	}

	Movement::Movement()
	{
		Scheduler::Once([]
		{
			static const char* bg_bouncesValues[] =
			{
				"disabled",
				"enabled",
				"double",
				nullptr
			};

			Movement::PlayerDuckedSpeedScale = Game::Dvar_RegisterFloat("player_duckedSpeedScale",
				0.65f, 0.0f, 5.0f, Game::DVAR_CHEAT | Game::DVAR_CODINFO,
				"The scale applied to the player speed when ducking");

			Movement::PlayerProneSpeedScale = Game::Dvar_RegisterFloat("player_proneSpeedScale",
				0.15f, 0.0f, 5.0f, Game::DVAR_CHEAT | Game::DVAR_CODINFO,
				"The scale applied to the player speed when crawling");

			// 3arc naming convention
			Movement::CGUfoScaler = Dvar::Register<float>("cg_ufo_scaler",
				6.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT | Game::DVAR_CODINFO,
				"The speed at which ufo camera moves");

			Movement::CGNoclipScaler = Dvar::Register<float>("cg_noclip_scaler",
				3.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT | Game::DVAR_CODINFO,
				"The speed at which noclip camera moves");

			Movement::BGBounces = Game::Dvar_RegisterEnum("bg_bounces",
				bg_bouncesValues, Movement::DISABLED, Game::DVAR_CODINFO, "Bounce glitch settings");

			Movement::BGBouncesAllAngles = Dvar::Register<bool>("bg_bouncesAllAngles",
				false, Game::DVAR_CODINFO, "Force bounce from all angles");

			Movement::BGRocketJump = Dvar::Register<bool>("bg_rocketJump",
				false, Game::DVAR_CODINFO, "Enable CoD4 rocket jumps");

			Movement::BGPlayerEjection = Dvar::Register<bool>("bg_playerEjection",
				true, Game::DVAR_CODINFO, "Push intersecting players away from each other");

			Movement::BGPlayerCollision = Dvar::Register<bool>("bg_playerCollision",
				true, Game::DVAR_CODINFO, "Push intersecting players away from each other");
		}, Scheduler::Pipeline::MAIN);

		// Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used.
		Utils::Hook(0x448990, Movement::Dvar_RegisterSpectateSpeedScale, HOOK_CALL).install()->quick();

		// PM_CmdScaleForStance
		Utils::Hook(0x572D9B, Movement::PM_PlayerDuckedSpeedScaleStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x572DA5, Movement::PM_PlayerProneSpeedScaleStub, HOOK_JUMP).install()->quick();

		// Hook PM_MoveScale so we can add custom speed scale for Ufo and Noclip
		Utils::Hook(0x56F845, Movement::PM_MoveScaleStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x56FABD, Movement::PM_MoveScaleStub, HOOK_CALL).install()->quick();

		// Bounce logic
		Utils::Hook(0x4B1B2D, Movement::PM_StepSlideMoveStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x57383E, Movement::Jump_ClearState_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x4B1B97, Movement::PM_ProjectVelocityStub, HOOK_CALL).install()->quick();

		// Rocket jump
		Utils::Hook(0x4A4F9B, Movement::Weapon_RocketLauncher_Fire_Hk, HOOK_CALL).install()->quick(); //  FireWeapon        

		// Hook StuckInClient & CM_TransformedCapsuleTrace 
		// so we can prevent intersecting players from being pushed away from each other
		Utils::Hook(0x5D8153, Movement::StuckInClient_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x45A5BF, Movement::CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // SV_ClipMoveToEntity
		Utils::Hook(0x5A0CAD, Movement::CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // CG_ClipMoveToEntity
	}
}
