#include <STDInclude.hpp>

#include "ClanTags.hpp"
#include "Events.hpp"
#include "PlayerName.hpp"
#include "ServerCommands.hpp"

namespace Components
{
	const Game::dvar_t* ClanTags::ClanName;

	// bgs_t and clientState_s do not have this
	char ClanTags::ClientState[Game::MAX_CLIENTS][MAX_CLAN_NAME_LENGTH];

	const char* ClanTags::GetClanTagWithName(int clientNum, const char* playerName)
	{
		AssertIn(clientNum, Game::MAX_CLIENTS);

		if (ClientState[clientNum][0] == '\0')
		{
			return playerName;
		}

		return Utils::String::VA("[%s]%s", ClientState[clientNum], playerName);
	}

	void ClanTags::SendClanTagsToClients()
	{
		std::string list;

		for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			list.append(std::format("\\{}\\{}", i, ClientState[i]));
		}

		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} clanNames \"{}\"", 22, list));
	}

	void ClanTags::ParseClanTags(const char* infoString)
	{
		for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			const auto index = std::to_string(i);
			const auto* clanTag = Game::Info_ValueForKey(infoString, index.data());

			if (clanTag[0] == '\0')
			{
				ClientState[i][0] = '\0';
			}
			else
			{
				Game::I_strncpyz(ClientState[i], clanTag, sizeof(ClientState[0]) / sizeof(char));
			}
		}
	}

	int ClanTags::CL_FilterChar(unsigned char input)
	{
		if (input == '^')
		{
			return ' ';
		}
		if (input < ' ')
		{
			return -1;
		}

		if (input == 188 || input == 189)
		{
			return -1;
		}

		return input;
	}

	void ClanTags::CL_SanitizeClanName()
	{
		char saneNameBuf[MAX_CLAN_NAME_LENGTH]{};
		auto* saneName = saneNameBuf;
		
		assert(ClanName);
		const auto* currentName = ClanName->current.string;
		if (currentName)
		{
			auto nameLen = std::strlen(currentName);
			for (std::size_t i = 0; (i < nameLen) && (i < sizeof(saneNameBuf)); ++i)
			{
				auto curChar = CL_FilterChar(static_cast<unsigned char>(currentName[i]));
				if (curChar > 0)
				{
					*saneName++ = curChar & 0xFF;
				}
			}

			saneNameBuf[sizeof(saneNameBuf) - 1] = '\0';
			Game::Dvar_SetString(ClanName, saneNameBuf);
		}
	}

	char* ClanTags::GamerProfile_GetClanName(int controllerIndex)
	{
		AssertIn(controllerIndex, Game::MAX_LOCAL_CLIENTS);
		assert(ClanName);

		CL_SanitizeClanName();
		Game::I_strncpyz(Game::gamerSettings[controllerIndex].exeConfig.clanPrefix, ClanName->current.string, sizeof(Game::GamerSettingExeConfig::clanPrefix));

		return Game::gamerSettings[controllerIndex].exeConfig.clanPrefix;
	}

	void ClanTags::Dvar_InfoString_Stub(char* s, const char* key, const char* value)
	{
		Utils::Hook::Call<void(char*, const char*, const char*)>(0x4AE560)(s, key, value); // Info_SetValueForKey

		// Set 'clanAbbrev' in the info string
		Utils::Hook::Call<void(char*, const char*, const char*)>(0x4AE560)(s, "clanAbbrev", GamerProfile_GetClanName(0)); // Info_SetValueForKey
	}

	void ClanTags::ClientUserinfoChanged(const char* s, int clientNum)
	{
		AssertIn(clientNum, Game::MAX_CLIENTS);

		const auto* clanAbbrev = Game::Info_ValueForKey(s, "clanAbbrev");

		if (clanAbbrev[0] == '\0')
		{
			ClientState[clientNum][0] = '\0';
		}
		else
		{
			Game::I_strncpyz(ClientState[clientNum], clanAbbrev, sizeof(ClientState[0]) / sizeof(char));
		}
	}

	void __declspec(naked) ClanTags::ClientUserinfoChanged_Stub()
	{
		__asm
		{
			pushad

			push [esp + 0x20 + 0x824] // clientNum
			push ecx // s
			call ClientUserinfoChanged
			add esp, 0x8

			popad

			push 0x445334 // Return address
			push 0x47C820 // Info_ValueForKey
			// Jump to Info_ValueForKey & add return address
			retn
		}
	}

	void __declspec(naked) ClanTags::DrawPlayerNameOnScoreboard()
	{
		__asm
		{
			push eax
			pushad

			push edi
			push [ebp]

			call GetClanTagWithName
			add esp, 0x8

			mov [esp + 0x20], eax

			popad
			pop edi

			push 0x591247 // Return address
			push 0x5909E0 // DrawListString
			retn
		}
	}

	// s1 is always an empty string
	int ClanTags::PartyClient_Frame_Stub(const char* s0, [[maybe_unused]] const char* s1)
	{
		return Utils::Hook::Call<int(const char*, const char*)>(0x4B0100)(s0, GamerProfile_GetClanName(0)); // I_strcmp
	}

	// clanAbbrev is always an empty string
	void ClanTags::Party_UpdateClanName_Stub(Game::PartyData* party, [[maybe_unused]] const char* clanAbbrev)
	{
		Utils::Hook::Call<void(Game::PartyData*, const char*)>(0x4B3B10)(party, GamerProfile_GetClanName(0)); // Party_UpdateClanName
	}

	void ClanTags::PlayerCards_SetCachedPlayerData(Game::PlayerCardData* data, const int clientNum)
	{
		Game::I_strncpyz(data->clanAbbrev, ClientState[clientNum], sizeof(Game::PlayerCardData::clanAbbrev));
	}

	void __declspec(naked) ClanTags::PlayerCards_SetCachedPlayerData_Stub()
	{
		using namespace Game;

		__asm
		{
			call I_strncpyz
			add esp, 0xC

			mov byte ptr [esi + 0x3C], 0x0

			// Copy the clanName
			push [esp + 0xC] // clientNum
			push esi // g_PlayerCardCache
			call PlayerCards_SetCachedPlayerData
			add esp, 0x8

			// Exit function
			pop esi
			ret
		}
	}

	Game::PlayerCardData* ClanTags::PlayerCards_GetLiveProfileDataForClient_Stub(const unsigned int clientIndex)
	{
		auto* result = Utils::Hook::Call<Game::PlayerCardData*(unsigned int)>(0x46C0F0)(clientIndex);
		Game::I_strncpyz(result->clanAbbrev, GamerProfile_GetClanName(static_cast<int>(clientIndex)), sizeof(Game::PlayerCardData::clanAbbrev));

		return result;
	}

	Game::PlayerCardData* ClanTags::PlayerCards_GetLiveProfileDataForController_Stub(const unsigned int controllerIndex)
	{
		auto* result = Utils::Hook::Call<Game::PlayerCardData*(unsigned int)>(0x463B90)(controllerIndex);
		AssertIn(controllerIndex, Game::MAX_LOCAL_CLIENTS);
		Game::I_strncpyz(result->clanAbbrev, GamerProfile_GetClanName(static_cast<int>(controllerIndex)), sizeof(Game::PlayerCardData::clanAbbrev));

		return result;
	}

	ClanTags::ClanTags()
	{
		Events::OnDvarInit([]
		{
			ClanName = Game::Dvar_RegisterString("clanName", "", Game::DVAR_ARCHIVE, "Your clan abbreviation");
		});

		std::memset(&ClientState, 0, sizeof(char[Game::MAX_CLIENTS][MAX_CLAN_NAME_LENGTH]));

		ServerCommands::OnCommand(22, [](const Command::Params* params)
		{
			if (std::strcmp(params->get(1), "clanNames") == 0)
			{
				if (params->size() == 3)
				{
					ParseClanTags(params->get(2));
					return true;
				}
			}

			return false;
		});

		Utils::Hook(0x430B00, Dvar_InfoString_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x44532F, ClientUserinfoChanged_Stub, HOOK_JUMP).install()->quick();

		// clanName before playerName
		Utils::Hook(0x591242, DrawPlayerNameOnScoreboard, HOOK_JUMP).install()->quick();

		Utils::Hook(0x49765B, PartyClient_Frame_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x49767E, Party_UpdateClanName_Stub, HOOK_CALL).install()->quick();

		// clanName in the PlayerCard (GetPlayerCardClientData)
		Utils::Hook(0x458DF4, PlayerCards_SetCachedPlayerData_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x62EAB6, PlayerCards_GetLiveProfileDataForClient_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x62EAC3, PlayerCards_GetLiveProfileDataForController_Stub, HOOK_CALL).install()->quick();

		// clanName in CG_Obituary
		Utils::Hook(0x586DD6, PlayerName::GetClientName, HOOK_CALL).install()->quick();
		Utils::Hook(0x586E2A, PlayerName::GetClientName, HOOK_CALL).install()->quick();
	}
}
