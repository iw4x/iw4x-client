#include "LanguageVoices.hpp"
#include "ZoneBuilder.hpp"

namespace Components
{
	std::string LanguageVoices::Language;
	int LanguageVoices::LanguageIndex = 0;

	namespace
	{
		// The game's languages, in loc_language order
		constexpr const char* GAME_LANGUAGES[]
		{
			"english", "french", "german", "italian", "spanish", "british", "russian", "polish",
			"korean", "taiwanese", "japanese", "chinese", "thai", "leet", "czech",
		};

		std::filesystem::path GameFolder()
		{
			char path[MAX_PATH]{};
			GetModuleFileNameA(nullptr, path, sizeof(path));
			return std::filesystem::path(path).parent_path();
		}
	}

	const char* LanguageVoices::GetLanguage()
	{
		return Language.data();
	}

	std::optional<std::string> LanguageVoices::ReadTranslationChoice()
	{
		// The language is needed before any dvar or config exists, so read the choice the way the game will:
		// the command line first (iw4x-language.bat), then the saved config
		static const std::regex commandLineChoice(R"(loc_translation\s+"?([A-Za-z]*))");
		static const std::regex configChoice(R"(seta\s+loc_translation\s+"([A-Za-z]*)\")");

		std::smatch match;
		const std::string commandLine = GetCommandLineA();
		if (std::regex_search(commandLine, match, commandLineChoice))
		{
			return Utils::String::ToLower(match[1].str());
		}

		const auto config = Utils::IO::ReadFile((GameFolder() / "players" / "iw4x_config.cfg").string());
		if (std::regex_search(config, match, configChoice))
		{
			return Utils::String::ToLower(match[1].str());
		}

		return {};
	}

	bool LanguageVoices::IsInstalled(const std::string& language)
	{
		const auto folder = GameFolder();
		std::error_code error;
		return std::filesystem::exists(folder / "main" / std::format("localized_{}_iw00.iwd", language), error)
			&& std::filesystem::is_directory(folder / "zone" / "iw4x" / "x86" / language, error);
	}

	Game::dvar_t* LanguageVoices::RegisterLanguageDvar(const char* name, [[maybe_unused]] const int value, const int min, const int max, const unsigned int flags, const char* description)
	{
		// Read only and not saved, so an older loc_language in the config cannot switch the files mid-game
		const auto languageFlags = (flags & ~(Game::DVAR_ARCHIVE | Game::DVAR_LATCH)) | Game::DVAR_ROM;
		auto* dvar = Utils::Hook::Call<Game::dvar_t*(const char*, int, int, int, unsigned int, const char*)>(0x479830)(name, LanguageIndex, min, max, languageFlags, description);
		if (dvar)
		{
			dvar->current.integer = LanguageIndex;
			dvar->latched.integer = LanguageIndex;
			dvar->reset.integer = LanguageIndex;
		}

		return dvar;
	}

	LanguageVoices::LanguageVoices()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		// loc_language: which localized_<language>_iw*.iwd files the file system uses. Always set from the
		// translation, a value saved in the config would otherwise keep a language whose files are not wanted
		Utils::Hook(0x486172, RegisterLanguageDvar, HOOK_CALL).install()->quick();

		const auto choice = ReadTranslationChoice();
		if (!choice || choice->empty())
		{
			return;
		}

		const auto* language = std::ranges::find(GAME_LANGUAGES, *choice);
		if (language == std::end(GAME_LANGUAGES) || *choice == "english" || !IsInstalled(*choice))
		{
			return;
		}

		Language = *choice;
		LanguageIndex = static_cast<int>(language - std::begin(GAME_LANGUAGES));

		// Win_GetLanguage: zone folders and the localized zones
		Utils::Hook(0x45CBA0, GetLanguage, HOOK_JUMP).install()->quick();
	}
}
