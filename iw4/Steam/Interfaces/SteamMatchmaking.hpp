namespace Steam
{
	struct LobbyCreated
	{
		enum { CallbackID = 513 };

		int m_eResult;
		int m_pad;
		SteamID m_ulSteamIDLobby;
	};

	struct LobbyEnter
	{
		enum { CallbackID = 504 };

		SteamID m_ulSteamIDLobby;
		int m_rgfChatPermissions;
		bool m_bLocked;
		int m_EChatRoomEnterResponse;
	};

	class Matchmaking
	{
	public:
		virtual int GetFavoriteGameCount();
		virtual bool GetFavoriteGame(int iGame, unsigned int *pnAppID, unsigned int *pnIP, unsigned short *pnConnPort, unsigned short *pnQueryPort, unsigned int *punFlags, unsigned int *pRTime32LastPlayedOnServer);
		virtual int AddFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort, unsigned short nQueryPort, unsigned int unFlags, unsigned int rTime32LastPlayedOnServer);
		virtual bool RemoveFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort, unsigned short nQueryPort, unsigned int unFlags);
		virtual unsigned __int64 RequestLobbyList();
		virtual void AddRequestLobbyListStringFilter(const char *pchKeyToMatch, const char *pchValueToMatch, int eComparisonType);
		virtual void AddRequestLobbyListNumericalFilter(const char *pchKeyToMatch, int nValueToMatch, int eComparisonType);
		virtual void AddRequestLobbyListNearValueFilter(const char *pchKeyToMatch, int nValueToBeCloseTo);
		virtual void AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable);
		virtual SteamID GetLobbyByIndex(int iLobby);
		virtual unsigned __int64 CreateLobby(int eLobbyType, int cMaxMembers);
		virtual unsigned __int64 JoinLobby(SteamID steamIDLobby);
		virtual void LeaveLobby(SteamID steamIDLobby);
		virtual bool InviteUserToLobby(SteamID steamIDLobby, SteamID steamIDInvitee);
		virtual int GetNumLobbyMembers(SteamID steamIDLobby);
		virtual SteamID GetLobbyMemberByIndex(SteamID steamIDLobby, int iMember);
		virtual const char *GetLobbyData(SteamID steamIDLobby, const char *pchKey);
		virtual bool SetLobbyData(SteamID steamIDLobby, const char *pchKey, const char *pchValue);
		virtual int GetLobbyDataCount(SteamID steamIDLobby);
		virtual bool GetLobbyDataByIndex(SteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize);
		virtual bool DeleteLobbyData(SteamID steamIDLobby, const char *pchKey);
		virtual const char *GetLobbyMemberData(SteamID steamIDLobby, SteamID steamIDUser, const char *pchKey);
		virtual void SetLobbyMemberData(SteamID steamIDLobby, const char *pchKey, const char *pchValue);
		virtual bool SendLobbyChatMsg(SteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody);
		virtual int GetLobbyChatEntry(SteamID steamIDLobby, int iChatID, SteamID *pSteamIDUser, void *pvData, int cubData, int *peChatEntryType);
		virtual bool RequestLobbyData(SteamID steamIDLobby);
		virtual void SetLobbyGameServer(SteamID steamIDLobby, unsigned int unGameServerIP, unsigned short unGameServerPort, SteamID steamIDGameServer);
		virtual bool GetLobbyGameServer(SteamID steamIDLobby, unsigned int *punGameServerIP, unsigned short *punGameServerPort, SteamID *psteamIDGameServer);
		virtual bool SetLobbyMemberLimit(SteamID steamIDLobby, int cMaxMembers);
		virtual int GetLobbyMemberLimit(SteamID steamIDLobby);
		virtual bool SetLobbyType(SteamID steamIDLobby, int eLobbyType);
		virtual bool SetLobbyJoinable(SteamID steamIDLobby, bool bLobbyJoinable);
		virtual SteamID GetLobbyOwner(SteamID steamIDLobby);
		virtual bool SetLobbyOwner(SteamID steamIDLobby, SteamID steamIDNewOwner);
	};
}
