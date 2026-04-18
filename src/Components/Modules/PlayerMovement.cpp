#include "PlayerMovement.hpp"
#include "Events.hpp"
#include "GSC/Script.hpp"

constexpr int MASK_PLAYER_CLIP = 0x10000;
constexpr int MASK_BARRIER_CLIP = 0x400;

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
	const Game::dvar_t* PlayerMovement::BGDisableBarrierClips;
	const Game::dvar_t* PlayerMovement::BGLadderFixedInput;
	const Game::dvar_t* PlayerMovement::BGSprintIgnoreRepress;
	const Game::dvar_t* PlayerMovement::BGOmnimovement;
	const Game::dvar_t* PlayerMovement::BGDive;
	const Game::dvar_t* PlayerMovement::BGOmnimovementDive;

	Game::dvar_t** PlayerMovement::player_sprintStrafeSpeedScale = reinterpret_cast<Game::dvar_t**>(0x7ADCEC);

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

	void PlayerMovement::PmoveSingle_Stub(Game::pmove_s* pm)
	{
		if (BGDisableBarrierClips && BGDisableBarrierClips->current.enabled)
		{
			if (pm != nullptr && (pm->ps->pm_flags & Game::PMF_LADDER) == 0)
			{
				pm->tracemask &= ~MASK_PLAYER_CLIP;
				pm->tracemask |= MASK_BARRIER_CLIP;
			}
		}

		Game::PMoveSingle(pm);
	}


	void PlayerMovement::PM_CheckLadderMove_Stub(Game::pmove_s* pm, Game::pml_t* pml)
	{
		const auto should_fix_ladders = (
			BGDisableBarrierClips && 
			BGDisableBarrierClips->current.enabled && 
			pm != nullptr
		);

		if (should_fix_ladders)
		{
			pm->tracemask |= MASK_PLAYER_CLIP;
		}

		Game::PM_CheckLadderMove(pm, pml);

		if (should_fix_ladders && (pm->ps->pm_flags & Game::PMF_LADDER) == 0)
		{
			pm->tracemask &= ~MASK_PLAYER_CLIP;
		}
	}

	// Replaces PM_LadderMove's pitch-scaled climb multiplier
	// (v18 = clamp((forward[2] + 0.25) * 2.5, -1, +1)) at 0x573FEF with a
	// constant 1.0 so vertical wishvel becomes 0.5 * cmdScale * forwardmove.
	__declspec(naked) void PlayerMovement::PM_LadderMove_PitchStub()
	{
		__asm
		{
			// Fall back to stock if dvar is null or disabled
			mov eax, BGLadderFixedInput
			test eax, eax
			jz stockPath
			cmp byte ptr [eax + 0x10], 1
			jne stockPath

			// Force v18 = 1.0 and skip the ±1 clamp
			fld1
			fstp dword ptr [esp + 0xC]
			push 0x574034
			ret

		stockPath:
			// Game's code: fld pml->forward[2]; fadd 0.25
			fld dword ptr [ebp + 0x8]
			fadd qword ptr ds:[0x70D1E8]
			push 0x573FF8
			ret
		}
	}

	// Replaces the camera-relative right-vector projection inside PM_LadderMove
	// (Vec3ProjectOnPlane at 0x4C3130, single call site at 0x574061) with the
	// ladder-locked form: pml->right = (-Ly, Lx, 0) / |L_xy|.
	float* PlayerMovement::PM_LadderMove_RightVector_Hk(float* source, const float* ladderNormal, float* pmlRight)
	{
		if (BGLadderFixedInput && BGLadderFixedInput->current.enabled)
		{
			const float lx = ladderNormal[0];
			const float ly = ladderNormal[1];
			const float len2 = lx * lx + ly * ly;
			const float invLen = (len2 > 0.0f) ? (1.0f / std::sqrtf(len2)) : 1.0f;
			pmlRight[0] = -ly * invLen;
			pmlRight[1] =  lx * invLen;
			pmlRight[2] = 0.0f;
			return source; // match the original helper's return-first-arg contract
		}

		return Utils::Hook::Call<float*(float*, const float*, float*)>(0x4C3130)(source, ladderNormal, pmlRight);
	}

	// Disables PM_UpdateSprint's PC-only sprint re-press cancel when enabled.
	// Otherwise keeps the original behavior.
	__declspec(naked) void PlayerMovement::PM_UpdateSprint_RepressCallStub()
	{
		__asm
		{
			push eax
			pushfd

			mov eax, BGSprintIgnoreRepress
			test eax, eax
			jz stockPath                     // null during early init
			cmp byte ptr [eax + 0x10], 1     // dvar_t.current.enabled
			jne stockPath

			// Enabled: skip end-sprint call AND sprintButtonUpRequired write.
			popfd
			pop eax
			push 0x56EF64
			ret

		stockPath:
			popfd
			pop eax
			mov ecx, ebp
			mov eax, edi
			push 0x56EF5E
			push 0x56ECD0
			ret
		}
	}

	// Omnimovement

	int PlayerMovement::ComputeHorizontalIntent(int forwardSpeed, int rightSpeed)
	{
		if (!BGOmnimovement || !BGOmnimovement->current.enabled)
			return forwardSpeed;
		return std::max(std::abs(forwardSpeed), std::abs(rightSpeed));
	}

	// Stock PM_WalkMove behavior: rightmove *= player_sprintStrafeSpeedScale while sprinting
	void PlayerMovement::ApplyStockSprintStrafeScale(Game::pmove_s* pm)
	{
		if (!pm || !pm->ps)
		{
			return;
		}

		if ((pm->ps->pm_flags & Game::PMF_SPRINTING) == 0)
		{
			return;
		}

		const auto* dvar = *player_sprintStrafeSpeedScale;
		if (!dvar)
		{
			return;
		}

		float scaled = static_cast<float>(pm->cmd.rightmove) * dvar->current.value;
		pm->cmd.rightmove = static_cast<char>(std::clamp(scaled, -127.0f, 127.0f));
	}

	// Allow the sprint bit to be written to cmd.buttons when the back key is
	// held. Stock CL_KeyMove skips the sprint-bit update whenever the back
	// kbutton's active flag is set, which is what prevents sprint from
	// starting backward even after our PM_UpdateSprint hooks pass.
	__declspec(naked) void PlayerMovement::CL_KeyMove_SprintBit_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push eax
			mov eax, BGOmnimovement
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock
			pop eax

			// Bypass the back-active check, fall through to the sprint kbutton block
			push 0x5A6061
			ret

		doStock:
			pop eax

			// Original: cmp byte ptr [esi+4Ch], 0; jnz loc_5A6084
			cmp byte ptr [esi + 0x4C], 0
			jnz stockSkip

			push 0x5A6061
			ret

		stockSkip:
			push 0x5A6084
			ret
		}
	}

	// Replace the sprint-start gate's forwardmove argument with max(|forwardmove|, |rightmove|)
	__declspec(naked) void PlayerMovement::PM_SprintStartInterferingButtons_Stub()
	{
		__asm
		{
			pushfd
			push eax
			push ecx
			push edx

			// ComputeHorizontalIntent(forwardmove, rightmove)
			movsx eax, byte ptr [ebp + 0x1F]
			push eax
			push esi
			call ComputeHorizontalIntent
			add esp, 8

			// Substitute esi (forwardSpeed) with the horizontal-intent magnitude
			mov esi, eax

			pop edx
			pop ecx
			pop eax
			popfd

			// Jump into the original function
			push 0x56ED10
			ret
		}
	}

	// Replace the sprint-end gate's forwardmove argument with max(|forwardmove|, |rightmove|)
	__declspec(naked) void PlayerMovement::PM_SprintEndingButtons_Stub()
	{
		__asm
		{
			pushfd
			push eax
			push ecx

			// ComputeHorizontalIntent(forwardmove, rightmove)
			movsx eax, byte ptr [ebp + 0x1F]
			push eax
			push edx
			call ComputeHorizontalIntent
			add esp, 8

			// Substitute edx (forwardSpeed) with the horizontal-intent magnitude
			mov edx, eax

			pop ecx
			pop eax
			popfd

			// Jump into the original function
			push 0x56ED80
			ret
		}
	}

	// Skip the player_sprintStrafeSpeedScale attenuation in PM_WalkMove while sprinting
	__declspec(naked) void PlayerMovement::PM_WalkMove_SprintStrafeStub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push eax
			mov eax, BGOmnimovement
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock
			pop eax

			// Bypass the attenuation
			push 0x573289
			ret

		doStock:
			pop eax

			// Apply the original rightmove scaling
			push eax
			push ecx
			push edx
			push edi
			call ApplyStockSprintStrafeScale
			add esp, 4
			pop edx
			pop ecx
			pop eax

			push 0x573289
			ret
		}
	}

	// Widen the PM_SetMovementDir clamp from +/-90 to +/-127 at the prone/ladder branch
	__declspec(naked) void PlayerMovement::PM_SetMovementDir_ClampProneLadder_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push edx
			mov edx, BGOmnimovement
			test edx, edx
			jz stockClamp
			cmp byte ptr [edx + 0x10], 0
			jz stockClamp
			pop edx

			// Wide clamp: +/-127
			cmp eax, 127
			jle doneWide
			xor eax, eax
			test ecx, ecx
			setle al
			sub eax, 1
			and eax, 254
			add eax, -127
			mov ecx, eax
		doneWide:
			push 0x443421
			ret

		stockClamp:
			pop edx

			// Original clamp: +/-90
			cmp eax, 90
			jle doneStock
			xor eax, eax
			test ecx, ecx
			setle al
			sub eax, 1
			and eax, 180
			add eax, -90
			mov ecx, eax
		doneStock:
			push 0x443421
			ret
		}
	}

	// Widen the PM_SetMovementDir clamp from +/-90 to +/-127 at the generic branch
	__declspec(naked) void PlayerMovement::PM_SetMovementDir_ClampGeneric_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push edx
			mov edx, BGOmnimovement
			test edx, edx
			jz stockClamp
			cmp byte ptr [edx + 0x10], 0
			jz stockClamp
			pop edx

			// Wide clamp: +/-127
			cmp eax, 127
			jle doneWide
			xor eax, eax
			test ecx, ecx
			setle al
			sub eax, 1
			and eax, 254
			add eax, -127
			mov ecx, eax
		doneWide:
			push 0x4435C3
			ret

		stockClamp:
			pop edx

			// Original clamp: +/-90
			cmp eax, 90
			jle doneStock
			xor eax, eax
			test ecx, ecx
			setle al
			sub eax, 1
			and eax, 180
			add eax, -90
			mov ecx, eax
		doneStock:
			push 0x4435C3
			ret
		}
	}

	// Force PM_SetStrafeCondition to "not strafing" while sprinting so the forward-run anim plays
	__declspec(naked) void PlayerMovement::PM_SetStrafeCondition_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push eax
			mov eax, BGOmnimovement
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock

			// Only override while PMF_SPRINTING is set
			mov eax, [esi]
			test dword ptr [eax + 0xC], 0x4000
			jz doStock
			pop eax

			// BG_SetConditionValue(ps->clientNum, 7, 0, 0)
			mov eax, [esi]
			push 0
			push 0
			push 7
			push dword ptr [eax + 0x104]
			mov eax, BG_SetConditionValueAddr
			call eax
			add esp, 0x10
			ret

		doStock:
			pop eax

			// Original function body
			sub esp, 0xC
			movsx eax, byte ptr [esi + 0x1F]
			push 0x571617
			ret
		}
	}

	// Bypass the specialty_jumpdive perk check at the top of Jump_CheckDive
	__declspec(naked) void PlayerMovement::Jump_CheckDive_PerkGate_Stub()
	{
		__asm
		{
			// Check the value of BGDive
			push eax
			mov eax, BGDive
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock
			pop eax

			// Skip the perk check
			push 0x56D7C5
			ret

		doStock:
			pop eax

			// Original perk check
			test dword ptr [esi + 0x428], 0x100000
			jnz perkPresent

			// Original fail: no perk, return false
			xor al, al
			add esp, 0x6C
			ret

		perkPresent:
			push 0x56D7C5
			ret
		}
	}

	// Skip the player_backSpeedScale attenuation on diagonal backward sprint
	__declspec(naked) void PlayerMovement::PM_GetMaxSpeed_BackDiagonal_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push eax
			mov eax, BGOmnimovement
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock

			// Only skip while sprinting; walking backward keeps the penalty
			mov eax, [esp + 0x10]    // a3 = isSprinting, shifted by push eax
			test eax, eax
			jz doStock
			pop eax

			// Bypass the attenuation; 0x5738E2 pops the FPU leftovers
			push 0x5738E2
			ret

		doStock:
			pop eax

			// Skip if forwardmove >= 0
			test al, al
			jge stockSkip

			// FPU entry: ST0 = 0.5, ST1 = 1.0 (leftovers from the strafe blend
			// at 0x573881..0x573890). Reproduce: v9 *= (backScale + 1) * 0.5.
			mov eax, dword ptr ds:[0x7ADC44]
			fld dword ptr [eax + 0x10]  // push backScale; FPU: backScale, 0.5, 1.0
			faddp st(2), st             // ST(2) += backScale; FPU: 0.5, 1+backScale
			fmulp st(1), st             // multiply; FPU: (1+backScale)*0.5
			fmul dword ptr [esp]        // FPU: v9 * (1+backScale) * 0.5
			fstp dword ptr [esp]        // store, pop; FPU: empty
			push 0x5738E6
			ret

		stockSkip:
			// Tail-jump to 0x5738E2, which runs "fstp st(1); fstp st" to pop
			// the two FPU leftovers before falling into loc_5738E6.
			push 0x5738E2
			ret
		}
	}

	// Skip the player_backSpeedScale attenuation on pure backward sprint
	__declspec(naked) void PlayerMovement::PM_GetMaxSpeed_BackPure_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovement
			push eax
			mov eax, BGOmnimovement
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock

			// Only skip while sprinting; walking backward keeps the penalty
			mov eax, [esp + 0x10]
			test eax, eax
			jz doStock
			pop eax

			// Bypass the attenuation
			push 0x5738E6
			ret

		doStock:
			pop eax

			// Skip if forwardmove >= 0
			test al, al
			jge stockSkip

			// Original: v9 *= player_backSpeedScale
			mov edx, dword ptr ds:[0x7ADC44]
			fld dword ptr [edx + 0x10]
			fmul dword ptr [esp]
			fstp dword ptr [esp]
			push 0x5738E6
			ret

		stockSkip:
			push 0x5738E6
			ret
		}
	}

	// Skip the 0.3x backward-dive attenuation so backward dives launch at full perk_diveVelocity
	__declspec(naked) void PlayerMovement::Jump_CheckDive_BackDiveVelocity_Stub()
	{
		__asm
		{
			// Check the value of BGOmnimovementDive
			push eax
			mov eax, BGOmnimovementDive
			test eax, eax
			jz doStock
			cmp byte ptr [eax + 0x10], 0
			jz doStock
			pop eax

			// Bypass the attenuation, keep ST0 unchanged
			push 0x56D81C
			ret

		doStock:
			pop eax

			// Original: fmul 0.3, then round-trip through single precision
			fmul dword ptr ds:[0x71FE18]
			sub esp, 4
			fstp dword ptr [esp]
			fld dword ptr [esp]
			add esp, 4
			push 0x56D81C
			ret
		}
	}

	void PlayerMovement::RegisterMovementDvars()
	{
		PlayerDuckedSpeedScale = Game::Dvar_RegisterFloat("player_duckedSpeedScale",
			0.65f, 0.0f, 5.0f, Game::DVAR_CHEAT,
			"The scale applied to the player speed when ducking");

		PlayerProneSpeedScale = Game::Dvar_RegisterFloat("player_proneSpeedScale",
			0.15f, 0.0f, 5.0f, Game::DVAR_CHEAT,
			"The scale applied to the player speed when crawling");

		// 3arc naming convention
		CGUfoScaler = Game::Dvar_RegisterFloat("cg_ufo_scaler",
			6.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT,
			"The speed at which ufo camera moves");

		CGNoclipScaler = Game::Dvar_RegisterFloat("cg_noclip_scaler",
			3.0f, 0.001f, 1000.0f, Game::DVAR_CHEAT,
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

		BGDisableBarrierClips = Game::Dvar_RegisterBool("bg_disableBarrierClips",
			false, Game::DVAR_CODINFO, "Disable player collision with out of bound barriers");

		BGLadderFixedInput = Game::Dvar_RegisterBool("bg_ladderFixedInput",
			false, Game::DVAR_SYSTEMINFO, "Make ladder climb and strafe independent of view angle");

		BGSprintIgnoreRepress = Game::Dvar_RegisterBool("bg_sprintIgnoreRepress",
			false, Game::DVAR_SYSTEMINFO,
			"Ignore sprint-key re-presses while already sprinting (matches console behaviour)");

		BGOmnimovement = Game::Dvar_RegisterBool("bg_omnimovement",
			false, Game::DVAR_CODINFO,
			"Toggle omnidirectional sprint (sprint in any direction)");

		BGDive = Game::Dvar_RegisterBool("bg_dive",
			false, Game::DVAR_CODINFO,
			"Toggle dive-to-prone (bypasses specialty_jumpdive perk requirement)");

		BGOmnimovementDive = Game::Dvar_RegisterBool("bg_omnimovementDive",
			false, Game::DVAR_CODINFO,
			"Disable the backward-dive attenuation (requires bg_dive or specialty_jumpdive perk)");
	}

	PlayerMovement::PlayerMovement()
	{
		AssertOffset(Game::playerState_s, eFlags, 0xB0);
		AssertOffset(Game::playerState_s, pm_flags, 0xC);
		AssertOffset(Game::pmove_s, cmd, 0x4);
		AssertOffset(Game::usercmd_s, forwardmove, 0x1A);
		AssertOffset(Game::usercmd_s, rightmove, 0x1B);

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

		// Disable player collision with out of bound barriers
		Utils::Hook(0x4CFF5C, PmoveSingle_Stub, HOOK_CALL).install()->quick(); 			// single PmoveSingle call inside Pmove
		Utils::Hook(0x574AF4, PM_CheckLadderMove_Stub, HOOK_CALL).install()->quick(); 	// single PM_CheckLadderMove call inside PmoveSingle

		// View-independent ladder controls (opt-in via bg_ladderFixedInput)
		Utils::Hook(0x573FEF, PM_LadderMove_PitchStub, HOOK_JUMP).install()->quick();       // pitch-scaled climb rate block
		Utils::Hook(0x574061, PM_LadderMove_RightVector_Hk, HOOK_CALL).install()->quick();  // camera-relative right-vector projection

		// Console-style sprint hold (opt-in via bg_sprintIgnoreRepress)
		Utils::Hook(0x56EF59, PM_UpdateSprint_RepressCallStub, HOOK_JUMP).install()->quick();

		// Omnimovement - allow sprint bit to be set when pressing back
		Utils::Hook(0x5A605B, CL_KeyMove_SprintBit_Stub, HOOK_JUMP).install()->quick();

		// Omnimovement - sprint gate widening
		Utils::Hook(0x56EFBF, PM_SprintStartInterferingButtons_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x56EF29, PM_SprintEndingButtons_Stub, HOOK_CALL).install()->quick();

		// Full-speed lateral sprint
		Utils::Hook(0x573262, PM_WalkMove_SprintStrafeStub, HOOK_JUMP).install()->quick();

		// Widen the movementDir clamp for remote anim
		Utils::Hook(0x443408, PM_SetMovementDir_ClampProneLadder_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4435AA, PM_SetMovementDir_ClampGeneric_Stub, HOOK_JUMP).install()->quick();

		// Forward-run anim during sprint
		Utils::Hook(0x571610, PM_SetStrafeCondition_Stub, HOOK_JUMP).install()->quick();

		// Full-speed backward sprint
		Utils::Hook(0x573893, PM_GetMaxSpeed_BackDiagonal_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x5738A9, PM_GetMaxSpeed_BackPure_Stub, HOOK_JUMP).install()->quick();

		// Dive unlock
		Utils::Hook(0x56D7B3, Jump_CheckDive_PerkGate_Stub, HOOK_JUMP).install()->quick();

		// Full-velocity backward dive
		Utils::Hook(0x56D80E, Jump_CheckDive_BackDiveVelocity_Stub, HOOK_JUMP).install()->quick();

		GSC::Script::AddMethod("IsSprinting", GScr_IsSprinting);

		RegisterMovementDvars();
	}
}
