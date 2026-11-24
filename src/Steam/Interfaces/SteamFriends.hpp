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

  // Note that the game calls the methods of an interface by their slots,
  // and x64 is built against a newer version of this one than x86. So we
  // lay it out the way Steamworks SDK 1.20 does SteamFriends013, which is
  // the earliest version whose slots match the ones that x64 calls. Why
  // the earliest? The game gets its interfaces through steam_api's plain
  // accessors (SteamFriends() and so on) and never names a version, so the
  // slots it calls are all we have to go by. Any version that matches them
  // would do, and the oldest one seems the least likely to add anything
  // we don't need.
  //
  class Friends
  {
  public:
    virtual const char* GetPersonaName();
    virtual uint64_t SetPersonaName(const char *pchPersonaName);
    virtual int GetPersonaState();
    virtual int GetFriendCount(int iFriendFlags);
    virtual SteamID* GetFriendByIndex(SteamID* result, int iFriend, int iFriendFlags);
    virtual int GetFriendRelationship(SteamID steamIDFriend);
    virtual int GetFriendPersonaState(SteamID steamIDFriend);
    virtual const char* GetFriendPersonaName(SteamID steamIDFriend);
    virtual bool GetFriendGamePlayed(SteamID steamIDFriend, FriendGameInfo *pFriendGameInfo);
    virtual const char* GetFriendPersonaNameHistory(SteamID steamIDFriend, int iPersonaName);
    virtual bool HasFriend(SteamID steamIDFriend, int iFriendFlags);
    virtual int GetClanCount();
    virtual SteamID* GetClanByIndex(SteamID* result, int iClan);
    virtual const char* GetClanName(SteamID steamIDClan);
    virtual const char* GetClanTag(SteamID steamIDClan);
    virtual bool GetClanActivityCounts(SteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting);
    virtual uint64_t DownloadClanActivityCounts(SteamID *psteamIDClans, int cClansToRequest);
    virtual int GetFriendCountFromSource(SteamID steamIDSource);
    virtual SteamID* GetFriendFromSourceByIndex(SteamID* result, SteamID steamIDSource, int iFriend);
    virtual bool IsUserInSource(SteamID steamIDUser, SteamID steamIDSource);
    virtual void SetInGameVoiceSpeaking(SteamID steamIDUser, bool bSpeaking);
    virtual void ActivateGameOverlay(const char *pchDialog);
    virtual void ActivateGameOverlayToUser(const char *pchDialog, SteamID steamID);
    virtual void ActivateGameOverlayToWebPage(const char *pchURL);
    virtual void ActivateGameOverlayToStore(unsigned int nAppID, int eFlag);
    virtual void SetPlayedWith(SteamID steamIDUserPlayedWith);
    virtual void ActivateGameOverlayInviteDialog(SteamID steamIDLobby);
    virtual int GetSmallFriendAvatar(SteamID steamIDFriend);
    virtual int GetMediumFriendAvatar(SteamID steamIDFriend);
    virtual int GetLargeFriendAvatar(SteamID steamIDFriend);
    virtual bool RequestUserInformation(SteamID steamIDUser, bool bRequireNameOnly);
    virtual uint64_t RequestClanOfficerList(SteamID steamIDClan);
    virtual SteamID* GetClanOwner(SteamID* result, SteamID steamIDClan);
    virtual int GetClanOfficerCount(SteamID steamIDClan);
    virtual SteamID* GetClanOfficerByIndex(SteamID* result, SteamID steamIDClan, int iOfficer);
    virtual unsigned int GetUserRestrictions();
    virtual bool SetRichPresence(const char *pchKey, const char *pchValue);
    virtual void ClearRichPresence();
    virtual const char* GetFriendRichPresence(SteamID steamIDFriend, const char *pchKey);
    virtual int GetFriendRichPresenceKeyCount(SteamID steamIDFriend);
    virtual const char* GetFriendRichPresenceKeyByIndex(SteamID steamIDFriend, int iKey);
    virtual void RequestFriendRichPresence(SteamID steamIDFriend);
    virtual bool InviteUserToGame(SteamID steamIDFriend, const char *pchConnectString);
    virtual int GetCoplayFriendCount();
    virtual SteamID* GetCoplayFriend(SteamID* result, int iCoplayFriend);
    virtual int GetFriendCoplayTime(SteamID steamIDFriend);
    virtual unsigned int GetFriendCoplayGame(SteamID steamIDFriend);
    virtual uint64_t JoinClanChatRoom(SteamID steamIDClan);
    virtual bool LeaveClanChatRoom(SteamID steamIDClan);
    virtual int GetClanChatMemberCount(SteamID steamIDClan);
    virtual SteamID* GetChatMemberByIndex(SteamID* result, SteamID steamIDClan, int iUser);
    virtual bool SendClanChatMessage(SteamID steamIDClanChat, const char *pchText);
    virtual int GetClanChatMessage(SteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, int *peChatEntryType, SteamID *pSteamIDChatter);
    virtual bool IsClanChatAdmin(SteamID steamIDClanChat, SteamID steamIDUser);
    virtual bool IsClanChatWindowOpenInSteam(SteamID steamIDClanChat);
    virtual bool OpenClanChatWindowInSteam(SteamID steamIDClanChat);
    virtual bool CloseClanChatWindowInSteam(SteamID steamIDClanChat);
    virtual bool SetListenForFriendsMessages(bool bInterceptEnabled);
    virtual bool ReplyToFriendMessage(SteamID steamIDFriend, const char *pchMsgToSend);
    virtual int GetFriendMessage(SteamID steamIDFriend, int iMessageID, void *pvData, int cubData, int *peChatEntryType);
    virtual uint64_t GetFollowerCount(SteamID steamID);
    virtual uint64_t IsFollowing(SteamID steamID);
    virtual uint64_t EnumerateFollowingList(unsigned int unStartIndex);
  };

  class Friends15
  {
  public:
    virtual const char* GetPersonaName() = 0;
    virtual uint64_t SetPersonaName(const char *pchPersonaName) = 0;
    virtual int GetPersonaState() = 0;
    virtual int GetFriendCount(int iFriendFlags) = 0;
    virtual SteamID* GetFriendByIndex(SteamID* result, int iFriend, int iFriendFlags) = 0;
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
    virtual SteamID* GetClanByIndex(SteamID* result, int iClan) = 0;
    virtual const char* GetClanName(SteamID steamIDClan) = 0;
    virtual const char* GetClanTag(SteamID steamIDClan) = 0;
    virtual bool GetClanActivityCounts(SteamID steamID, int *pnOnline, int *pnInGame, int *pnChatting) = 0;
    virtual uint64_t DownloadClanActivityCounts(SteamID groupIDs[], int nIds) = 0;
    virtual int GetFriendCountFromSource(SteamID steamIDSource) = 0;
    virtual SteamID* GetFriendFromSourceByIndex(SteamID* result, SteamID steamIDSource, int iFriend) = 0;
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
    virtual SteamID* GetClanOwner(SteamID* result, SteamID steamIDClan) = 0;
    virtual int GetClanOfficerCount(SteamID steamIDClan) = 0;
    virtual SteamID* GetClanOfficerByIndex(SteamID* result, SteamID steamIDClan, int iOfficer) = 0;
    virtual int GetUserRestrictions() = 0;
    virtual bool SetRichPresence(const char *pchKey, const char *pchValue) = 0;
    virtual void ClearRichPresence() = 0;
    virtual const char* GetFriendRichPresence(SteamID steamIDFriend, const char *pchKey) = 0;
    virtual int GetFriendRichPresenceKeyCount(SteamID steamIDFriend) = 0;
    virtual const char* GetFriendRichPresenceKeyByIndex(SteamID steamIDFriend, int iKey) = 0;
    virtual void RequestFriendRichPresence(SteamID steamIDFriend) = 0;
    virtual bool InviteUserToGame(SteamID steamIDFriend, const char *pchConnectString) = 0;
    virtual int GetCoplayFriendCount() = 0;
    virtual SteamID* GetCoplayFriend(SteamID* result, int iCoplayFriend) = 0;
    virtual int GetFriendCoplayTime(SteamID steamIDFriend) = 0;
    virtual uint32_t GetFriendCoplayGame(SteamID steamIDFriend) = 0;
    virtual uint64_t JoinClanChatRoom(SteamID steamIDClan) = 0;
    virtual bool LeaveClanChatRoom(SteamID steamIDClan) = 0;
    virtual int GetClanChatMemberCount(SteamID steamIDClan) = 0;
    virtual SteamID* GetChatMemberByIndex(SteamID* result, SteamID steamIDClan, int iUser) = 0;
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

    SteamID GetFriendByIndex(int iFriend, int iFriendFlags)
    {
      SteamID result;
      GetFriendByIndex(&result, iFriend, iFriendFlags);
      return result;
    }
    virtual uint64_t EnumerateFollowingList(uint32_t unStartIndex) = 0;
  };
}
