#include <STDInclude.hpp>

namespace Components
{
	std::string CardTitles::CustomTitles[18];
	Dvar::Var CardTitles::CustomTitleDvar;

	CClient* CardTitles::GetClientByIndex(std::uint32_t index)
	{
		return &reinterpret_cast<CClient*>(0x8E77B0)[index];
	}

	std::int32_t CardTitles::GetPlayerCardClientInfo(std::int32_t lookupResult, playercarddata_s* data)
	{
		std::int32_t returnResult = lookupResult;

		std::string username = Dvar::Var("name").get<std::string>();

		if (data->name == username)
		{
			returnResult += 0xFE000000;
		}
		else
		{
			for (auto clientNum = 0; clientNum < 18; clientNum++)
			{
				CClient* c = GetClientByIndex(clientNum);
				if (c != nullptr)
				{
					if (!strcmp(data->name, c->Name))
					{
						// Since a 4 byte integer is overkill for a row num: We can use it to store the customprefix + clientNum and use a 2 byte integer for the row number
						returnResult += 0xFF000000;
						returnResult += clientNum * 0x10000;
						break;
					}
				}
			}
		}

		return returnResult;
	}

	void __declspec(naked) CardTitles::GetPlayerCardClientInfoStub()
	{
		__asm
		{
			push eax
			pushad

			push esi
			push eax
			call GetPlayerCardClientInfo
			add esp, 8

			mov [esp + 20h], eax
			popad
			pop eax

			pop esi
			pop ebp
			mov [ebx + 4], eax
			pop ebx

			push 62EB2Ch
			retn
		}
	}

	const char* CardTitles::TableLookupByRowHook(Game::Operand* operand, tablelookuprequest_s* request)
	{
		std::uint8_t prefix = (request->tableRow >> (8 * 3)) & 0xFF;
		std::uint8_t data = (request->tableRow >> (8 * 2)) & 0xFF;

		if (data >= ARRAYSIZE(CardTitles::CustomTitles)) return nullptr;

		if (request->tablename == "mp/cardTitleTable.csv"s)
		{
			if (prefix != 0x00)
			{
				// Column 1 = CardTitle
				if (request->tableColumn == 1)
				{
					if (prefix == 0xFE)
					{
						if (!CardTitles::CustomTitleDvar.get<std::string>().empty())
						{
							// 0xFF in front of the title to skip localization. Or else it will wait for a couple of seconds for the asset of type localize
							const char* title = Utils::String::VA("\x15%s", CardTitles::CustomTitleDvar.get<const char*>());

							// prepare return value
							operand->internals.stringVal.string = title;
							operand->dataType = Game::VAL_STRING;

							return title;
						}
					}
					else if (prefix == 0xFF)
					{
						if (!CardTitles::CustomTitles[data].empty())
						{
							const char* title = Utils::String::VA("\x15%s", CardTitles::CustomTitles[data].data());

							// prepare return value
							operand->internals.stringVal.string = title;
							operand->dataType = Game::VAL_STRING;

							return title;
						}
					}
				}

				// If the title was changed it already returned at this point so...
				// Remove prefix and data to make being readable to the normal lookuprequest
				request->tableRow = static_cast<std::int32_t>(*(reinterpret_cast<WORD*>(&request->tableRow)));
			}
		}

		return nullptr;
	}

	__declspec(naked) void CardTitles::TableLookupByRowHookStub()
	{
		__asm
		{
			push eax
			pushad

			push esi
			push ebx

			call TableLookupByRowHook
			add esp, 8

			mov [esp + 20h], eax
			popad
			pop eax

			cmp eax, 0
			jz OriginalTitle

			pop ecx
			mov ecx, DWORD ptr[esi + 4]
			retn

		OriginalTitle:

			mov eax, [esi + 50h]
			cmp eax, 3

			push 62DCC7h
			retn
		}
	}

	void CardTitles::SendCustomTitlesToClients()
	{
		std::string list;

		for (int i = 0; i < 18; i++)
		{
			char playerTitle[18];

			if (Game::svs_clients[i].state >= 3)
			{
				strncpy_s(playerTitle, Game::Info_ValueForKey(Game::svs_clients[i].connectInfoString, "customTitle"), 17);
			}
			else
			{
				memset(playerTitle, 0, 18);
			}

			list.append(Utils::String::VA("\\%s\\%s", std::to_string(i).c_str(), playerTitle));
		}

		std::string command = Utils::String::VA("%c customTitles \"%s\"", 21, list.data());
		Game::SV_GameSendServerCommand(-1, 0, command.data());
	}

	void CardTitles::ParseCustomTitles(const char* msg)
	{
		for (int i = 0; i < 18; ++i)
		{
			const char* playerTitle = Game::Info_ValueForKey(msg, std::to_string(i).c_str());

			if (playerTitle) CardTitles::CustomTitles[i] = playerTitle;
			else CardTitles::CustomTitles[i].clear();
		}
	}

	CardTitles::CardTitles()
	{
		Dvar::OnInit([]()
		{
			CardTitles::CustomTitleDvar = Dvar::Register<const char*>("customtitle", "", Game::dvar_flag::DVAR_USERINFO | Game::dvar_flag::DVAR_ARCHIVE, "Custom card title");
		});

		ServerCommands::OnCommand(21, [](Command::Params* params)
		{
			if (params->get(1) == "customTitles"s && !Dedicated::IsEnabled())
			{
				if (params->size() == 3)
				{
					CardTitles::ParseCustomTitles(params->get(2));
					return true;
				}
			}

			return false;

		});

		for (int i = 0; i < ARRAYSIZE(CardTitles::CustomTitles); ++i)
		{
			CardTitles::CustomTitles[i].clear();
		}

		Utils::Hook(0x62EB26, CardTitles::GetPlayerCardClientInfoStub).install()->quick();

		// Table lookup stuff
		Utils::Hook(0x62DCC1, CardTitles::TableLookupByRowHookStub).install()->quick();
		Utils::Hook::Nop(0x62DCC6, 1);

        // This is placed here in case the anticheat has been disabled!
        // This checks specifically for launching the process suspended to inject a dll
#if !defined(DISABLE_ANTICHEAT)
        AntiCheat::CheckStartupTime();
#endif
	}

	CardTitles::~CardTitles()
	{
		for (int i = 0; i < ARRAYSIZE(CardTitles::CustomTitles); ++i)
		{
			CardTitles::CustomTitles[i].clear();
		}
	}
}
