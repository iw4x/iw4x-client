#pragma once

namespace Components
{
	// Right-aligns menu text that is in a right-to-left language, e.g. an Arabic translation
	class RtlMenus : public Component
	{
	public:
		RtlMenus();

	private:
		// Horizontal part of itemDef_s::textAlignMode
		static constexpr int ITEM_ALIGN_X_MASK = 3;
		static constexpr int ITEM_ALIGN_LEFT = 0;
		static constexpr int ITEM_ALIGN_RIGHT = 2;

		static Dvar::Var LocRightAlignMenus;
		static std::unordered_set<const Game::itemDef_s*> FlippedItems;

		static bool IsRtlText(const char* text);
		static bool DrawsText(const Game::itemDef_s* item);
		static const Game::itemDef_s* FindRowNeighbour(const Game::itemDef_s* item, const Game::menuDef_t* menu);
		static const char* GetSkipReason(const Game::itemDef_s* item, const Game::menuDef_t* menu, const Game::itemDef_s** neighbour);
		static void AlignItem(Game::itemDef_s* item, const Game::menuDef_t* menu);
		static void AlignMenu(const Game::menuDef_t* menu);
		static void AlignOpenMenus(const Game::UiContext* context);
		static void AlignAllMenus(const Game::UiContext* context);

		static std::string DescribeItem(const Game::itemDef_s* item, int index);
		static void DebugMenus();
	};
}
