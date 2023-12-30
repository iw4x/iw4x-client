#include <STDInclude.hpp>

#include "Updater.hpp"
#include "Scheduler.hpp"

#include <Utils/WebIO.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace Components
{
	namespace
	{
		const Game::dvar_t* cl_updateAvailable;

		// If they use the alterware-launcher once to install they will have this file
		// If they don't, what are they waiting for?
		constexpr auto* revisionFile = ".iw4xrevision";

		void CheckForUpdate()
		{
			std::string revision;
			if (!Utils::IO::ReadFile(revisionFile, &revision) || revision.empty())
			{
				Logger::Print(".iw4xrevision does not exist. Notifying the user an update is available\n");
				Game::Dvar_SetBool(cl_updateAvailable, true);
			}

			const auto result = Utils::WebIO("IW4x", "https://api.github.com/repos/iw4x/iw4x-client/releases/latest").setTimeout(5000)->get();
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
				Logger::Print("GitHub sent an invalid reply\n");
				return;
			}

			if (!doc.HasMember("tag_name") || !doc["tag_name"].IsString())
			{
				// Nothing to do in this situation. We won't know if we need to update or not
				Logger::Print("GitHub sent an invalid reply\n");
				return;
			}

			const auto* tag = doc["tag_name"].GetString();
			if (revision != tag)
			{
				// A new version came out!
				Game::Dvar_SetBool(cl_updateAvailable, true);
			}
		}

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
			Utils::OpenUrl("https://forum.alterware.dev/t/how-to-install-the-alterware-launcher/56");
		});
	}
}
