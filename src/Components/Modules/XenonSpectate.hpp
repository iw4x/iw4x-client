#pragma once

namespace Components
{
	class XenonSpectate : public Component
	{
	public:
		XenonSpectate();
		static bool HandleGamepadSpectatorInput(int localClientNum, int key, Game::GamePadButtonEvent buttonEvent, unsigned time);

	private:
#ifdef __XENON_UI_SPECTATOR
		// Replaces the PC spectator controls draw with the full console version
		static Utils::Hook s_drawSpectatorControlsHook;
		static void CG_DrawSpectatorControls_Hk(int localClientNum, const Game::rectDef_s* rect, Game::Font_s* font, float fontScale, float* color, int textStyle, int textAlignMode);
#endif

#ifdef __XENON_INPUT_SPECTATOR
		// Registers +specNext/-specNext, +specPrev/-specPrev, +toggleSpec/-toggleSpec
		// which the PC CL_InitInput stripped out (console-only)
		static Utils::Hook s_clInitInputHook;
		static void CL_InitInput_Hk();

		// Applies console-like third-person dvar selection in spectator mode
		static Utils::Hook s_updateThirdPersonHook;
		static void UpdateThirdPerson_Hk();
#endif
	};
}
