#include <STDInclude.hpp>

namespace Components
{
	std::string ClanTags::Tags[18];

	void ClanTags::ParseClantags(const char* infoString)
	{
		for (int i = 0; i < 18; i++)
		{
			const char* clantag = Game::Info_ValueForKey(infoString, std::to_string(i).data());

			if (clantag) ClanTags::Tags[i] = clantag;
			else ClanTags::Tags[i].clear();
		}
	}

	void ClanTags::SendClantagsToClients()
	{
		std::string list;

		for (int i = 0; i < 18; ++i)
		{
			char clantag[5] = { 0 };

			if (Game::svs_clients[i].state >= 3)
			{
				strncpy_s(clantag, Game::Info_ValueForKey(Game::svs_clients[i].connectInfoString, "clantag"), 4);
			}

			list.append(Utils::String::VA("\\%s\\%s", std::to_string(i).data(), clantag));
		}

		std::string command = Utils::String::VA("%c clantags \"%s\"", 22, list.data());
		Game::SV_GameSendServerCommand(-1, 0, command.data());
	}

	const char* ClanTags::GetUserClantag(std::uint32_t /*clientnum*/, const char* playername)
	{
#if 0
		if (ClanTags::Tags[clientnum].empty()) return playername;
		return Utils::String::VA("[%s] %s", ClanTags::Tags[clientnum].data(), playername);
#else
		return playername;
#endif

	}

	__declspec(naked) void ClanTags::DrawPlayerNameOnScoreboard()
	{
		__asm
		{
			push eax
			pushad

			push edi
			push [ebp]

			call ClanTags::GetUserClantag
			add esp, 8

			mov [esp + 20h], eax

			popad
			pop edi

			push 591247h // Return address
			push 5909E0h // Draw string func
			retn
		}
	}

	ClanTags::ClanTags()
	{
		// Create clantag dvar
		Scheduler::Once([]
		{
			Dvar::Register<const char*>("clantag", "", Game::dvar_flag::DVAR_USERINFO | Game::dvar_flag::DVAR_ARCHIVE,
				"If set, your clantag will be shown on the scoreboard.");
		}, Scheduler::Pipeline::MAIN);

		// Servercommand hook
		ServerCommands::OnCommand(22, [](Command::Params* params)
		{
			if (params->get(1) == "clantags"s && !Dedicated::IsEnabled())
			{
				if (params->size() == 3)
				{
					ClanTags::ParseClantags(params->get(2));
					return true;
				}
			}

			return false;
		});

		// Draw clantag before playername
		Utils::Hook(0x591242, ClanTags::DrawPlayerNameOnScoreboard).install()->quick();
	}
}
