#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> Bots::BotNames;

	void Bots::InsertBotName(const char* arg)
	{
		const char** args = &arg;
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

			if(Bots::BotNames.empty())
			{
				Bots::BotNames.push_back("bot");
			}
		}

		int botId = reinterpret_cast<int>(args[12]);
		args[12] = Utils::String::VA("%s", Bots::BotNames[botId % Bots::BotNames.size()].data());
	}

	__declspec(naked) void Bots::BuildConnectStringStub()
	{
		__asm
		{
			pushad
			call Bots::InsertBotName
			popad

			push 6B5D80h // sprintf
			retn
		}
	}

	Bots::Bots()
	{
		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");
		
		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectStringStub, HOOK_CALL).install()->quick();
	}

	Bots::~Bots()
	{
		Bots::BotNames.clear();
	}
}
