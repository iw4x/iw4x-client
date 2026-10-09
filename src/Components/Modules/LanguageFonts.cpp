#include "LanguageFonts.hpp"
#include "Localization.hpp"
#include "ZoneBuilder.hpp"
#include "FastFiles.hpp"

namespace Components
{
	Dvar::Var LanguageFonts::LocLanguageFonts;

	namespace
	{
		// Translation name -> prefix of its fonts
		const std::unordered_map<std::string, std::string> FONT_PREFIXES
		{
			// English (no translation) uses the Xbox console fonts as well
			{ "", "en" },
			{ "english", "en" },
			// Arabic text uses the Xbox fonts too, its letters come from the Arabic fonts (FindBackupGlyph)
			{ "arabic", "en" },
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
		// Arabic letters come from the Arabic font made for the same stock font, e.g. fonts/ar_bigFont for fonts/en_bigFont
		if (*backupFont && (*backupFont)->fontName && letter >= 0x600)
		{
			std::string stockName = (*backupFont)->fontName;
			if (stockName.starts_with("fonts/"))
			{
				stockName.erase(0, 6);
			}
			if (stockName.size() > 3 && stockName[2] == '_')
			{
				stockName.erase(0, 3);
			}

			if (auto* font = FindLoadedFont(std::format("fonts/ar_{}", stockName).data()))
			{
				if (auto* glyph = FindGlyph(font, letter))
				{
					*backupFont = font;
					return glyph;
				}
			}
		}

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
				const auto replacement = std::format("fonts/{}_{}", prefix->second, stockName);
				auto* font = FindLoadedFont(replacement.data());

				// Menus register their fonts at startup, maybe before the zone is done loading, so wait for it
				if (font == nullptr && FastFiles::Exists("iw4x_languages"))
				{
					auto* loaded = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_FONT, replacement.data()).font;
					if (loaded && loaded->fontName && replacement == loaded->fontName)
					{
						font = loaded;
					}
				}

				header.font = font;
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
