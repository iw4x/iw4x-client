#include "LanguageFonts.hpp"
#include "Localization.hpp"
#include "ZoneBuilder.hpp"

namespace Components
{
	Dvar::Var LanguageFonts::LocLanguageFonts;

	namespace
	{
		// Translation name -> prefix of its fonts
		const std::unordered_map<std::string, std::string> FONT_PREFIXES
		{
			{ "arabic", "ar" },
			{ "german", "de" },
			{ "italian", "it" },
			{ "spanish", "es" },
			{ "polish", "pl" },
			{ "russian", "ru" },
			{ "japanese", "ja" },
			{ "korean", "ko" },
		};

		constexpr const char* STOCK_FONTS[]
		{
			"smallFont", "normalFont", "boldFont", "bigFont", "extraBigFont", "objectiveFont",
			"hudBigFont", "hudSmallFont", "consoleFont", "smallDevFont", "bigDevFont",
		};

		// Tried in order for letters the current font does not have
		constexpr const char* BACKUP_FONTS[] { "fonts/fb_buttons", "fonts/fb_latin", "fonts/ar_normalFont", "fonts/fb_ja", "fonts/fb_ko" };

		Game::Font_s* FindLoadedFont(const char* name)
		{
			// Never creates a default asset, so missing zones just mean no font
			auto* entry = Game::DB_FindXAssetEntry(Game::ASSET_TYPE_FONT, name);
			return entry ? entry->asset.header.font : nullptr;
		}
	}

	Game::Glyph* LanguageFonts::FindGlyph(const Game::Font_s* font, const unsigned int letter)
	{
		// Like the game: 32-127 come first and are indexed directly, the rest is sorted
		constexpr auto directCount = 96;
		if (letter >= 32 && letter < 32 + directCount)
		{
			auto* glyph = letter - 32 < static_cast<unsigned int>(font->glyphCount) ? &font->glyphs[letter - 32] : nullptr;
			return glyph && glyph->letter == letter ? glyph : nullptr;
		}

		auto* begin = font->glyphs + std::min(font->glyphCount, directCount);
		auto* end = font->glyphs + font->glyphCount;
		auto* glyph = std::lower_bound(begin, end, letter, [](const Game::Glyph& g, const unsigned int l)
		{
			return g.letter < l;
		});

		return glyph != end && glyph->letter == letter ? glyph : nullptr;
	}

	Game::Glyph* LanguageFonts::FindBackupGlyph(const unsigned int letter, Game::Font_s** backupFont)
	{
		for (const auto* name : BACKUP_FONTS)
		{
			auto* font = FindLoadedFont(name);
			if (font == nullptr)
			{
				continue;
			}

			if (auto* glyph = FindGlyph(font, letter))
			{
				*backupFont = font;
				return glyph;
			}
		}

		return nullptr;
	}

	Game::XAssetHeader LanguageFonts::FindFont([[maybe_unused]] const Game::XAssetType type, const std::string& name)
	{
		Game::XAssetHeader header{ nullptr };

		if (!LocLanguageFonts.get<bool>())
		{
			return header;
		}

		const auto prefix = FONT_PREFIXES.find(Localization::GetTranslationName());
		if (prefix == FONT_PREFIXES.end())
		{
			return header;
		}

		// Menus are not consistent about casing, e.g. "fonts/bigfont"
		const auto lowerName = Utils::String::ToLower(name);
		for (const auto* stockName : STOCK_FONTS)
		{
			if (lowerName == Utils::String::ToLower(std::format("fonts/{}", stockName)))
			{
				// Only use the replacement when its zone is loaded
				if (auto* font = FindLoadedFont(std::format("fonts/{}_{}", prefix->second, stockName).data()))
				{
					header.font = font;
				}
				break;
			}
		}

		return header;
	}

	LanguageFonts::LanguageFonts()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		LocLanguageFonts = Dvar::Register<bool>("loc_languageFonts", true, Game::DVAR_ARCHIVE, "Use the fonts of the selected translation (loc_translation). Requires a restart.");

		AssetHandler::OnFind(Game::ASSET_TYPE_FONT, FindFont);
	}
}
