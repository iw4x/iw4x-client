#pragma once

namespace Components
{
	class ClanTags : public Component
	{
	public:
		static constexpr std::size_t MAX_CLAN_NAME_LENGTH = 5;

		ClanTags();

		static const char* GetClanTagWithName(int clientNum, const char* playerName);

		static void SendClanTagsToClients();

		static void CL_SanitizeClanName();

	private:
		static const Game::dvar_t* ClanName;

		static const char* dvarNameList[];

		static char ClientState[Game::MAX_CLIENTS][5];

		static void ParseClanTags(const char* infoString);

		static int CL_FilterChar(unsigned char input);

		static char* GamerProfile_GetClanName(int controllerIndex);

		static void Dvar_InfoString_Stub(char* s, const char* key, const char* value);

		static void ClientUserinfoChanged(const char* s, int clientNum);
		static void ClientUserinfoChanged_Stub();

		static void DrawPlayerNameOnScoreboard();

		static int PartyClient_Frame_Stub(const char* s0, const char* s1);
		static void Party_UpdateClanName_Stub(Game::PartyData* party, const char* clanAbbrev);

		static void PlayerCards_SetCachedPlayerData(Game::PlayerCardData* data, int clientNum);
		static void PlayerCards_SetCachedPlayerData_Stub();

		static Game::PlayerCardData* PlayerCards_GetLiveProfileDataForClient_Stub(unsigned int clientIndex);
		static Game::PlayerCardData* PlayerCards_GetLiveProfileDataForController_Stub(unsigned int controllerIndex);
	};
}
