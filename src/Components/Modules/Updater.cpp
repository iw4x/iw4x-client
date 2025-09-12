#include "Updater.hpp"
#include "Scheduler.hpp"
#include "version.hpp"

#include <Utils/WebIO.hpp>

#include <rapidjson/document.h>

namespace Components
{
	namespace
	{
		const Game::dvar_t* cl_updateAvailable;

		constexpr auto* GITHUB_REMOTE_URL = "https://api.github.com/repos/iw4x/iw4x-client/releases/latest";

		void CheckForUpdate()
		{
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

			const std::string tag = doc["tag_name"].GetString();
			if (REVISION_STR != tag)
			{
				// A new version came out!
				Game::Dvar_SetBool(cl_updateAvailable, true);
			}
		}

		// Depending on Linux/Windows 32/64 there are a few things we must check
		std::optional<std::string> GetLauncher()
		{
			const char* launchers[] = {
				"iw4x-launcher.exe",
				"iw4x-launcher-x86.exe",
				Utils::IsWineEnvironment() ? "iw4x-launcher" : nullptr
			};

			for (const char* launcher : launchers) {
				if (launcher && Utils::IO::FileExists(launcher)) {
					return launcher;
				}
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
			const auto exe = GetLauncher();
			if (exe.has_value())
			{
				Game::Sys_QuitAndStartProcess(exe.value().data());
				return;
			}

			// No launcher was found on the system
			Logger::Print("No launcher found - cannot auto-update\n");
		});
	}
}
