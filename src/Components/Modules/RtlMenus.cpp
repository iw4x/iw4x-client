#include "RtlMenus.hpp"
#include "Localization.hpp"
#include "ZoneBuilder.hpp"

namespace Components
{
	Dvar::Var RtlMenus::LocRightAlignMenus;
	std::unordered_set<const Game::itemDef_s*> RtlMenus::FlippedItems;

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

	const Game::itemDef_s* RtlMenus::FindRowNeighbour(const Game::itemDef_s* item, const Game::menuDef_t* menu)
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
				return other;
			}
		}

		return nullptr;
	}

	const char* RtlMenus::GetSkipReason(const Game::itemDef_s* item, const Game::menuDef_t* menu, const Game::itemDef_s** neighbour)
	{
		*neighbour = nullptr;

		// Other item types draw a value or control next to the text and rely on its position
		if (item->type != Game::ITEM_TYPE_TEXT && item->type != Game::ITEM_TYPE_BUTTON)
		{
			return "not a text or button item";
		}

		// Right-aligned items are skipped, which also keeps this from flipping an item twice
		if ((item->textAlignMode & ITEM_ALIGN_X_MASK) != ITEM_ALIGN_LEFT)
		{
			return "not left-aligned";
		}

		if (item->window.rect.w < 1.0f)
		{
			return "no width to align to";
		}

		// Text from an expression is only known while drawing, assume it is translated like the rest
		if (item->textExp ? !Localization::IsRtlTranslation() : !IsRtlText(item->text))
		{
			return "text is not right-to-left";
		}

		// Text would run into columns that share the row, e.g. labels next to their values
		if ((*neighbour = FindRowNeighbour(item, menu)))
		{
			return "shares its row with";
		}

		return nullptr;
	}

	void RtlMenus::AlignItem(Game::itemDef_s* item, const Game::menuDef_t* menu)
	{
		const Game::itemDef_s* neighbour;
		if (GetSkipReason(item, menu, &neighbour))
		{
			return;
		}

		// Alignment is relative to the item rect, so mirroring the offset keeps the same margin on the right
		item->textAlignMode = (item->textAlignMode & ~ITEM_ALIGN_X_MASK) | ITEM_ALIGN_RIGHT;
		item->textalignx = -item->textalignx;

		// Make the game recalculate the cached text position
		item->textRect[0].w = 0.0f;

		FlippedItems.insert(item);
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

	std::string RtlMenus::DescribeItem(const Game::itemDef_s* item, const int index)
	{
		std::string text;
		if (item->textExp)
		{
			text = "<expression>";
		}
		else if (item->text)
		{
			text = *item->text == '@' ? std::format("{} = {}", item->text, Localization::Get(item->text + 1)) : item->text;
		}

		const auto& rect = item->window.rect;
		return std::format("[{}] {} type {} rect ({}, {}, {}, {}) align {}/{} textalign {} \"{}\"", index,
			item->window.name ? item->window.name : "<unnamed>", item->type, rect.x, rect.y, rect.w, rect.h,
			static_cast<int>(rect.horzAlign), static_cast<int>(rect.vertAlign), item->textAlignMode, text);
	}

	void RtlMenus::DebugMenus()
	{
		std::string report;

		for (const auto* context : { Game::uiContext, Game::cgDC })
		{
			const auto openMenuCount = std::min(context->openMenuCount, static_cast<int>(std::size(context->menuStack)));
			for (auto m = 0; m < openMenuCount; ++m)
			{
				const auto* menu = context->menuStack[m];
				if (menu == nullptr || menu->items == nullptr)
				{
					continue;
				}

				report += std::format("menu {}\n", menu->window.name ? menu->window.name : "<unnamed>");

				for (auto i = 0; i < menu->itemCount; ++i)
				{
					const auto* item = menu->items[i];
					if (item == nullptr || !DrawsText(item))
					{
						continue;
					}

					const Game::itemDef_s* neighbour;
					const auto* reason = GetSkipReason(item, menu, &neighbour);

					std::string result;
					if (FlippedItems.contains(item))
					{
						result = "right-aligned for right-to-left";
					}
					else if (reason == nullptr)
					{
						result = "will be right-aligned";
					}
					else
					{
						result = std::format("kept: {}", reason);
						if (neighbour)
						{
							const auto index = std::find(menu->items, menu->items + menu->itemCount, neighbour) - menu->items;
							result += " " + DescribeItem(neighbour, static_cast<int>(index));
						}
					}

					report += std::format("  {}\n    -> {}\n", DescribeItem(item, i), result);
				}
			}
		}

		Utils::IO::CreateDir("userraw");
		Utils::IO::WriteFile("userraw/rtl_menus.txt", report);
		Logger::Print("{}Wrote userraw/rtl_menus.txt\n", report);
	}

	RtlMenus::RtlMenus()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		LocRightAlignMenus = Dvar::Register<bool>("loc_rightAlignMenus", true, Game::DVAR_ARCHIVE, "Right-align menu text that is in a right-to-left language");

		// Lists the text items of the open menus and whether they are right-aligned, and why not
		Command::Add("loc_debugMenus", DebugMenus);

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
