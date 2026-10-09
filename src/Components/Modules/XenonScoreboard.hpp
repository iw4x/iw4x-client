#pragma once

#include "../Loader.hpp"

namespace Components
{
	class XenonScoreboard : public Component
	{
	public:
		XenonScoreboard();
		static bool HandleGamepadScoreboardInput(int localClientNum, int key, Game::GamePadButtonEvent buttonEvent);
#ifdef __XENON_SCOREBOARD
		// Temporarily disabled while being reworked.
		// static void DrawControlsText(int localClientNum, float x, float y, float halfWidth);
#endif

	private:
		static bool IsScoreboardVisible(int localClientNum);

#ifdef __XENON_SCOREBOARD
		static Game::Material* s_whiteMaterial;
		static std::vector<int> s_visibleClients;
		static int s_selectedClientNum;
		static float s_lastDrawY;
		static void DrawOutline(Game::ScreenPlacement* scrPlace, float x, float y, float w, float h, float borderSize, const float* color, Game::Material* material);
		static float __cdecl DrawClientScoreHook(int localClientNum, int colorPtr, float y, int scorePtr, float listWidth);
#endif
	};
}
