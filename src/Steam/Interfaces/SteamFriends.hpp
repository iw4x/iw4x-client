#pragma once

namespace Steam
{
	struct FriendSessionStateInfo
	{
		uint32_t m_uiOnlineSessionInstances;
		uint8_t m_uiPublishedToFriendsSessionInstance;
	};

	struct FriendGameInfo
	{
		GameID_t m_gameID;
		uint32_t m_unGameIP;
		uint16_t m_usGamePort;
		uint16_t m_usQueryPort;
		SteamID m_steamIDLobby;
	};

	class Friends
	{
	public:
		virtual const char* GetPersonaName();
		virtual void SetPersonaName(const char *pchPersonaName);
		virtual int GetPersonaState();
		virtual int GetFriendCount(int eFriendFlags);
		virtual SteamID GetFriendByIndex(int iFriend, int iFriendFlags);
		virtual int GetFriendRelationship(SteamID steamIDFriend);
		virtual int GetFriendPersonaState(SteamID steamIDFriend);
		virtual const char* GetFriendPersonaName(SteamID steamIDFriend);
		virtual int GetFriendAvatar(SteamID steamIDFriend, int eAvatarSize);
		virtual bool GetFriendGamePlayed(SteamID steamIDFriend, FriendGameInfo *pFriendGameInfo);
		virtual const char* GetFriendPersonaNameHistory(SteamID steamIDFriend, int iPersonaName);
		virtual bool HasFriend(SteamID steamIDFriend, int eFriendFlags);
		virtual int GetClanCount();
		virtual SteamID GetClanByIndex(int iClan);
		virtual const char* GetClanName(SteamID steamIDClan);
		virtual int GetFriendCountFromSource(SteamID steamIDSource);
		virtual SteamID GetFriendFromSourceByIndex(SteamID steamIDSource, int iFriend);
		virtual bool IsUserInSource(SteamID steamIDUser, SteamID steamIDSource);
		virtual void SetInGameVoiceSpeaking(SteamID steamIDUser, bool bSpeaking);
		virtual void ActivateGameOverlay(const char *pchDialog);
		virtual void ActivateGameOverlayToUser(const char *pchDialog, SteamID steamID);
		virtual void ActivateGameOverlayToWebPage(const char *pchURL);
		virtual void ActivateGameOverlayToStore(unsigned int nAppID);
		virtual void SetPlayedWith(SteamID steamIDUserPlayedWith);
	};

	class Friends15
	{
	public:
		virtual const char* GetPersonaName() = 0;
		virtual uint64_t SetPersonaName(const char *pchPersonaName) = 0;
		virtual int GetPersonaState() = 0;
		virtual int GetFriendCount(int iFriendFlags) = 0;
		virtual SteamID GetFriendByIndex(int iFriend, int iFriendFlags) = 0;
		virtual int GetFriendRelationship(SteamID steamIDFriend) = 0;
		virtual int GetFriendPersonaState(SteamID steamIDFriend) = 0;
		virtual const char* GetFriendPersonaName(SteamID steamIDFriend) = 0;
		virtual bool GetFriendGamePlayed(SteamID steamID, void *pGamePlayInfo) = 0;
		virtual const char* GetFriendPersonaNameHistory(SteamID steamIDFriend, FriendGameInfo iPersonaName) = 0;
		virtual int GetFriendSteamLevel(SteamID steamIDFriend) = 0;
		virtual const char* GetPlayerNickname(SteamID steamIDPlayer) = 0;
		virtual int16_t GetFriendsGroupCount() = 0;
		virtual int16_t GetFriendsGroupIDByIndex(int32_t) = 0;
		virtual const char* GetFriendsGroupName(int16_t) = 0;
		virtual int GetFriendsGroupMembersCount(int16_t) = 0;
		virtual int GetFriendsGroupMembersList(int16_t, SteamID *, int32_t) = 0;
		virtual bool HasFriend(SteamID steamIDFriend, int iFriendFlags) = 0;
		virtual int GetClanCount() = 0;
		virtual SteamID GetClanByIndex(int iClan) = 0;
		virtual const char* GetClanName(SteamID steamIDClan) = 0;
		virtual const char* GetClanTag(SteamID steamIDClan) = 0;
		virtual bool GetClanActivityCounts(SteamID steamID, int *pnOnline, int *pnInGame, int *pnChatting) = 0;
		virtual uint64_t DownloadClanActivityCounts(SteamID groupIDs[], int nIds) = 0;
		virtual int GetFriendCountFromSource(SteamID steamIDSource) = 0;
		virtual SteamID GetFriendFromSourceByIndex(SteamID steamIDSource, int iFriend) = 0;
		virtual bool IsUserInSource(SteamID steamIDUser, SteamID steamIDSource) = 0;
		virtual void SetInGameVoiceSpeaking(SteamID steamIDUser, bool bSpeaking) = 0;
		virtual void ActivateGameOverlay(const char *pchDialog) = 0;
		virtual void ActivateGameOverlayToUser(const char *pchDialog, SteamID steamID) = 0;
		virtual void ActivateGameOverlayToWebPage(const char *pchURL) = 0;
		virtual void ActivateGameOverlayToStore(uint32_t nAppID, int eFlag) = 0;
		virtual void SetPlayedWith(SteamID steamIDUserPlayedWith) = 0;
		virtual void ActivateGameOverlayInviteDialog(SteamID steamIDLobby) = 0;
		virtual int GetSmallFriendAvatar(SteamID steamIDFriend) = 0;
		virtual int GetMediumFriendAvatar(SteamID steamIDFriend) = 0;
		virtual int GetLargeFriendAvatar(SteamID steamIDFriend) = 0;
		virtual bool RequestUserInformation(SteamID steamIDUser, bool bRequireNameOnly) = 0;
		virtual uint64_t RequestClanOfficerList(SteamID steamIDClan) = 0;
		virtual SteamID GetClanOwner(SteamID steamIDClan) = 0;
		virtual int GetClanOfficerCount(SteamID steamIDClan) = 0;
		virtual SteamID GetClanOfficerByIndex(SteamID steamIDClan, int iOfficer) = 0;
		virtual int GetUserRestrictions() = 0;
		virtual bool SetRichPresence(const char *pchKey, const char *pchValue) = 0;
		virtual void ClearRichPresence() = 0;
		virtual const char* GetFriendRichPresence(SteamID steamIDFriend, const char *pchKey) = 0;
		virtual int GetFriendRichPresenceKeyCount(SteamID steamIDFriend) = 0;
		virtual const char* GetFriendRichPresenceKeyByIndex(SteamID steamIDFriend, int iKey) = 0;
		virtual void RequestFriendRichPresence(SteamID steamIDFriend) = 0;
		virtual bool InviteUserToGame(SteamID steamIDFriend, const char *pchConnectString) = 0;
		virtual int GetCoplayFriendCount() = 0;
		virtual SteamID GetCoplayFriend(int iCoplayFriend) = 0;
		virtual int GetFriendCoplayTime(SteamID steamIDFriend) = 0;
		virtual uint32_t GetFriendCoplayGame(SteamID steamIDFriend) = 0;
		virtual uint64_t JoinClanChatRoom(SteamID steamIDClan) = 0;
		virtual bool LeaveClanChatRoom(SteamID steamIDClan) = 0;
		virtual int GetClanChatMemberCount(SteamID steamIDClan) = 0;
		virtual SteamID GetChatMemberByIndex(SteamID steamIDClan, int iUser) = 0;
		virtual bool SendClanChatMessage(SteamID steamIDClanChat, const char *pchText) = 0;
		virtual int GetClanChatMessage(SteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, int *peChatEntryType, SteamID *pSteamIDChatter) = 0;
		virtual bool IsClanChatAdmin(SteamID steamIDClanChat, SteamID steamIDUser) = 0;
		virtual bool IsClanChatWindowOpenInSteam(SteamID steamIDClanChat) = 0;
		virtual bool OpenClanChatWindowInSteam(SteamID steamIDClanChat) = 0;
		virtual bool CloseClanChatWindowInSteam(SteamID steamIDClanChat) = 0;
		virtual bool SetListenForFriendsMessages(bool bInterceptEnabled) = 0;
		virtual bool ReplyToFriendMessage(SteamID steamIDFriend, const char *pchMsgToSend) = 0;
		virtual int GetFriendMessage(SteamID steamIDFriend, int iMessageID, void *pvData, int cubData, int *peChatEntryType) = 0;
		virtual uint64_t GetFollowerCount(SteamID steamID) = 0;
		virtual uint64_t IsFollowing(SteamID steamID) = 0;
		virtual uint64_t EnumerateFollowingList(uint32_t unStartIndex) = 0;
	};
}
