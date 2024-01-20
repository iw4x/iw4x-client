#include <STDInclude.hpp>

#include "Updater.hpp"
#include "Scheduler.hpp"

#include <Utils/WebIO.hpp>

#include <rapidjson/document.h>

namespace Components
{
	namespace
	{
		const Game::dvar_t* cl_updateAvailable;

		// If they use the alterware-launcher once to install they will have this file
		// If they don't, what are they waiting for?
		constexpr auto* REVISION_FILE = ".iw4xrevision";
		constexpr auto* GITHUB_REMOTE_URL = "https://api.github.com/repos/iw4x/iw4x-client/releases/latest";
		constexpr auto* INSTALL_GUIDE_REMOTE_URL = "https://forum.alterware.dev/t/how-to-install-the-alterware-launcher/56";

		void CheckForUpdate()
		{
			std::string revision;
			if (!Utils::IO::ReadFile(REVISION_FILE, &revision) || revision.empty())
			{
				Logger::Print("{} does not exist. Notifying the user an update is available\n", REVISION_FILE);
				Game::Dvar_SetBool(cl_updateAvailable, true);
			}

			const auto result = Utils::WebIO("IW4x", GITHUB_REMOTE_URL).setTimeout(5000)->get();
			if (result.empty())
			{
				// Nothing to do in this situation. We won't know if we need to update or not
				Logger::Print("Could not fetch latest tag from GitHub\n");
				return;
			}

			rapidjson::Document doc{};
			const rapidjson::ParseResult parseResult = doc.Parse(result);
			if (!parseResult || !doc.IsObject())
			{
				// Nothing to do in this situation. We won't know if we need to update or not
				Logger::Print("GitHub sent an invalid reply (malformed JSON)\n");
				return;
			}

			if (!doc.HasMember("tag_name") || !doc["tag_name"].IsString())
			{
				// Nothing to do in this situation. We won't know if we need to update or not
				Logger::Print("GitHub sent an invalid reply (missing 'tag_name' JSON member)\n");
				return;
			}

			const auto* tag = doc["tag_name"].GetString();
			if (revision != tag)
			{
				// A new version came out!
				Game::Dvar_SetBool(cl_updateAvailable, true);
			}
		}

		// Depending on Linux/Windows 32/64 there are a few things we must check
		std::optional<std::string> GetLauncher()
		{
			if (Utils::IO::FileExists("alterware-launcher.exe"))
			{
				return "alterware-launcher.exe";
			}

			if (Utils::IO::FileExists("alterware-launcher-x86.exe"))
			{
				return "alterware-launcher-x86.exe";
			}

			if (Utils::IsWineEnvironment() && Utils::IO::FileExists("alterware-launcher"))
			{
				return "alterware-launcher";
			}

			return {};
		}
	}

	Updater::Updater()
	{
		cl_updateAvailable = Game::Dvar_RegisterBool("cl_updateAvailable", false, Game::DVAR_NONE, "Whether an update is available or not");
		Scheduler::Once(CheckForUpdate, Scheduler::Pipeline::ASYNC);

		UIScript::Add("checkForUpdate", [](const UIScript::Token& /*token*/, const Game::uiInfo_s* /*info*/)
		{
			CheckForUpdate();
		});

		UIScript::Add("getAutoUpdate", [](const UIScript::Token& /*token*/, const Game::uiInfo_s* /*info*/)
		{
			if (const auto exe = GetLauncher(); exe.has_value())
			{
				Game::Sys_QuitAndStartProcess(exe.value().data());
				return;
			}

			// No launcher was found on the system, time to tell them to download it from GitHub
			Utils::OpenUrl(INSTALL_GUIDE_REMOTE_URL);
		});
	}
}
