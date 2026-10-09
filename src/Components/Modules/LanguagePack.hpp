#pragma once

namespace Components
{
	// ZoneBuilder commands that turn the official language data and fonts into the zones and files the game loads:
	//   buildlanguages [language pack folder] [Noto font folder]   default: languagepack and languagepack/noto
	//   buildarabicfonts [Arabic TrueType font]                    default: languagepack/NotoKufiArabic.ttf
	class LanguagePack : public Component
	{
	public:
		LanguagePack();

	private:
		static void BuildLanguages(const std::filesystem::path& pack, const std::filesystem::path& noto);
		static void BuildArabicFonts(const std::filesystem::path& font);

		static void BuildZone(const std::string& zone, const std::vector<std::string>& fonts);
		static void RemoveTemporaryFiles();
	};
}
