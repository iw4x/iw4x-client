#include "XenonScoreboard.hpp"

namespace Components
{
	Game::Material* XenonScoreboard::s_whiteMaterial = nullptr;
	std::vector<int> XenonScoreboard::s_visibleClients{};
	int XenonScoreboard::s_selectedClientNum = -1;
	float XenonScoreboard::s_lastDrawY = -1.0f;

	bool XenonScoreboard::IsScoreboardVisible(const int localClientNum)
	{
		if (localClientNum < 0 || localClientNum >= Game::MAX_LOCAL_CLIENTS)
		{
			return false;
		}

		if (Game::UI_GetActiveMenu(localClientNum) == Game::UIMENU_SCOREBOARD)
		{
			return true;
		}

		// In-game scoreboard is usually shown while the "togglescores" bind is held down.
		const auto& keyState = Game::playerKeys[localClientNum];
		for (int key = 0; key < Game::K_LAST_KEY; ++key)
		{
			if (!keyState.keys[key].down)
			{
				continue;
			}

			const auto* binding = keyState.keys[key].binding;
			if (binding && std::strcmp(binding, "togglescores") == 0)
			{
				return true;
			}
		}

		return false;
	}


	bool XenonScoreboard::HandleGamepadScoreboardInput(const int localClientNum, const int key, const Game::GamePadButtonEvent buttonEvent)
	{
#ifndef __XENON_SCOREBOARD
		(void)localClientNum;
		(void)key;
		(void)buttonEvent;
		return false;
#else
		if (!IsScoreboardVisible(localClientNum))
		{
			s_visibleClients.clear();
			s_selectedClientNum = -1;
			s_lastDrawY = -1.0f;
			return false;
		}

		if (buttonEvent != Game::GPAD_BUTTON_PRESSED && buttonEvent != Game::GPAD_BUTTON_UPDATE)
		{
			return false;
		}

		switch (key)
		{
		case Game::K_DPAD_UP:
			if (!s_visibleClients.empty())
			{
				auto selectedIndex = 0;
				auto foundSelected = false;
				for (auto i = 0u; i < s_visibleClients.size(); ++i)
				{
					if (s_visibleClients[i] == s_selectedClientNum)
					{
						selectedIndex = static_cast<int>(i);
						foundSelected = true;
						break;
					}
				}

				if (!foundSelected)
				{
					s_selectedClientNum = s_visibleClients.front();
					return true;
				}

				if (selectedIndex > 0)
				{
					s_selectedClientNum = s_visibleClients[selectedIndex - 1];
				}
				else
				{
					Game::CG_ScrollScoreboardUp(Game::cgArray);
				}
			}
			else
			{
				Game::CG_ScrollScoreboardUp(Game::cgArray);
			}
			return true;

		case Game::K_DPAD_DOWN:
			if (!s_visibleClients.empty())
			{
				auto selectedIndex = 0;
				auto foundSelected = false;
				for (auto i = 0u; i < s_visibleClients.size(); ++i)
				{
					if (s_visibleClients[i] == s_selectedClientNum)
					{
						selectedIndex = static_cast<int>(i);
						foundSelected = true;
						break;
					}
				}

				if (!foundSelected)
				{
					s_selectedClientNum = s_visibleClients.front();
					return true;
				}

				if (selectedIndex + 1 < static_cast<int>(s_visibleClients.size()))
				{
					s_selectedClientNum = s_visibleClients[selectedIndex + 1];
				}
				else
				{
					Game::CG_ScrollScoreboardDown(Game::cgArray);
				}
			}
			else
			{
				Game::CG_ScrollScoreboardDown(Game::cgArray);
			}
			return true;

		default:
			(void)localClientNum;
			return false;
		}
#endif
	}

#ifdef __XENON_SCOREBOARD
	/* Temporarily disabled while being reworked.
	void XenonScoreboard::DrawControlsText(const int localClientNum, const float x, const float y, const float halfWidth)
	{
		const auto* cxt = Game::ScrPlace_GetActivePlacement(localClientNum);
		if (!cxt)
		{
			return;
		}

		const auto controlsY = y - 8.0f;

		float controlsScale = 0.30000001f;
		if (const auto* fontScaleDvar = Game::Dvar_FindVar("cg_scoreboardHeaderFontScale"))
		{
			controlsScale = fontScaleDvar->current.value;
		}

		int fontEnum = 3;
		if (const auto* fontDvar = Game::Dvar_FindVar("cg_scoreboardFont"))
		{
			fontEnum = fontDvar->current.integer;
		}

		auto* controlsFont = Game::UI_GetFontHandle(const_cast<Game::ScreenPlacement*>(cxt), fontEnum, controlsScale);
		if (!controlsFont)
		{
			return;
		}

		constexpr float controlsColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		const auto* gamercardText = Game::UI_SafeTranslateString("XBOXLIVE_GAMERCARD");
		const auto* muteToggleText = Game::UI_SafeTranslateString("PLATFORM_PLAYER_TOGGLE_MUTE_BTN");

		const auto gamercardWidth = static_cast<float>(Game::UI_TextWidth(gamercardText, std::numeric_limits<int>::max(), controlsFont, controlsScale));
		const auto muteToggleWidth = static_cast<float>(Game::UI_TextWidth(muteToggleText, std::numeric_limits<int>::max(), controlsFont, controlsScale));

		const auto leftXAdj = ((halfWidth - gamercardWidth) * 0.5f > 0.0f) ? (halfWidth - gamercardWidth) * 0.5f : 0.0f;
		const auto rightXAdj = ((halfWidth - muteToggleWidth) * 0.5f > 0.0f) ? (halfWidth - muteToggleWidth) * 0.5f : 0.0f;

		Game::UI_DrawText(cxt, gamercardText, std::numeric_limits<int>::max(), controlsFont, x + leftXAdj, controlsY, 0, 0, controlsScale, controlsColor, 3);
		Game::UI_DrawText(cxt, muteToggleText, std::numeric_limits<int>::max(), controlsFont, x + halfWidth + rightXAdj, controlsY, 0, 0, controlsScale, controlsColor, 3);
	}
	*/

	void XenonScoreboard::DrawOutline(Game::ScreenPlacement* scrPlace, const float x, const float y, const float w, const float h,
		const float borderSize, const float* color, Game::Material* material)
	{
		Game::UI_DrawHandlePic(scrPlace, x, y, w, borderSize, 1, 0, color, material);
		Game::UI_DrawHandlePic(scrPlace, x, y + h - borderSize, w, borderSize, 1, 0, color, material);
		Game::UI_DrawHandlePic(scrPlace, x, y, borderSize, h, 1, 0, color, material);
		Game::UI_DrawHandlePic(scrPlace, x + w - borderSize, y, borderSize, h, 1, 0, color, material);
	}

	float __cdecl XenonScoreboard::DrawClientScoreHook(const int localClientNum, const int colorPtr, const float y,
		const int scorePtr, const float listWidth)
	{
		const auto nextY = Utils::Hook::Call<float(__cdecl)(int, int, float, int, float)>(0x590DE0)(
			localClientNum, colorPtr, y, scorePtr, listWidth);

		if (!scorePtr)
		{
			return nextY;
		}

		const auto currentClientNum = *reinterpret_cast<const int*>(scorePtr);

		// Rows are drawn top-to-bottom; a lower Y than the previous row means a new draw pass.
		if (s_lastDrawY < 0.0f || y + 0.5f < s_lastDrawY)
		{
			/*s_visibleClients.clear();
			const auto scoreboardHeight = (*Game::cg_scoreboardHeight)->current.value;
			const auto scoreboardTop = std::max((480.0f - scoreboardHeight) * 0.5f, 0.0f);
			const auto controlsAnchorY = scoreboardTop + scoreboardHeight;
			const auto controlsX = Game::UI_GetScoreboardLeft(reinterpret_cast<void*>(localClientNum)) - 24.0f;
			*/
			// DrawControlsText(localClientNum, controlsX, controlsAnchorY, listWidth * 0.5f);
		}
		s_lastDrawY = y;

		if (s_visibleClients.empty() || s_visibleClients.back() != currentClientNum)
		{
			s_visibleClients.push_back(currentClientNum);
		}

		if (s_selectedClientNum < 0)
		{
			s_selectedClientNum = currentClientNum;
		}

		if (currentClientNum != s_selectedClientNum)
		{
			return nextY;
		}

		if (!s_whiteMaterial)
		{
			s_whiteMaterial = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "white").material;
		}

		const auto* scrPlace = Game::ScrPlace_GetActivePlacement(localClientNum);
		const auto* scoreboardWidthDvar = *Game::cg_scoreboardWidth;
		if (!scrPlace || !s_whiteMaterial || !scoreboardWidthDvar)
		{
			return nextY;
		}

		const auto x = Game::UI_GetScoreboardLeft(reinterpret_cast<void*>(localClientNum)) + 5.0f;
		const auto w = listWidth + 8.0f;

		float itemHeight = 18.0f;
		if (const auto* itemHeightDvar = Game::Dvar_FindVar("cg_scoreboardItemHeight"))
		{
			itemHeight = static_cast<float>(itemHeightDvar->current.integer);
		}

		const auto* color = reinterpret_cast<const float*>(colorPtr);
		const float borderColor[4] = {1.0f, 1.0f, 1.0f, color ? color[3] : 1.0f};

		DrawOutline(const_cast<Game::ScreenPlacement*>(scrPlace), x, y, w, itemHeight, 2.0f, borderColor, s_whiteMaterial);
		return nextY;
	}
#endif

	XenonScoreboard::XenonScoreboard()
	{
#ifdef __XENON_SCOREBOARD
		Utils::Hook(0x59161D, DrawClientScoreHook, HOOK_CALL).install()->quick();
#endif
	}
}
