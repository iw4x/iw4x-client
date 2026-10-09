#pragma once

namespace Components
{
	// Swaps the stock fonts for the fonts of the selected translation (loc_translation) and keeps backup
	// fonts for letters the current font does not have
	class LanguageFonts : public Component
	{
	public:
		// Zones with the replacement fonts, named "fonts/<prefix>_<stock name>"
		static constexpr const char* ZONE_NAMES[] = { "iw4x_arabic", "iw4x_languages" };

		LanguageFonts();

		// Finds a letter in the backup fonts, returns nullptr when none of them has it
		static Game::Glyph* FindBackupGlyph(unsigned int letter, Game::Font_s** backupFont);

		static Game::Glyph* FindGlyph(const Game::Font_s* font, unsigned int letter);

	private:
		static Dvar::Var LocLanguageFonts;

		static Game::XAssetHeader FindFont(Game::XAssetType type, const std::string& name);
	};
}
