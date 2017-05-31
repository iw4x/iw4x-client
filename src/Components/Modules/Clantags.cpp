#include "STDInclude.hpp"

namespace Components
{
	std::vector < std::string > Clantags::Tags;
	const char* CurrentName;

	void Clantags::ParseClantags(const char* infoString)
	{
		const char* clantag;
		for (int i = 0; i < 18; i++)
		{
			clantag = Game::Info_ValueForKey(infoString, std::to_string(i).c_str());

			if (clantag != nullptr)
			{
				Tags[i] = _strdup(clantag);
			}
			else
			{
				Tags[i] = "";
			}
		}
	}

	void Clantags::SendClantagsToClients()
	{
		const char* list = "";

		for (int i = 0; i < 18; i++)
		{
			char clantag[5];

			if (Game::svs_clients[i].state >= 3)
			{
				strncpy_s(clantag, Game::Info_ValueForKey(Game::svs_clients[i].connectInfoString, "iw4x_clantag"), 4);
			}
			else
			{
				memset(clantag, 0, 5);
			}

			list = Utils::String::VA("%s\\%s\\%s", list, std::to_string(i).c_str(), clantag);
		}

		list = Utils::String::VA("%c clantags \"%s\"", 22, list);
		Game::SV_GameSendServerCommand(-1, 0, list);
	}

	void Clantags::GetUserClantag(std::uint32_t clientnum, const char* playername)
	{
		if (Tags[clientnum].empty())
		{
			CurrentName = playername;
			return;
		}

		CurrentName = Utils::String::VA("[%s] %s", Tags[clientnum].data(), playername);
	}

	void __declspec(naked) Clantags::DrawPlayerNameOnScoreboard()
	{
		static DWORD drawstringfunc = 0x5909E0;

		__asm
		{
			pushad;

			push edi;
			push [ebp];

			call GetUserClantag;
			add esp, 8;

			popad;

			mov edi, CurrentName;

			call drawstringfunc;

			push 591247h;
			retn;
		}
	}

	Clantags::Clantags()
	{
		// Create clantag dvar
		Dvar::OnInit([]() {
			CardTitles::CustomTitleDvar = Game::Dvar_RegisterString("iw4x_clantag", "", Game::dvar_flag::DVAR_FLAG_USERINFO | Game::dvar_flag::DVAR_FLAG_SAVED, "If set, your clantag will be shown on the scoreboard.");
		});

		// Servercommand hook
		ServerCommands::OnCommand(22, [](Command::Params* params) {

			if (params->get(1) == "clantags"s && !Flags::HasFlag("dedicated"))
			{
				if (params->length() == 3)
				{
					Clantags::ParseClantags(params->get(2));
					return true;
				}
			}

			return false;

		});

		// Resize clantags array
		Tags.resize(18);
		
		// Draw clantag before playername
		Utils::Hook(0x591242, DrawPlayerNameOnScoreboard).install()->quick();
	}

	Clantags::~Clantags()
	{
		Tags.clear();
	}
}
