#pragma once

namespace Components
{
	// Plays the game in the language of the selected translation (loc_translation) when its official files are installed:
	// voices from main/localized_<language>_iw*.iwd and zones from zone/iw4x/x86/<language>.
	// Anything a language does not have comes from English.
	class LanguageVoices : public Component
	{
	public:
		LanguageVoices();

		// The game language in use, e.g. "german"
		static const char* GetLanguage();

	private:
		static std::string Language;
		static int LanguageIndex;

		static std::optional<std::string> ReadTranslationChoice();
		static bool IsInstalled(const std::string& language);

		static Game::dvar_t* RegisterLanguageDvar(const char* name, int value, int min, int max, unsigned int flags, const char* description);
	};
}
