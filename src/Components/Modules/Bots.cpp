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

	Bots::Bots()
	{
		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");
		
		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectString, HOOK_CALL).install()->quick();
	}

	Bots::~Bots()
	{
		Bots::BotNames.clear();
	}
}
