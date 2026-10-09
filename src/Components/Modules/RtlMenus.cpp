#include "RtlMenus.hpp"
#include "Localization.hpp"
#include "ZoneBuilder.hpp"

namespace Components
{
	Dvar::Var RtlMenus::LocRightAlignMenus;

	bool RtlMenus::IsRtlText(const char* text)
	{
		if (text == nullptr || *text == '\0')
		{
			return false;
		}

		// Localized references look like "@MENU_OPTIONS"
		if (*text == '@')
		{
			text = Localization::Get(text + 1);
		}

		return Utils::Arabic::ContainsRtl(text);
	}

	bool RtlMenus::DrawsText(const Game::itemDef_s* item)
	{
		if (item->type != Game::ITEM_TYPE_TEXT && item->type != Game::ITEM_TYPE_BUTTON)
		{
			// Value items like sliders and option lists draw their value as text
			return item->type != Game::ITEM_TYPE_MODEL && item->type != Game::ITEM_TYPE_MENUMODEL;
		}

		return (item->text && *item->text) || item->textExp || item->window.ownerDraw;
	}

	bool RtlMenus::SharesRow(const Game::itemDef_s* item, const Game::menuDef_t* menu)
	{
		const auto& rect = item->window.rect;

		for (auto i = 0; i < menu->itemCount; ++i)
		{
			const auto* other = menu->items[i];
			if (other == nullptr || other == item || !DrawsText(other))
			{
				continue;
			}

			const auto& otherRect = other->window.rect;
			if (otherRect.horzAlign != rect.horzAlign || otherRect.vertAlign != rect.vertAlign)
			{
				continue;
			}

			const auto sameRow = otherRect.y < rect.y + rect.h && otherRect.y + std::max(otherRect.h, 1.0f) > rect.y;
			const auto insideItem = otherRect.x < rect.x + rect.w && otherRect.x + std::max(otherRect.w, 1.0f) > rect.x;
			if (sameRow && insideItem)
			{
				return true;
			}
		}

		return false;
	}

	void RtlMenus::AlignItem(Game::itemDef_s* item, const Game::menuDef_t* menu)
	{
		// Other item types draw a value or control next to the text and rely on its position
		if (item->type != Game::ITEM_TYPE_TEXT && item->type != Game::ITEM_TYPE_BUTTON)
		{
			return;
		}

		// Right-aligned items are skipped, which also keeps this from flipping an item twice.
		// Without a width there is no right edge to align to.
		if ((item->textAlignMode & ITEM_ALIGN_X_MASK) != ITEM_ALIGN_LEFT || item->window.rect.w < 1.0f)
		{
			return;
		}

		// Text from an expression is only known while drawing, assume it is translated like the rest
		const auto isRtl = item->textExp ? Localization::IsRtlTranslation() : IsRtlText(item->text);

		// Text would run into columns that share the row, e.g. labels next to their values
		if (!isRtl || SharesRow(item, menu))
		{
			return;
		}

		// Alignment is relative to the item rect, so mirroring the offset keeps the same margin on the right
		item->textAlignMode = (item->textAlignMode & ~ITEM_ALIGN_X_MASK) | ITEM_ALIGN_RIGHT;
		item->textalignx = -item->textalignx;

		// Make the game recalculate the cached text position
		item->textRect[0].w = 0.0f;
	}

	void RtlMenus::AlignMenu(const Game::menuDef_t* menu)
	{
		if (menu == nullptr || menu->items == nullptr)
		{
			return;
		}

		for (auto i = 0; i < menu->itemCount; ++i)
		{
			if (menu->items[i])
			{
				AlignItem(menu->items[i], menu);
			}
		}
	}

	void RtlMenus::AlignOpenMenus(const Game::UiContext* context)
	{
		const auto openMenuCount = std::min(context->openMenuCount, static_cast<int>(std::size(context->menuStack)));
		for (auto i = 0; i < openMenuCount; ++i)
		{
			AlignMenu(context->menuStack[i]);
		}
	}

	void RtlMenus::AlignAllMenus(const Game::UiContext* context)
	{
		const auto menuCount = std::min(context->menuCount, static_cast<int>(std::size(context->Menus)));
		for (auto i = 0; i < menuCount; ++i)
		{
			AlignMenu(context->Menus[i]);
		}
	}

	RtlMenus::RtlMenus()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		LocRightAlignMenus = Dvar::Register<bool>("loc_rightAlignMenus", true, Game::DVAR_ARCHIVE, "Right-align menu text that is in a right-to-left language");

		// Menus can be opened or reloaded at any time, so check the open ones every frame
		Scheduler::Loop([]
		{
			if (!LocRightAlignMenus.get<bool>())
			{
				return;
			}

			AlignOpenMenus(Game::uiContext);
			AlignOpenMenus(Game::cgDC);
		}, Scheduler::Pipeline::MAIN);

		// Some menus are drawn without being opened, like the loading screen
		Scheduler::Loop([]
		{
			if (!LocRightAlignMenus.get<bool>())
			{
				return;
			}

			AlignAllMenus(Game::uiContext);
			AlignAllMenus(Game::cgDC);
		}, Scheduler::Pipeline::MAIN, 1s);
	}
}
