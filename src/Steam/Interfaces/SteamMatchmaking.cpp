#include <STDInclude.hpp>
#include <Components/Modules/Party.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	int Matchmaking::GetFavoriteGameCount()
	{
		return 0;
	}

	bool Matchmaking::GetFavoriteGame(int iGame, unsigned int *pnAppID, unsigned int *pnIP, unsigned short *pnConnPort, unsigned short *pnQueryPort, unsigned int *punFlags, unsigned int *pRTime32LastPlayedOnServer)
	{
		return false;
	}

	int Matchmaking::AddFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort, unsigned short nQueryPort, unsigned int unFlags, unsigned int rTime32LastPlayedOnServer)
	{
		return 0;
	}

	bool Matchmaking::RemoveFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort, unsigned short nQueryPort, unsigned int unFlags)
	{
		return false;
	}

	unsigned __int64 Matchmaking::RequestLobbyList()
	{
		return 0;
	}

	void Matchmaking::AddRequestLobbyListStringFilter(const char *pchKeyToMatch, const char *pchValueToMatch, int eComparisonType)
	{
	}

	void Matchmaking::AddRequestLobbyListNumericalFilter(const char *pchKeyToMatch, int nValueToMatch, int eComparisonType)
	{
	}

	void Matchmaking::AddRequestLobbyListNearValueFilter(const char *pchKeyToMatch, int nValueToBeCloseTo)
	{
	}

	void Matchmaking::AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable)
	{
	}

	SteamID Matchmaking::GetLobbyByIndex(int iLobby)
	{
		return SteamID();
	}

	unsigned __int64 Matchmaking::CreateLobby(int eLobbyType, int cMaxMembers)
	{
		uint64_t result = Callbacks::RegisterCall();
		LobbyCreated* retvals = ::Utils::Memory::Allocate<LobbyCreated>();
		SteamID id;

		id.accountID = 1337132;
		id.universe = 1;
		id.accountType = 8;
		id.accountInstance = 0x40000;

		retvals->m_eResult = 1;
		retvals->m_ulSteamIDLobby = id;

		Callbacks::ReturnCall(retvals, sizeof(LobbyCreated), LobbyCreated::CallbackID, result);

		Matchmaking::JoinLobby(id);

		return result;
	}

	unsigned __int64 Matchmaking::JoinLobby(SteamID steamIDLobby)
	{
		uint64_t result = Callbacks::RegisterCall();
		LobbyEnter* retvals = ::Utils::Memory::Allocate<LobbyEnter>();
		retvals->m_bLocked = false;
		retvals->m_EChatRoomEnterResponse = 1;
		retvals->m_rgfChatPermissions = 0xFFFFFFFF;
		retvals->m_ulSteamIDLobby = steamIDLobby;

		Callbacks::ReturnCall(retvals, sizeof(LobbyEnter), LobbyEnter::CallbackID, result);

		return result;
	}

	void Matchmaking::LeaveLobby(SteamID steamIDLobby)
	{
		Components::Party::RemoveLobby(steamIDLobby);
	}

	bool Matchmaking::InviteUserToLobby(SteamID steamIDLobby, SteamID steamIDInvitee)
	{
		return true;
	}

	int Matchmaking::GetNumLobbyMembers(SteamID steamIDLobby)
	{
		return 1;
	}

	SteamID Matchmaking::GetLobbyMemberByIndex(SteamID steamIDLobby, int iMember)
	{
		return SteamUser()->GetSteamID();
	}

	const char *Matchmaking::GetLobbyData(SteamID steamIDLobby, const char *pchKey)
	{
		return Components::Party::GetLobbyInfo(steamIDLobby, pchKey);
	}

	bool Matchmaking::SetLobbyData(SteamID steamIDLobby, const char *pchKey, const char *pchValue)
	{
		return true;
	}

	int Matchmaking::GetLobbyDataCount(SteamID steamIDLobby)
	{
		return 0;
	}

	bool Matchmaking::GetLobbyDataByIndex(SteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize)
	{
		return false;
	}

	bool Matchmaking::DeleteLobbyData(SteamID steamIDLobby, const char *pchKey)
	{
		return false;
	}

	const char *Matchmaking::GetLobbyMemberData(SteamID steamIDLobby, SteamID steamIDUser, const char *pchKey)
	{
		return "";
	}

	void Matchmaking::SetLobbyMemberData(SteamID steamIDLobby, const char *pchKey, const char *pchValue)
	{
	}

	bool Matchmaking::SendLobbyChatMsg(SteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody)
	{
		return true;
	}

	int Matchmaking::GetLobbyChatEntry(SteamID steamIDLobby, int iChatID, SteamID *pSteamIDUser, void *pvData, int cubData, int *peChatEntryType)
	{
		return 0;
	}

	bool Matchmaking::RequestLobbyData(SteamID steamIDLobby)
	{
		return false;
	}

	void Matchmaking::SetLobbyGameServer(SteamID steamIDLobby, unsigned int unGameServerIP, unsigned short unGameServerPort, SteamID steamIDGameServer)
	{
	}

	bool Matchmaking::GetLobbyGameServer(SteamID steamIDLobby, unsigned int *punGameServerIP, unsigned short *punGameServerPort, SteamID *psteamIDGameServer)
	{
		return false;
	}

	bool Matchmaking::SetLobbyMemberLimit(SteamID steamIDLobby, int cMaxMembers)
	{
		return true;
	}

	int Matchmaking::GetLobbyMemberLimit(SteamID steamIDLobby)
	{
		return 0;
	}

	bool Matchmaking::SetLobbyType(SteamID steamIDLobby, int eLobbyType)
	{
		return true;
	}

	bool Matchmaking::SetLobbyJoinable(SteamID steamIDLobby, bool bLobbyJoinable)
	{
		return true;
	}

	SteamID Matchmaking::GetLobbyOwner(SteamID steamIDLobby)
	{
		return SteamUser()->GetSteamID();
	}

	bool Matchmaking::SetLobbyOwner(SteamID steamIDLobby, SteamID steamIDNewOwner)
	{
		return true;
	}
}

STEAM_IGNORE_WARNINGS_END
