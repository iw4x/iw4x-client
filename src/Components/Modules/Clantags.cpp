#include "STDInclude.hpp"

namespace Components
{
	std::string Clantags::Tags[18];

	void Clantags::ParseClantags(const char* infoString)
	{
		for (int i = 0; i < 18; i++)
		{
			const char* clantag = Game::Info_ValueForKey(infoString, std::to_string(i).data());

			if (clantag) Clantags::Tags[i] = clantag;
			else Clantags::Tags[i].clear();
		}
	}

	void Clantags::SendClantagsToClients()
	{
		std::string list;

		for (int i = 0; i < 18; ++i)
		{
			char clantag[5] = { 0 };

			if (Game::svs_clients[i].state >= 3)
			{
				strncpy_s(clantag, Game::Info_ValueForKey(Game::svs_clients[i].connectInfoString, "iw4x_clantag"), 4);
			}

			list.append(Utils::String::VA("\\%s\\%s", std::to_string(i).data(), clantag));
		}

		std::string command = Utils::String::VA("%c clantags \"%s\"", 22, list.data());
		Game::SV_GameSendServerCommand(-1, 0, command.data());
	}

	const char* Clantags::GetUserClantag(std::uint32_t clientnum, const char* playername)
	{
		if (Clantags::Tags[clientnum].empty()) return playername;
		return Utils::String::VA("[%s] %s", Clantags::Tags[clientnum].data(), playername);
	}

	__declspec(naked) void Clantags::DrawPlayerNameOnScoreboard()
	{
		__asm
		{
			push eax
			pushad

			push edi
			push [ebp]

			call Clantags::GetUserClantag
			add esp, 8

			mov [esp + 20h], eax

			popad
			pop edi

			push 591247h // Return address
			push 5909E0h // Draw string func
			retn
		}
	}

	Clantags::Clantags()
	{
		// Create clantag dvar
		Dvar::OnInit([]()
		{
			Dvar::Register<const char*>("iw4x_clantag", "", Game::dvar_flag::DVAR_FLAG_USERINFO | Game::dvar_flag::DVAR_FLAG_SAVED, "If set, your clantag will be shown on the scoreboard.");
		});

		// Servercommand hook
		ServerCommands::OnCommand(22, [](Command::Params* params)
		{
			if (params->get(1) == "clantags"s && !Dedicated::IsEnabled())
			{
				if (params->length() == 3)
				{
					Clantags::ParseClantags(params->get(2));
					return true;
				}
			}

			return false;
		});

		for (int i = 0; i < ARRAYSIZE(Clantags::Tags); ++i)
		{
			Clantags::Tags[i].clear();
		}
		
		// Draw clantag before playername
		Utils::Hook(0x591242, Clantags::DrawPlayerNameOnScoreboard).install()->quick();
	}

	Clantags::~Clantags()
	{
		for (int i = 0; i < ARRAYSIZE(Clantags::Tags); ++i)
		{
			Clantags::Tags[i].clear();
		}
	}
}
