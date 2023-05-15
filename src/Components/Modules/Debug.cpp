#include <STDInclude.hpp>
#include "Debug.hpp"
#include "Events.hpp"
#include "TextRenderer.hpp"

#include "Game/Engine/ScopedCriticalSection.hpp"

namespace Components
{
	const Game::dvar_t* Debug::DebugOverlay;
	const Game::dvar_t* Debug::BugName;

	const Game::dvar_t* Debug::PlayerDebugHealth;

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

	const char Debug::StrButtons[] =
	{
		'\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x0E', '\x0F', '\x10',
		'\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\0'
	};

	const char Debug::StrTemplate[] = "%s: %s All those moments will be lost in time, like tears in rain.";

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

		auto* const font1 = Game::UI_GetFontHandle(scrPlace, 6, MY_SCALE_2);
		auto* const font2 = Game::UI_GetFontHandle(scrPlace, 6, MY_SCALE2);

		Game::UI_DrawText(scrPlace, "Client View of Flags", maxChars, font2, -60.0f, 0, 1, 1,
			MY_SCALE2, TextRenderer::WHITE_COLOR, 1);

		const auto pmf = BuildPMFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pmf.data(), maxChars, font1, 30.0f, MY_Y, 1, 1, MY_SCALE_2, TextRenderer::WHITE_COLOR, 3);

		const auto pof = BuildPOFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pof.data(), maxChars, font1, 350.0f, MY_Y, 1, 1, MY_SCALE_2, TextRenderer::WHITE_COLOR, 3);

		const auto plf = BuildPLFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, plf.data(), maxChars, font1, 350.0f, 250.0f, 1, 1, MY_SCALE_2, TextRenderer::WHITE_COLOR, 3);

		const auto pef = BuildPEFlagsString(&cgameGlob->predictedPlayerState);
		Game::UI_DrawText(scrPlace, pef.data(), maxChars, font1, 525.0f, MY_Y, 1, 1, MY_SCALE_2, TextRenderer::WHITE_COLOR, 3);
	}

	void Debug::CG_DrawDebugPlayerHealth(const int localClientNum)
	{
		float healtha;
		constexpr float color1[] = {0.0f, 0.0f, 0.0f, 1.0f};
		constexpr float color2[] = {0.0f, 1.0f, 0.0f, 1.0f};

		assert(PlayerDebugHealth->current.enabled);
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

	void Debug::CG_Debug_DrawFontTest(const int localClientNum)
	{
		char strFinal[0x200]{};

		auto* const scrPlace = Game::ScrPlace_GetActivePlacement(localClientNum);

		auto* const font1 = Game::UI_GetFontHandle(scrPlace, 1, 0.4f);
		auto* const font2 = Game::UI_GetFontHandle(scrPlace, 2, 0.4f);
		auto* const font3 = Game::UI_GetFontHandle(scrPlace, 3, 0.4f);
		auto* const font5 = Game::UI_GetFontHandle(scrPlace, 5, 0.4f);
		auto* const font6 = Game::UI_GetFontHandle(scrPlace, 6, 0.4f);

		sprintf_s(strFinal, StrTemplate, font1->fontName, StrButtons);
		Game::UI_FilterStringForButtonAnimation(strFinal, sizeof(strFinal));
		Game::UI_DrawText(scrPlace, strFinal, std::numeric_limits<int>::max(), font1, MY_X, 10.0f, 1, 1, 0.4f, TextRenderer::WHITE_COLOR, 3);

		sprintf_s(strFinal, StrTemplate, font2->fontName, StrButtons);
		Game::UI_FilterStringForButtonAnimation(strFinal, sizeof(strFinal));
		Game::UI_DrawText(scrPlace, strFinal, std::numeric_limits<int>::max(), font2, MY_X, 35.0f, 1, 1, 0.4f, TextRenderer::WHITE_COLOR, 3);

		sprintf_s(strFinal, StrTemplate, font3->fontName, StrButtons);
		Game::UI_FilterStringForButtonAnimation(strFinal, sizeof(strFinal));
		Game::UI_DrawText(scrPlace, strFinal, std::numeric_limits<int>::max(), font3, MY_X, 60.0f, 1, 1, 0.4f, TextRenderer::WHITE_COLOR, 3);

		sprintf_s(strFinal, StrTemplate, font5->fontName, StrButtons);
		Game::UI_FilterStringForButtonAnimation(strFinal, sizeof(strFinal));
		Game::UI_DrawText(scrPlace, strFinal, std::numeric_limits<int>::max(), font5, MY_X, 85.0f, 1, 1, 0.4f, TextRenderer::WHITE_COLOR, 3);

		sprintf_s(strFinal, StrTemplate, font6->fontName, StrButtons);
		Game::UI_FilterStringForButtonAnimation(strFinal, sizeof(strFinal));
		Game::UI_DrawText(scrPlace, strFinal, std::numeric_limits<int>::max(), font6, MY_X, 110.0f, 1, 1, 0.4f, TextRenderer::WHITE_COLOR, 3);
	}

	void Debug::CG_DrawDebugOverlays_Hk(const int localClientNum)
	{
		assert(DebugOverlay);
		if (!DebugOverlay)
		{
			return;
		}

		switch (DebugOverlay->current.integer)
		{
		case 2:
			CG_Debug_DrawPSFlags(localClientNum);
			break;
		case 5:
			CG_Debug_DrawFontTest(localClientNum);
			break;
		default:
			break;
		}

		if (PlayerDebugHealth->current.enabled)
		{
			CG_DrawDebugPlayerHealth(localClientNum);
		}
	}

	void Debug::Com_Assert_f()
	{
		assert(0 && "a");
	}

	void Debug::Com_Bug_f(const Command::Params* params)
	{
		char newFileName[MAX_PATH]{};
		char to_ospath[MAX_OSPATH]{};
		char from_ospath[MAX_OSPATH]{};
		const char* bug;

		if (!*Game::logfile)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "CopyFile failed: logfile wasn't opened\n");
		}

		if (params->size() == 2)
		{
			bug = params->get(1);
		}
		else
		{
			assert(BugName);
			bug = BugName->current.string;
		}

		sprintf_s(newFileName, "%s_%s.log", bug, Game::Live_GetLocalClientName(0));

		Game::Engine::ScopedCriticalSection _(Game::CRITSECT_CONSOLE, Game::Engine::SCOPED_CRITSECT_NORMAL);

		if (*Game::logfile)
		{
			Game::FS_FCloseFile(*Game::logfile);
			*Game::logfile = 0;
		}

		Game::FS_BuildOSPath(Game::Sys_DefaultInstallPath(), "", "logs/console_mp.log", from_ospath);
		Game::FS_BuildOSPath(Game::Sys_DefaultInstallPath(), "", newFileName, to_ospath);
		const auto result = CopyFileA(from_ospath, to_ospath, 0);
		Game::Com_OpenLogFile();

		if (!result)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "CopyFile failed({}) {} {}\n", GetLastError(), "console_mp.log", newFileName);
		}
	}

	void Debug::Com_BugNameInc_f()
	{
		char buf[512]{};

		if (std::strlen(BugName->current.string) < 4)
		{
			Game::Dvar_SetString(BugName, "bug0");
			return;
		}

		if (std::strncmp(BugName->current.string, "bug", 3) != 0)
		{
			Game::Dvar_SetString(BugName, "bug0");
			return;
		}

		const auto n = std::strtol(BugName->current.string + 3, nullptr, 10);
		sprintf_s(buf, "bug%d", n + 1);
		Game::Dvar_SetString(BugName, buf);
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

		DebugOverlay = Game::Dvar_RegisterEnum("debugOverlay", debugOverlayNames_0, 0, Game::DVAR_NONE, "Toggles the display of various debug info.");
		BugName = Game::Dvar_RegisterString("bug_name", "bug0", Game::DVAR_NONE, "Name appended to the copied console log");
	}

	const Game::dvar_t* Debug::Dvar_Register_PlayerDebugHealth(const char* name, bool value, [[maybe_unused]] std::uint16_t flags, const char* description)
	{
		PlayerDebugHealth = Game::Dvar_RegisterBool(name, value, Game::DVAR_NONE, description);
		return PlayerDebugHealth;
	}

	Debug::Debug()
	{
		Events::OnDvarInit(CL_InitDebugDvars);

		// Hook end of CG_DrawDebugOverlays (This is to ensure some checks are done before our hook is executed).
		Utils::Hook(0x49CB0A, CG_DrawDebugOverlays_Hk, HOOK_JUMP).install()->quick();

		Utils::Hook::Set<void(*)()>(0x60BCEA, Com_Assert_f);

		Utils::Hook(0x4487F7, Dvar_Register_PlayerDebugHealth, HOOK_CALL).install()->quick();

#ifdef _DEBUG
		Command::Add("bug", Com_Bug_f);
		Command::Add("bug_name_inc", Com_BugNameInc_f);
#endif
	}
}
