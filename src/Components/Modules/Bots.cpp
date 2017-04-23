#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> Bots::BotNames;

	void Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port)
	{
		static int botId = 0;

		if (Bots::BotNames.empty())
		{
			FileSystem::File bots("bots.txt");

			if (bots.exists())
			{
				std::vector<std::string> names = Utils::String::Explode(bots.getBuffer(), '\n');

				for (auto name : names)
				{
					Utils::String::Replace(name, "\r", "");
					name = Utils::String::Trim(name);

					if (!name.empty())
					{
						Bots::BotNames.push_back(name);
					}
				}
			}

			if (Bots::BotNames.empty())
			{
				Bots::BotNames.push_back("bot");
			}
		}

		botId %= Bots::BotNames.size();
		strncpy_s(buffer, 0x400, Utils::String::VA(connectString, num, Bots::BotNames[botId++].data(), protocol, checksum, statVer, statStuff, port), 0x400);
	}

	unsigned int Bots::Spawn(unsigned int count)
	{
		int error = 0;
		std::vector<Game::gentity_t*> entRefs;

		for (unsigned int i = 0; i < count; ++i)
		{
			std::this_thread::sleep_for(10ms);

			Game::gentity_t* entRef = Game::SV_AddTestClient();
			if (entRef) entRefs.push_back(entRef);
			else if (++error == 3) break;
		}

		for (auto& entRef : entRefs)
		{
			Renderer::OnDelay([entRef]()
			{
				Game::Scr_AddString("autoassign");
				Game::Scr_AddString("team_marinesopfor");
				Game::Scr_Notify(entRef, Game::SL_GetString("menuresponse", 0), 2);

				Renderer::OnDelay([entRef]()
				{
					Game::Scr_AddString(Utils::String::VA("class%d", Utils::Cryptography::Rand::GenerateInt() % 5));
					Game::Scr_AddString("changeclass");
					Game::Scr_Notify(entRef, Game::SL_GetString("menuresponse", 0), 2);
				}, 1s);
			}, 1s);
		}

		return entRefs.size();
	}

	Bots::Bots()
	{
		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectString, HOOK_CALL).install()->quick();

		Command::Add("spawnBot", [](Command::Params* params)
		{
			unsigned int count = 1;

			if (params->length() > 1)
			{
				if (params->get(1) == "all"s) count = static_cast<unsigned int>(-1);
				else count = atoi(params->get(1));
			}

			// Check if ingame and host
			if (!Game::SV_Loaded())
			{
				Toast::Show("cardicon_headshot", "^1Error", "You need to be host to spawn bots!", 3000);
				Logger::Print("You need to be host to spawn bots!\n");
			}

			count = Bots::Spawn(count);
			if (count)
			{
				Toast::Show("cardicon_headshot", "^2Success", Utils::String::VA("%d %s spawned", count, (count == 1 ? "bot" : "bots")), 3000);
				Logger::Print("%d %s spawned successfully!\n", count, (count == 1 ? "bot" : "bots"));
			}
			else
			{
				Toast::Show("cardicon_headshot", "^1Error", "Unable to spawn new bots!", 3000);
				Logger::Print("Unable to spawn new bots!");
			}
		});
	}

	Bots::~Bots()
	{
		Bots::BotNames.clear();
	}
}
