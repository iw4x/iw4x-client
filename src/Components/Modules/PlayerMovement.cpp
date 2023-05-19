#include <STDInclude.hpp>

#include "Events.hpp"
#include "PlayerMovement.hpp"

#include "GSC/Script.hpp"

namespace Components
{
	const Game::dvar_t* PlayerMovement::BGRocketJump;
	const Game::dvar_t* PlayerMovement::BGRocketJumpScale;
	const Game::dvar_t* PlayerMovement::BGPlayerEjection;
	const Game::dvar_t* PlayerMovement::BGPlayerCollision;
	const Game::dvar_t* PlayerMovement::BGClimbAnything;
	const Game::dvar_t* PlayerMovement::CGNoclipScaler;
	const Game::dvar_t* PlayerMovement::CGUfoScaler;
	const Game::dvar_t* PlayerMovement::PlayerSpectateSpeedScale;
	const Game::dvar_t* PlayerMovement::BGBounces;
	const Game::dvar_t* PlayerMovement::BGBouncesAllAngles;
	const Game::dvar_t* PlayerMovement::BGDisableLandingSlowdown;
	const Game::dvar_t* PlayerMovement::BGBunnyHopAuto;
	const Game::dvar_t* PlayerMovement::PlayerDuckedSpeedScale;
	const Game::dvar_t* PlayerMovement::PlayerProneSpeedScale;

	void PlayerMovement::PM_PlayerTraceStub(Game::pmove_s* pm, Game::trace_t* results, const float* start, const float* end, Game::Bounds* bounds, int passEntityNum, int contentMask)
	{
		Game::PM_playerTrace(pm, results, start, end, bounds, passEntityNum, contentMask);

		if (results && BGClimbAnything->current.enabled)
		{
			results[0].surfaceFlags |= SURF_LADDER;
		}
	}

	__declspec(naked) void PlayerMovement::PM_PlayerDuckedSpeedScaleStub()
	{
		__asm
		{
			push eax
			mov eax, PlayerDuckedSpeedScale
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			pop eax

			// Game's code
			pop ecx
			ret
		}
	}

	__declspec(naked) void PlayerMovement::PM_PlayerProneSpeedScaleStub()
	{
		__asm
		{
			push eax
			mov eax, PlayerProneSpeedScale
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			pop eax

			// Game's code
			pop ecx
			ret
		}
	}

	__declspec(naked) void PlayerMovement::PM_MoveScale_Noclip()
	{
		__asm
		{
			mov eax, CGNoclipScaler
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			fmul dword ptr [esp + 0xC]
			fstp dword ptr [esp + 0xC]

			push 0x56F43A
			ret
		}
	}

	__declspec(naked) void PlayerMovement::PM_MoveScale_Ufo()
	{
		__asm
		{
			mov eax, CGUfoScaler
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			fmul dword ptr [esp + 0xC]
			fstp dword ptr [esp + 0xC]

			push 0x56F44D
			ret
		}
	}

	__declspec(naked) void PlayerMovement::PM_MoveScale_Spectate()
	{
		__asm
		{
			mov eax, PlayerSpectateSpeedScale
			fld dword ptr [eax + 0x10] // dvar_t.current.value
			fmul dword ptr [esp + 0xC]
			fstp dword ptr [esp + 0xC]

			push 0x56F462
			ret
		}
	}

	__declspec(naked) void PlayerMovement::PM_StepSlideMoveStub()
	{
		__asm
		{
			// Check the value of BGBounces
			push eax

			mov eax, BGBounces
			mov eax, dword ptr [eax + 0x10]
			test eax, eax

			pop eax

			// Do not bounce if BGBounces is 0
			jle noBounce

			push eax

			mov eax, BGBouncesAllAngles
			mov eax, dword ptr [eax + 0x10]
			cmp eax, 2

			pop eax

			// Do not apply all angles patch if BGBouncesAllAngles is not set to "all surfaces"
			jne regularBounce

			push 0x4B1B7D
			ret
		
			// Bounce
		regularBounce:
			push 0x4B1B34
			ret

		noBounce:
			// Original game code
			cmp dword ptr [esp + 0x24], 0
			push 0x4B1B32
			ret
		}
	}

	// Double bounces
	void PlayerMovement::Jump_ClearState_Hk(Game::playerState_s* ps)
	{
		if (BGBounces->current.integer == DOUBLE)
		{
			return;
		}

		Game::Jump_ClearState(ps);
	}

	__declspec(naked) void PlayerMovement::PM_ProjectVelocityStub()
	{
		__asm
		{
			push eax
			mov eax, BGBouncesAllAngles
			mov eax, dword ptr [eax + 0x10]
			test eax, eax
			pop eax

			je noBounce

			// Force the bounce
			push 0x417B6F
			ret

		noBounce:
			fstp ST(0)
			pop esi
			add esp, 0x10
			ret
		}
	}

	Game::gentity_s* PlayerMovement::Weapon_RocketLauncher_Fire_Hk(Game::gentity_s* ent, unsigned int weaponIndex,
		float spread, Game::weaponParms* wp, const float* gunVel, Game::lockonFireParms* lockParms, bool magicBullet)
	{
		auto* result = Game::Weapon_RocketLauncher_Fire(ent, weaponIndex, spread, wp, gunVel, lockParms, magicBullet);

		if (ent->client && BGRocketJump->current.enabled && wp->weapDef->inventoryType != Game::WEAPINVENTORY_EXCLUSIVE)
		{
			const auto scale = BGRocketJumpScale->current.value;
			ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * scale;
			ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * scale;
			ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * scale;
		}

		return result;
	}

	int PlayerMovement::StuckInClient_Hk(Game::gentity_s* self)
	{
		if (BGPlayerEjection->current.enabled)
		{
			return Utils::Hook::Call<int(Game::gentity_s*)>(0x402D30)(self); // StuckInClient
		}

		return 0;
	}

	void PlayerMovement::CM_TransformedCapsuleTrace_Hk(Game::trace_t* results, const float* start, const float* end,
		const Game::Bounds* bounds, const Game::Bounds* capsule, int contents, const float* origin, const float* angles)
	{
		if (BGPlayerCollision->current.enabled)
		{
			Utils::Hook::Call<void(Game::trace_t*, const float*, const float*,
				const Game::Bounds*, const Game::Bounds*, int, const float*, const float*)>
				(0x478300)
				(results, start, end, bounds, capsule, contents, origin, angles); // CM_TransformedCapsuleTrace
		}
	}

	void PlayerMovement::PM_CrashLand_Stub(const float* v, float scale, const float* result)
	{
		if (!BGDisableLandingSlowdown->current.enabled)
		{
			Utils::Hook::Call<void(const float*, float, const float*)>(0x4C12B0)(v, scale, result);
		}
	}

	__declspec(naked) void PlayerMovement::Jump_Check_Stub()
	{
		using namespace Game;

		__asm
		{
			push eax
			mov eax, BGBunnyHopAuto
			cmp byte ptr [eax + 0x10], 1
			pop eax

			je autoHop

			// Game's code
			test dword ptr [ebp + 0x30], CMD_BUTTON_UP
			push 0x4E9890
			ret

		autoHop:
			push 0x4E989F
			ret
		}
	}

	void PlayerMovement::GScr_IsSprinting(const Game::scr_entref_t entref)
	{
		const auto* client = Game::GetEntity(entref)->client;
		if (!client)
		{
			Game::Scr_Error("IsSprinting can only be called on a player");
			return;
		}

		Game::Scr_AddBool(Game::PM_IsSprinting(&client->ps));
	}

	const Game::dvar_t* PlayerMovement::Dvar_RegisterSpectateSpeedScale(const char* dvarName, float value,
		float min, float max, unsigned __int16 /*flags*/, const char* description)
	{
		PlayerSpectateSpeedScale = Game::Dvar_RegisterFloat(dvarName, value,
			min, max, Game::DVAR_CODINFO, description);

		return PlayerSpectateSpeedScale;
	}

	void PlayerMovement::RegisterMovementDvars()
	{
		PlayerDuckedSpeedScale = Game::Dvar_RegisterFloat("player_duckedSpeedScale",
			0.65f, 0.0f, 5.0f, Game::DVAR_CODINFO,
			"The scale applied to the player speed when ducking");

		PlayerProneSpeedScale = Game::Dvar_RegisterFloat("player_proneSpeedScale",
			0.15f, 0.0f, 5.0f, Game::DVAR_CODINFO,
			"The scale applied to the player speed when crawling");

		// 3arc naming convention
		CGUfoScaler = Game::Dvar_RegisterFloat("cg_ufo_scaler",
			6.0f, 0.001f, 1000.0f, Game::DVAR_CODINFO,
			"The speed at which ufo camera moves");

		CGNoclipScaler = Game::Dvar_RegisterFloat("cg_noclip_scaler",
			3.0f, 0.001f, 1000.0f, Game::DVAR_CODINFO,
			"The speed at which noclip camera moves");

		BGDisableLandingSlowdown = Game::Dvar_RegisterBool("bg_disableLandingSlowdown",
			false, Game::DVAR_CODINFO, "Toggle landing slowdown");

		BGBunnyHopAuto = Game::Dvar_RegisterBool("bg_bunnyHopAuto",
			false, Game::DVAR_CODINFO, "Constantly jump when holding space");

		BGRocketJump = Game::Dvar_RegisterBool("bg_rocketJump",
			false, Game::DVAR_CODINFO, "Enable CoD4 rocket jumps");

		BGRocketJumpScale = Game::Dvar_RegisterFloat("bg_rocketJumpScale",
			64.0f, 1.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO,
			"The scale applied to the pushback force of a rocket");

		BGPlayerEjection = Game::Dvar_RegisterBool("bg_playerEjection",
			true, Game::DVAR_CODINFO, "Push intersecting players away from each other");

		BGPlayerCollision = Game::Dvar_RegisterBool("bg_playerCollision",
			true, Game::DVAR_CODINFO, "Push intersecting players away from each other");

		BGClimbAnything = Game::Dvar_RegisterBool("bg_climbAnything",
			false, Game::DVAR_CODINFO, "Treat any surface as a ladder");
	}

	PlayerMovement::PlayerMovement()
	{
		AssertOffset(Game::playerState_s, eFlags, 0xB0);
		AssertOffset(Game::playerState_s, pm_flags, 0xC);

		Events::OnDvarInit([]
		{
			static const char* bg_bouncesValues[] =
			{
				"disabled",
				"enabled",
				"double",
				nullptr,
			};

			static const char* bg_bouncesAllAnglesValues[] =
			{
				"disabled",
				"simple",
				"all surfaces",
				nullptr,
			};

			BGBounces = Game::Dvar_RegisterEnum("bg_bounces", bg_bouncesValues, DISABLED, Game::DVAR_CODINFO, "Bounce glitch settings");
			BGBouncesAllAngles = Game::Dvar_RegisterEnum("bg_bouncesAllAngles", bg_bouncesAllAnglesValues, DISABLED, Game::DVAR_CODINFO, "Force bounce from all angles");
		});

		// Hook Dvar_RegisterFloat. Only thing that's changed is that the 0x80 flag is not used
		Utils::Hook(0x448990, Dvar_RegisterSpectateSpeedScale, HOOK_CALL).install()->quick();

		// PM_CmdScaleForStance
		Utils::Hook(0x572D9B, PM_PlayerDuckedSpeedScaleStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x572DA5, PM_PlayerProneSpeedScaleStub, HOOK_JUMP).install()->quick();

		// Hook PM_MoveScale so we can add custom speed scale for Ufo and Noclip
		Utils::Hook(0x56F42C, PM_MoveScale_Noclip, HOOK_JUMP).install()->quick();
		Utils::Hook(0x56F43F, PM_MoveScale_Ufo, HOOK_JUMP).install()->quick();
		Utils::Hook(0x56F452, PM_MoveScale_Spectate, HOOK_JUMP).install()->quick();

		// Bounce logic
		Utils::Hook(0x4B1B2D, PM_StepSlideMoveStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x57383E, Jump_ClearState_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x417B66, PM_ProjectVelocityStub, HOOK_JUMP).install()->quick();

		// Rocket jump
		Utils::Hook(0x4A4F9B, Weapon_RocketLauncher_Fire_Hk, HOOK_CALL).install()->quick(); //  FireWeapon        

		// Hook StuckInClient & CM_TransformedCapsuleTrace
		// so we can prevent intersecting players from being pushed away from each other
		Utils::Hook(0x5D8153, StuckInClient_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x45A5BF, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // SV_ClipMoveToEntity
		Utils::Hook(0x5A0CAD, CM_TransformedCapsuleTrace_Hk, HOOK_CALL).install()->quick(); // CG_ClipMoveToEntity

		Utils::Hook(0x573F39, PM_PlayerTraceStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x573E93, PM_PlayerTraceStub, HOOK_CALL).install()->quick();

		Utils::Hook(0x570020, PM_CrashLand_Stub, HOOK_CALL).install()->quick(); // Vec3Scale
		Utils::Hook(0x4E9889, Jump_Check_Stub, HOOK_JUMP).install()->quick();

		GSC::Script::AddMethod("IsSprinting", GScr_IsSprinting);

		RegisterMovementDvars();
	}
}
