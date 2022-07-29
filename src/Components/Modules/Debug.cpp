#include <STDInclude.hpp>

namespace Components
{
	Dvar::Var Debug::DebugOverlay;

	Game::dvar_t** Debug::PlayerDebugHealth = reinterpret_cast<Game::dvar_t**>(0x7A9F7C);

	const char* Debug::PMFlagsValues[] =
	{
		"PMF_PRONE",
		"PMF_DUCKED",
		"PMF_MANTLE",
		"PMF_LADDER",
		"PMF_SIGHT_AIMING",
		"PMF_BACKWARDS_RUN",
		"PMF_WALKING",
		"PMF_TIME_HARDLANDING",
		"PMF_TIME_KNOCKBACK",
		"PMF_PRONEMOVE_OVERRIDDEN",
		"PMF_RESPAWNED",
		"PMF_FROZEN",
		"PMF_LADDER_FALL",
		"PMF_JUMPING",
		"PMF_SPRINTING",
		"PMF_SHELLSHOCKED",
		"PMF_MELEE_CHARGE",
		"PMF_NO_SPRINT",
		"PMF_NO_JUMP",
		"PMF_REMOTE_CONTROLLING",
		"PMF_ANIM_SCRIPTED",
		"PMF_UNK1",
		"PMF_DIVING",
	};

	const char* Debug::POFlagsValues[] =
	{
		"POF_INVULNERABLE",
		"POF_REMOTE_EYES",
		"POF_LASER_ALTVIEW",
		"POF_THERMAL_VISION",
		"POF_THERMAL_VISION_OVERLAY_FOF",
		"POF_REMOTE_CAMERA_SOUNDS",
		"POF_ALT_SCENE_REAR_VIEW",
		"POF_ALT_SCENE_TAG_VIEW",
		"POF_SHIELD_ATTACHED_TO_WORLD_MODEL",
		"POF_DONT_LERP_VIEWANGLES",
		"POF_EMP_JAMMED",
		"POF_FOLLOW",
		"POF_PLAYER",
		"POF_SPEC_ALLOW_CYCLE",
		"POF_SPEC_ALLOW_FREELOOK",
		"POF_AC130",
		"POF_COMPASS_PING",
		"POF_ADS_THIRD_PERSON_TOGGLE",
	};

	const char* Debug::PLFlagsValues[] =
	{
		"PLF_ANGLES_LOCKED",
		"PLF_USES_OFFSET",
		"PLF_WEAPONVIEW_ONLY",
	};

	const char* Debug::PEFlagsValues[] =
	{
		"EF_NONSOLID_BMODEL",
		"EF_TELEPORT_BIT",
		"EF_CROUCHING",
		"EF_PRONE",
		"EF_UNK1",
		"EF_NODRAW",
		"EF_TIMED_OBJECT",
		"EF_VOTED",
		"EF_TALK",
		"EF_FIRING",
		"EF_TURRET_ACTIVE_PRONE",
		"EF_TURRET_ACTIVE_DUCK",
		"EF_LOCK_LIGHT_VIS",
		"EF_AIM_ASSIST",
		"EF_LOOP_RUMBLE",
		"EF_LASER_SIGHT",
		"EF_MANTLE",
		"EF_DEAD",
		"EF_ADS",
		"EF_NEW",
		"EF_VEHICLE_ACTIVE",
		"EF_JAMMING",
		"EF_COMPASS_PING",
		"EF_SOFT",
	};

	std::string Debug::BuildPMFlagsString(const Game::playerState_s* ps)
	{
		std::string result;

		for (size_t i = 0; i < ARRAYSIZE(PMFlagsValues); ++i)
		{
			result.append(Utils::String::VA("^%c%s\n", ((ps->pm_flags & (1 << i)) == 0) ? '7' : '2', PMFlagsValues[i]));
		}

		return result;
	}

	std::string Debug::BuildPOFlagsString(const Game::playerState_s* ps)
	{
		std::string result;

		for (size_t i = 0; i < ARRAYSIZE(POFlagsValues); ++i)
		{
			result.append(Utils::String::VA("^%c%s\n", ((ps->otherFlags & (1 << i)) == 0) ? '7' : '2', POFlagsValues[i]));
		}

		return result;
	}

	std::string Debug::BuildPLFlagsString(const Game::playerState_s* ps)
	{
		std::string result;

		for (size_t i = 0; i < ARRAYSIZE(PLFlagsValues); ++i)
		{
			result.append(Utils::String::VA("^%c%s\n", ((ps->linkFlags & (1 << i)) == 0) ? '7' : '2', PLFlagsValues[i]));
		}

		return result;
	}

	std::string Debug::BuildPEFlagsString(const Game::playerState_s* ps)
	{
		std::string result;

		for (size_t i = 0; i < ARRAYSIZE(PEFlagsValues); ++i)
		{
			result.append(Utils::String::VA("^%c%s\n", ((ps->eFlags & (1 << i)) == 0) ? '7' : '2', PEFlagsValues[i]));
		}

		return result;
	}

	void Debug::CG_Debug_DrawPSFlags(const int localClientNum)
	{
		const auto* cgameGlob = Game::cgArray;
		auto* const scrPlace = Game::ScrPlace_GetActivePlacement(localClientNum);

		constexpr auto maxChars = 4096;
		constexpr float colorWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};

		auto* const font1 = Game::UI_GetFontHandle(scrPlace, 6, MY_SCALE_2);
		auto* const font2 = Game::UI_GetFontHandle(scrPlace, 6, MY_SCALE2);

		Game::UI_DrawText(scrPlace, "Client View of Flags", maxChars, font2, -60.0f, 0, 1, 1,
			MY_SCALE2, colorWhite, 1);

		const auto pmf = BuildPMFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pmf.data(), maxChars, font1, 30.0f, 20.0f, 1, 1, MY_SCALE_2, colorWhite, 3);

		const auto pof = BuildPOFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pof.data(), maxChars, font1, 350.0f, 20.0f, 1, 1, MY_SCALE_2, colorWhite, 3);

		const auto plf = BuildPLFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, plf.data(), maxChars, font1, 350.0f, 250.0f, 1, 1, MY_SCALE_2, colorWhite, 3);

		const auto pef = BuildPEFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pef.data(), maxChars, font1, 525.0f, 20.0f, 1, 1, MY_SCALE_2, colorWhite, 3);
	}

	void Debug::CG_DrawDebugPlayerHealth(const int localClientNum)
	{
		float healtha;
		constexpr float color1[] = {0.0f, 0.0f, 0.0f, 1.0f};
		constexpr float color2[] = {0.0f, 1.0f, 0.0f, 1.0f};

		assert((*PlayerDebugHealth)->current.enabled);
		const auto* cgameGlob = Game::cgArray;

		if (cgameGlob->predictedPlayerState.stats[0] && cgameGlob->predictedPlayerState.stats[2])
		{
			const auto health = static_cast<float>(cgameGlob->predictedPlayerState.stats[0]) / static_cast<float>(cgameGlob->predictedPlayerState.stats[2]);

			const auto stats = ((health - 1.0f) < 0.0f)
				? static_cast<float>(cgameGlob->predictedPlayerState.stats[0]) / static_cast<float>(cgameGlob->predictedPlayerState.stats[2])
				: 1.0f;

			healtha = ((0.0f - health) < 0.0f)
				? stats
				: 0.0f;
		}
		else
		{
			healtha = 0.0f;
		}

		auto* const scrPlace = Game::ScrPlace_GetActivePlacement(localClientNum);
		Game::CL_DrawStretchPic(scrPlace, 10.0f, 10.0f, 100.0f, 10.0f, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f, color1, *Game::whiteMaterial);
		Game::CL_DrawStretchPic(scrPlace, 10.0f, 10.0f, 100.0f * healtha, 10.0f, 1, 1, 0.0f, 0.0f, healtha, 1.0f, color2, *Game::whiteMaterial);
	}

	void Debug::CG_DrawDebugOverlays_Hk(const int localClientNum)
	{
		switch (DebugOverlay.get<int>())
		{
		case 2:
			CG_Debug_DrawPSFlags(localClientNum);
			break;
		default:
			break;
		}

		if ((*PlayerDebugHealth)->current.enabled)
		{
			CG_DrawDebugPlayerHealth(localClientNum);
		}
	}

	void Debug::Com_Assert_f()
	{
		assert(("a", false));
	}

	void Debug::CL_InitDebugDvars()
	{
		static const char* debugOverlayNames_0[] =
		{
			"Off",
			"ViewmodelInfo",
			"Playerstate Flags",
			"Entity Counts",
			"Controllers",
			"FontTest",
			nullptr,
		};

		DebugOverlay = Game::Dvar_RegisterEnum("debugOverlay", debugOverlayNames_0, 0,
			Game::DVAR_NONE, "Toggles the display of various debug info.");
	}

	Debug::Debug()
	{
		Scheduler::Once(CL_InitDebugDvars, Scheduler::Pipeline::MAIN);

		// Hook end of CG_DrawDebugOverlays (This is to ensure some checks are done before our hook is executed).
		Utils::Hook(0x49CB0A, CG_DrawDebugOverlays_Hk, HOOK_JUMP).install()->quick();

		Utils::Hook::Set<void(*)()>(0x60BCEA, Com_Assert_f);
	}
}
