#include "XenonSpectate.hpp"
#include "Gamepad.hpp"

namespace Components
{
#ifdef __XENON_UI_SPECTATOR
	Utils::Hook XenonSpectate::s_drawSpectatorControlsHook;

	namespace
	{
		const char* Spectate_GetPrevCommand(int localClientNum)
		{
			UNREFERENCED_PARAMETER(localClientNum);
			return "+specPrev";
		}

		const char* Spectate_GetToggleCommand(int localClientNum)
		{
			UNREFERENCED_PARAMETER(localClientNum);
			return "+toggleSpec";
		}

		bool Spectate_GetKeyString(int localClientNum, const char* command, char* out, size_t outSize)
		{
			if (!command || !*command || !out || !outSize) return false;
			const auto& ks = Game::playerKeys[localClientNum];
			const auto* keyNameMap = Gamepad::GetLocalizedKeyNameMap();
			for (int k = 0; k < Game::K_LAST_KEY; ++k)
			{
				const auto* binding = ks.keys[k].binding;
				if (!binding || _stricmp(binding, command) != 0) continue;
				for (int i = 0; keyNameMap[i].name; ++i)
				{
					if (keyNameMap[i].keynum == k)
					{
						Game::I_strncpyz(out, keyNameMap[i].name, static_cast<int>(outSize));
						return true;
					}
				}
				break;
			}
			return false;
		}
	}

	void XenonSpectate::CG_DrawSpectatorControls_Hk(int localClientNum, const Game::rectDef_s* rect, Game::Font_s* /*font*/, float fontScale, float* color, int textStyle, int textAlignMode)
	{
		if (localClientNum < 0 || localClientNum >= Game::MAX_LOCAL_CLIENTS || !rect) return;

		const auto* descDvar = Game::Dvar_FindVar("cg_descriptiveText");
		if (!descDvar || !descDvar->current.enabled) return;
		if (Game::Key_IsCatcherActive(localClientNum, 16)) return;

		auto* cg = Game::CL_GetLocalClientGlobals(localClientNum);
		if (!cg) return;

		const int flags = cg->predictedPlayerState.otherFlags;
		if ((flags & 0x6000) == 0) return;

		auto* placement = Game::ScrPlace_GetActivePlacement(localClientNum);
		if (!placement) return;

		auto* drawFont = Game::UI_GetFontHandle(const_cast<Game::ScreenPlacement*>(placement), 0, fontScale);
		if (!drawFont) return;

		const char* prevCmd   = Spectate_GetPrevCommand(localClientNum);
		const char* toggleCmd = Spectate_GetToggleCommand(localClientNum);

		constexpr int MAX_LINES = 8;
		const char* cmds[MAX_LINES]{};
		const char* refs[MAX_LINES]{};
		int n = 0;

		cmds[n] = "togglemenu";
		refs[n++] = "PLATFORM_MP_OPEN_MENU";

		if ((flags & 0x800) != 0)
		{
			if ((flags & 0x2000) != 0)
			{
				cmds[n] = "+specNext";
				refs[n++] = "PLATFORM_FOLLOWNEXTPLAYER";
				cmds[n] = prevCmd;
				refs[n++] = "PLATFORM_FOLLOWPREVPLAYER";
			}

			if ((flags & 0x4000) != 0)
			{
				cmds[n] = toggleCmd;
				refs[n++] = cg->renderingThirdPerson ? "PLATFORM_FIRSTPERSON" : "PLATFORM_FOLLOWSTOP";
			}
			else
			{
				cmds[n] = toggleCmd;
				refs[n++] = cg->renderingThirdPerson ? "PLATFORM_FIRSTPERSON" : "PLATFORM_THIRDPERSON";
			}
		}
		else
		{
			cmds[n] = "+specNext";
			refs[n++] = "PLATFORM_FOLLOWSTART";
		}

		cmds[n] = "togglescores";
		refs[n++] = "PLATFORM_SCOREBOARD";

		const float rowHeight = rect->h + static_cast<float>(Game::UI_TextHeight(drawFont, fontScale));
		static const float white[] = {1.f, 1.f, 1.f, 1.f};
		const float* drawColor = color ? color : white;

		char keyBuf[256]{};
		char lineBuf[256]{};
		float currentY = rect->y;

		for (int i = 0; i < n; ++i)
		{
			if (!Spectate_GetKeyString(localClientNum, cmds[i], keyBuf, sizeof(keyBuf)))
				Game::I_strncpyz(keyBuf, Game::UI_SafeTranslateString("KEY_UNBOUND"), sizeof(keyBuf));

			Game::ConversionArguments args{};
			args.args[args.argCount++] = keyBuf;
			Game::UI_ReplaceConversions(Game::UI_SafeTranslateString(refs[i]), &args, lineBuf, sizeof(lineBuf));
			Game::UI_FilterStringForButtonAnimation(lineBuf, static_cast<unsigned int>(sizeof(lineBuf)));

			int textRect[5]{};
			Game::UI_DrawWrappedText(
				placement,
				lineBuf,
				rect,
				drawFont,
				rect->x,
				currentY,
				fontScale,
				drawColor,
				textStyle,
				textAlignMode,
				textRect,
				0
			);
			currentY += rowHeight;
		}
	}
#endif

#ifdef __XENON_INPUT_SPECTATOR
	Utils::Hook XenonSpectate::s_clInitInputHook;
	Utils::Hook XenonSpectate::s_updateThirdPersonHook;
	Game::dvar_t* cg_thirdPersonSpectator = nullptr;
	int* const inKillCam = reinterpret_cast<int*>(0x864D90);

	namespace
	{
		Game::kbutton_t* const in_attack_kb = reinterpret_cast<Game::kbutton_t*>(0xA1AAC0);

		void IN_SpecNext_Down()  { Game::IN_KeyDown(in_attack_kb); }
		void IN_SpecNext_Up()    { Game::IN_KeyUp(in_attack_kb); }
		void IN_SpecPrev_Down()  { Game::Cbuf_AddText(0, "+speed_throw\n"); }
		void IN_SpecPrev_Up()    { Game::Cbuf_AddText(0, "-speed_throw\n"); }

		void IN_ToggleSpec_Down()
		{
			auto* dvar = cg_thirdPersonSpectator;
			if (dvar)
				Game::Dvar_SetBool(dvar, !dvar->current.enabled);
		}
		void IN_ToggleSpec_Up() {}

		Game::cmd_function_s in_specNext_down_VAR{};
		Game::cmd_function_s in_specNext_up_VAR{};
		Game::cmd_function_s in_specPrev_down_VAR{};
		Game::cmd_function_s in_specPrev_up_VAR{};
		Game::cmd_function_s in_toggleSpec_down_VAR{};
		Game::cmd_function_s in_toggleSpec_up_VAR{};
	}

	void XenonSpectate::CL_InitInput_Hk()
	{
		s_clInitInputHook.uninstall();
		Utils::Hook::Call<void()>(0x45E380)();
		s_clInitInputHook.install();

		Game::Cmd_AddCommand("+specNext",   IN_SpecNext_Down,   &in_specNext_down_VAR,   1);
		Game::Cmd_AddCommand("-specNext",   IN_SpecNext_Up,     &in_specNext_up_VAR,     1);
		Game::Cmd_AddCommand("+specPrev",   IN_SpecPrev_Down,   &in_specPrev_down_VAR,   1);
		Game::Cmd_AddCommand("-specPrev",   IN_SpecPrev_Up,     &in_specPrev_up_VAR,     1);
		Game::Cmd_AddCommand("+toggleSpec", IN_ToggleSpec_Down, &in_toggleSpec_down_VAR, 1);
		Game::Cmd_AddCommand("-toggleSpec", IN_ToggleSpec_Up,   &in_toggleSpec_up_VAR,   1);
	}

	void XenonSpectate::UpdateThirdPerson_Hk()
	{
		s_updateThirdPersonHook.uninstall();
		Utils::Hook::Call<void()>(0x599520)();
		s_updateThirdPersonHook.install();

		auto* cg = Game::CL_GetLocalClientGlobals(0);
		if (!cg || !cg->nextSnap || !cg_thirdPersonSpectator)
		{
			return;
		}

		const bool isSpectator = (cg->nextSnap->ps.otherFlags & 0x800) != 0;
		const bool isInKillCam = inKillCam && (*inKillCam != 0);

		if (!isSpectator || isInKillCam)
		{
			return;
		}

		cg->renderingThirdPerson = cg_thirdPersonSpectator->current.enabled ? 1 : 0;
	}

	bool XenonSpectate::HandleGamepadSpectatorInput(int localClientNum, int key, Game::GamePadButtonEvent buttonEvent, unsigned time)
	{
		if (localClientNum < 0 || localClientNum >= Game::MAX_LOCAL_CLIENTS)
		{
			return false;
		}

		// Let menu/UI handling keep full control while a UI catcher is active.
		if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_UI))
		{
			return false;
		}

		auto* cg = Game::CL_GetLocalClientGlobals(localClientNum);
		if (!cg)
		{
			return false;
		}

		// We only remap controls while actively spectating.
		if ((cg->predictedPlayerState.otherFlags & 0x6000) == 0)
		{
			return false;
		}

		char cmd[96]{};
		switch (key)
		{
		case Game::K_BUTTON_B:
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED)
			{
				sprintf_s(cmd, "+specNext %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			else if (buttonEvent == Game::GPAD_BUTTON_RELEASED)
			{
				sprintf_s(cmd, "-specNext %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			break;

		case Game::K_BUTTON_A:
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED)
			{
				sprintf_s(cmd, "+speed_throw %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			else if (buttonEvent == Game::GPAD_BUTTON_RELEASED)
			{
				sprintf_s(cmd, "-speed_throw %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			break;

		case Game::K_BUTTON_Y:
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED)
			{
				sprintf_s(cmd, "+toggleSpec %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			else if (buttonEvent == Game::GPAD_BUTTON_RELEASED)
			{
				sprintf_s(cmd, "-toggleSpec %i %u\n", key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}
			break;

		case Game::K_BUTTON_LTRIG:
		case Game::K_BUTTON_RTRIG:
			break;

		case Game::K_BUTTON_BACK:
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED)
				Game::Cbuf_AddText(localClientNum, "togglescores\n");
			break;

		case Game::K_BUTTON_START:
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED)
				Game::Cbuf_AddText(localClientNum, "togglemenu\n");
			break;

		default:
			return false;
		}

		return true;
	}
#else
	bool XenonSpectate::HandleGamepadSpectatorInput(int /*localClientNum*/, int /*key*/, Game::GamePadButtonEvent /*buttonEvent*/, unsigned /*time*/)
	{
		return false;
	}
#endif

	XenonSpectate::XenonSpectate()
	{
#ifdef __XENON_UI_SPECTATOR
		s_drawSpectatorControlsHook.initialize(0x4790A0, CG_DrawSpectatorControls_Hk, HOOK_JUMP)->install();
#endif

#ifdef __XENON_INPUT_SPECTATOR
		s_clInitInputHook.initialize(0x45E380, CL_InitInput_Hk, HOOK_JUMP)->install();

		cg_thirdPersonSpectator = Game::Dvar_RegisterBool(
			"cg_thirdPersonSpectator",
			true,
			Game::DVAR_CHEAT,
			"Use third person view in spectator mode"
		);
		s_updateThirdPersonHook.initialize(0x599520, UpdateThirdPerson_Hk, HOOK_JUMP)->install();
#endif
	}
}
