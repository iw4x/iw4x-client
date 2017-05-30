#include "STDInclude.hpp"

namespace Components
{
	std::vector < std::string > CardTitles::CustomTitles;
	Game::dvar_t* CardTitles::CustomTitleDvar;

	CClient* CardTitles::GetClientByIndex(std::uint32_t index)
	{
		return reinterpret_cast<CClient*>(0x8E77B0 + (sizeof CClient * index));
	}

	std::int32_t CardTitles::GetPlayerCardClientInfo(std::int32_t lookupResult, playercarddata_s* data)
	{
		std::int32_t returnResult = lookupResult;

		CClient* c;
		std::string username = Dvar::Var("name").get<std::string>();

		if (data->name == username)
		{
			returnResult += 0xFE000000;
		}
		else 
		{
			for (auto clientNum = 0; clientNum < 18; clientNum++)
			{
				c = GetClientByIndex(clientNum);

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
			push esi;
			push eax;
			call GetPlayerCardClientInfo;
			add esp, 8;

			pop esi;
			pop ebp;
			mov[ebx + 4], eax;
			pop ebx;

			push 62EB2Ch;
			retn;
		}
	}

	const char* CardTitles::TableLookupByRowHook(Game::Operand* operand, tablelookuprequest_s* request)
	{
		std::uint8_t prefix =	(request->tableRow >> (8 * 3)) & 0xFF;
		std::uint8_t data =		(request->tableRow >> (8 * 2)) & 0xFF;
		const char* title = nullptr;

		if (request->tablename == "mp/cardTitleTable.csv"s)
		{
			if (prefix != 0x00)
			{
				// Column 1 = CardTitle
				if (request->tableColumn == 1)
				{
					if (prefix == 0xFE)
					{
						// 0xFF in front of the title to skip localization. Or else it will wait for a couple of seconds for the asset of type localize
						if (*CustomTitleDvar->current.string)
						{
							title = Utils::String::VA("\x15%s", CustomTitleDvar->current.string);

							// prepare return value
							operand->internals.stringVal.string = title;
							operand->dataType = Game::VAL_STRING;

							return title;
						}
					}
					else if (prefix == 0xFF)
					{
						if (!CustomTitles[data].empty())
						{
							title = Utils::String::VA("\x15%s", CustomTitles[data].data());

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

	void __declspec(naked) CardTitles::TableLookupByRowHookStub()
	{
		__asm 
		{
			push esi;
			push ebx;
			
			pushad;
			call TableLookupByRowHook;
			cmp eax, 0;
			popad;

			jz OriginalTitle;

			add esp, 8;

			pop ecx;
			mov ecx, DWORD ptr[esi + 4];
			retn;

		OriginalTitle:

			add esp, 8;

			mov eax, [esi + 50h];
			cmp eax, 3;

			push 62DCC7h;
			retn;
		}
	}

	void CardTitles::SendCustomTitlesToClients() 
	{
		const char* list = "";

		for (int i = 0; i < 18; i++)
		{
			char playerTitle[18] = { 0 };

			if (Game::svs_clients[i].state >= 3)
			{
				strncpy_s(playerTitle, Game::Info_ValueForKey(Game::svs_clients[i].connectInfoString, "customTitle"), 18);
			}
			else
			{
				memset(playerTitle, 0, 18);
			}

			list = Utils::String::VA("%s\\%s\\%s", list, std::to_string(i).c_str(), playerTitle);
		}

		list = Utils::String::VA("%c customTitles \"%s", 20, list);
		Game::SV_GameSendServerCommand(-1, 0, list);
	}

	void CardTitles::ParseCustomTitles(const char* msg) 
	{
		const char* playerTitle;
		for (int i = 0; i < 18; i++) 
		{
			playerTitle = Game::Info_ValueForKey(msg, std::to_string(i).c_str());

			if (playerTitle != nullptr) 
			{
				CustomTitles[i] = playerTitle;
			}
			else 
			{
				CustomTitles[i] = "";
			}
		}
	}

	CardTitles::CardTitles()
	{
		Dvar::OnInit([]() {
			CardTitles::CustomTitleDvar = Game::Dvar_RegisterString("cardtitle", "", Game::dvar_flag::DVAR_FLAG_SAVED, "Custom card title");
		});

		ServerCommands::OnCommand(20, [](Command::Params* params) {

			if (params->get(1) == "customTitles"s && !Flags::HasFlag("dedicated"))
			{
				if (params->length() == 3)
				{
					CardTitles::ParseCustomTitles(params->get(2));
					return true;
				}
			}

			return false;

		});

		CardTitles::CustomTitles.resize(18);

		Utils::Hook(0x62EB26, GetPlayerCardClientInfoStub).install()->quick();

		// Translation fixup
		// Utils::Hook(0x63A2D9, LocalizationSkipHookStub).install()->quick();
		// Utils::Hook::Nop(0x63A2D5, 3);

		// Table lookup stuff
		Utils::Hook(0x62DCC1, TableLookupByRowHookStub).install()->quick();
		Utils::Hook::Nop(0x62DCC6, 1);
	}

	CardTitles::~CardTitles()
	{
		CustomTitles.clear();
	}
}