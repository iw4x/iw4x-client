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
			add esp, 0x08;

			pop esi;
			pop ebp;
			mov[ebx + 0x4], eax;
			pop ebx;

			push 0x62EB2C;
			retn;
		}
	}

	void __declspec(naked) CardTitles::LocalizationSkipHookStub()
	{
		__asm 
		{
			cmp byte ptr[edi + 0x01], 0xFF; // Check if skip prefix exists (edi + 0x00 = @)
			jne back;
			add edi, 0x02;					// Ignore the 0x40 and 0xFF prefix (Localize and Skip prefix)
			jmp jumpback;

		back:
			add edi, 0x01;
			push edi;

			mov eax, 0x4F1700;
			call eax;

			add esp, 0x04;
			mov edi, eax;

		jumpback:
			push 0x63A2E3;
			retn;
		}
	}

	void __declspec(naked) CardTitles::TableLookupByRowHookStub()
	{
		static tablelookuprequest_s*	currentLookupRequest;
		static tablelookupresult_s*		currentLookupResult;
		static std::int32_t				prefix;
		static std::int32_t				data;
		static const char*				title;

		__asm 
		{
			mov currentLookupRequest, esi;
			mov currentLookupResult, ebx;
		}

		prefix = (currentLookupRequest->tableRow >> (8 * 3)) & 0xff;
		data = (currentLookupRequest->tableRow >> (8 * 2)) & 0xff;

		//So we don't accidentally mess up other tables that might have a ridiculous amount of rows. Who knows.
		if (!strcmp(currentLookupRequest->tablename, "mp/cardTitleTable.csv")) 
		{
			if (prefix != 0x00) 
			{
				// Column 1 = CardTitle
				if (currentLookupRequest->tableColumn == 1) 
				{ 
					if (prefix == 0xFE) 
					{
						// 0xFF in front of the title to skip localization. Or else it will wait for a couple of seconds for the asset of type localize
						if ((BYTE)*CustomTitleDvar->current.string) 
						{
							title = Utils::String::VA("\xFF%s", CustomTitleDvar->current.string);
							currentLookupResult->result = title;
							currentLookupResult->dunno = 2; // Seems to be nessecary. Don't ask me why.

							__asm 
							{
								mov eax, title;
								pop ecx;
								mov ecx, currentLookupRequest;
								mov ecx, DWORD ptr[ecx + 0x04];
								retn;
							}
						}
					}
					else if (prefix == 0xFF) 
					{
						if (!CustomTitles[data].empty()) 
						{
							title = Utils::String::VA("\xFF%s", CustomTitles[data].data());
							currentLookupResult->result = title;
							currentLookupResult->dunno = 2;

							__asm 
							{
								mov eax, title;
								pop ecx;
								mov ecx, currentLookupRequest;
								mov ecx, DWORD ptr[ecx + 0x04];
								retn;
							}
						}
					}
				}
				// If the title was changed it already returned at this point so...
				// Remove prefix and data to make being readable to the normal lookuprequest
				currentLookupRequest->tableRow = static_cast<std::int32_t>(*(reinterpret_cast<WORD*>(&currentLookupRequest->tableRow)));
			}
		}

		// Continue with the normal lookup request because we did not use our custom result
		__asm 
		{
			mov eax, [esi + 0x50];
			cmp eax, 0x3;

			push 0x62DCC7;
			retn;
		}
	}

	void CardTitles::ParseCustomTitles(const char* msg) 
	{
		const char* playerTitle;
		for (int i = 0; i < 18; i++) 
		{
			playerTitle = Utils::Hook::Call<const char*(const char*, const char*)>(0x47C820)(msg, std::to_string(i).c_str());

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
		Utils::Hook(0x63A2D5, LocalizationSkipHookStub).install()->quick();
		Utils::Hook(0x62DCC1, TableLookupByRowHookStub).install()->quick();

		Utils::Hook::Nop(0x62DCC6, 1);
	}

	CardTitles::~CardTitles()
	{

	}
}