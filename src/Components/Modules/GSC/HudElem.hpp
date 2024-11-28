#pragma once

namespace Components::GSC
{
	class HudElem : public Component
	{

	public:
		HudElem();
		
	private:
		// 0x417730
		static const std::unordered_map<std::string, int> HEFontStyles;

		static Game::game_hudelem_s* HECmd_GetHudElem(Game::scr_entref_t entref);
		static void DrawSingleHudElem2d(int localClientNum, Game::hudelem_s * elem);
		static void DrawWorldSpaceTextHudElem(int localClientIndex, Game::hudelem_s * elem);
		static void GetHudElemInfo(int clientIndex, Game::hudelem_s * elem, Game::cg_hudelem_t * cg_hud_elem, uint8_t * elementText);
		static void HudElemColorToVec4(IN const Game::hudelem_color_t & inHudColor, OUT Game::vec4_t & outFloatColor);
		static void AddDrawSurfForHudElem3DText(int localClientIndex, Game::hudelem_s * elem);
		static void CG_AddDrawSurfsFor3dHudElems_Stub();

		static void AddScriptMethods();
		static bool Is3D(const Game::hudelem_s * hudElem);

		static void HECmd_SetTextExtended(Game::scr_entref_t entref);
	};
}
