#include <STDInclude.hpp>
#include "CardTitles.hpp"
#include "Events.hpp"
#include "ServerCommands.hpp"

namespace Components
{
	char CardTitles::CustomTitles[Game::MAX_CLIENTS][18];
	Dvar::Var CardTitles::CustomTitle;

	CClient* CardTitles::GetClientByIndex(std::uint32_t index)
	{
		return &reinterpret_cast<CClient*>(0x8E77B0)[index];
	}

	int CardTitles::GetPlayerCardClientInfo(int lookupResult, Game::PlayerCardData* data)
	{
		auto result = lookupResult;

		const auto* username = Dvar::Name.get<const char*>();
		if (std::strcmp(data->name, username) == 0)
		{
			result += 0xFE000000;
		}
		else
		{
			for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
			{
				CClient* c = GetClientByIndex(i);
				if (c != nullptr)
				{
					if (!std::strcmp(data->name, c->Name))
					{
						// Since a 4 byte integer is overkill for a row num: We can use it to store the customprefix + clientNum and use a 2 byte integer for the row number
						result += 0xFF000000;
						result += i * 0x10000;
						break;
					}
				}
			}
		}

		return result;
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

			retn
		}
	}

	const char* CardTitles::TableLookupByRowHook(Game::Operand* operand, tablelookuprequest_s* request)
	{
		std::uint8_t prefix = (request->tableRow >> (8 * 3)) & 0xFF;
		std::uint8_t data = (request->tableRow >> (8 * 2)) & 0xFF;

		if (data >= Game::MAX_CLIENTS) return nullptr;

		if (request->tablename == "mp/cardTitleTable.csv"s)
		{
			if (prefix != 0x00)
			{
				// Column 1 = CardTitle
				if (request->tableColumn == 1)
				{
					if (prefix == 0xFE)
					{
						if (!CustomTitle.get<std::string>().empty())
						{
							// 0xFF in front of the title to skip localization. Or else it will wait for a couple of seconds for the asset of type localize
							const auto* title = Utils::String::VA("\x15%s", CustomTitle.get<const char*>());

							// prepare return value
							operand->internals.stringVal.string = title;
							operand->dataType = Game::VAL_STRING;

							return title;
						}
					}
					else if (prefix == 0xFF)
					{
						if (CustomTitles[data][0] != '\0')
						{
							const auto* title = Utils::String::VA("\x15%s", CustomTitles[data]);

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
			jz originalTitle

			pop ecx
			mov ecx, dword ptr[esi + 4]
			retn

		originalTitle:
			mov eax, [esi + 50h]
			cmp eax, 3

			push 62DCC7h
			retn
		}
	}

	void CardTitles::SendCustomTitlesToClients()
	{
		std::string list;

		for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			char playerTitle[18]{};

			if (Game::svs_clients[i].userinfo[0] != '\0')
			{
				strncpy_s(playerTitle, Game::Info_ValueForKey(Game::svs_clients[i].userinfo, "customTitle"), _TRUNCATE);
			}
			else
			{
				playerTitle[0] = '\0';
			}

			list.append(std::format("\\{}\\{}", i, playerTitle));
		}

		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} customTitles \"{}\"", 21, list));
	}

	void CardTitles::ParseCustomTitles(const char* msg)
	{
		for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			const auto index = std::to_string(i);
			const auto* playerTitle = Game::Info_ValueForKey(msg, index.data());

			if (playerTitle[0] == '\0')
			{
				CustomTitles[i][0] = '\0';
			}
			else
			{
				Game::I_strncpyz(CustomTitles[i], playerTitle, sizeof(CustomTitles[0]) / sizeof(char));
			}
		}
	}

	CardTitles::CardTitles()
	{
		Events::OnDvarInit([]
		{
			CustomTitle = Dvar::Register<const char*>("customTitle", "", Game::DVAR_USERINFO | Game::DVAR_ARCHIVE, "Custom card title");
		});

		std::memset(&CustomTitles, 0, sizeof(char[Game::MAX_CLIENTS][18]));

		ServerCommands::OnCommand(21, [](const Command::Params* params)
		{
			if (std::strcmp(params->get(1), "customTitles") == 0)
			{
				if (params->size() == 3)
				{
					ParseCustomTitles(params->get(2));
					return true;
				}
			}

			return false;
		});

		Utils::Hook(0x62EB26, GetPlayerCardClientInfoStub).install()->quick();

		// Table lookup stuff
		Utils::Hook(0x62DCC1, TableLookupByRowHookStub).install()->quick();
		Utils::Hook::Nop(0x62DCC6, 1);
	}
}
