#pragma once

namespace Steam
{
	struct FriendSessionStateInfo_t
	{
		uint32_t m_uiOnlineSessionInstances;
		uint8_t m_uiPublishedToFriendsSessionInstance;
	};

	class Friends
	{
	public:
		virtual const char *GetPersonaName();
		virtual void SetPersonaName(const char *pchPersonaName);
		virtual int GetPersonaState();
		virtual int GetFriendCount(int eFriendFlags);
		virtual SteamID GetFriendByIndex(int iFriend, int iFriendFlags);
		virtual int GetFriendRelationship(SteamID steamIDFriend);
		virtual int GetFriendPersonaState(SteamID steamIDFriend);
		virtual const char *GetFriendPersonaName(SteamID steamIDFriend);
		virtual int GetFriendAvatar(SteamID steamIDFriend, int eAvatarSize);
		virtual bool GetFriendGamePlayed(SteamID steamIDFriend, void *pFriendGameInfo);
		virtual const char *GetFriendPersonaNameHistory(SteamID steamIDFriend, int iPersonaName);
		virtual bool HasFriend(SteamID steamIDFriend, int eFriendFlags);
		virtual int GetClanCount();
		virtual SteamID GetClanByIndex(int iClan);
		virtual const char *GetClanName(SteamID steamIDClan);
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
		virtual const char *GetPersonaName() = 0;
		virtual uint64_t SetPersonaName(const char *pchPersonaName) = 0;
		virtual int GetPersonaState() = 0;
		virtual int GetFriendCount(int iFriendFlags) = 0;
		virtual SteamID GetFriendByIndex(int iFriend, int iFriendFlags) = 0;
		virtual int GetFriendRelationship(SteamID steamIDFriend) = 0;
		virtual int GetFriendPersonaState(SteamID steamIDFriend) = 0;
		virtual const char *GetFriendPersonaName(SteamID steamIDFriend) = 0;
		virtual bool GetFriendGamePlayed(SteamID steamID, void *pGamePlayInfo) = 0;
		virtual const char *GetFriendPersonaNameHistory(SteamID steamIDFriend, int iPersonaName) = 0;
		virtual int GetFriendSteamLevel(SteamID steamIDFriend) = 0;
		virtual const char *GetPlayerNickname(SteamID steamIDPlayer) = 0;
		virtual int16_t GetFriendsGroupCount() = 0;
		virtual int16_t GetFriendsGroupIDByIndex(int32_t) = 0;
		virtual const char * GetFriendsGroupName(int16_t) = 0;
		virtual int GetFriendsGroupMembersCount(int16_t) = 0;
		virtual int GetFriendsGroupMembersList(int16_t, SteamID *, int32_t) = 0;
		virtual bool HasFriend(SteamID steamIDFriend, int iFriendFlags) = 0;
		virtual int GetClanCount() = 0;
		virtual SteamID GetClanByIndex(int iClan) = 0;
		virtual const char *GetClanName(SteamID steamIDClan) = 0;
		virtual const char *GetClanTag(SteamID steamIDClan) = 0;
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
		virtual const char *GetFriendRichPresence(SteamID steamIDFriend, const char *pchKey) = 0;
		virtual int GetFriendRichPresenceKeyCount(SteamID steamIDFriend) = 0;
		virtual const char *GetFriendRichPresenceKeyByIndex(SteamID steamIDFriend, int iKey) = 0;
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

	class Friends2
	{
	public:
		virtual const char *GetPersonaName() = 0;
		virtual void SetPersonaName(const char *pchPersonaName) = 0;
		virtual int GetPersonaState() = 0;
		virtual void SetPersonaState(int ePersonaState) = 0;
		// [...]
	};

	class IClientFriends
	{
	public:

		// returns the local players name - guaranteed to not be NULL.
		virtual const char *GetPersonaName() = 0;

		// sets the player name, stores it on the server and publishes the changes to all friends who are online
		virtual void SetPersonaName(const char *pchPersonaName) = 0;
		virtual uint64_t SetPersonaNameEx(const char *pchPersonaName, bool bSendCallback) = 0;

		virtual bool IsPersonaNameSet() = 0;

		// gets the friend status of the current user
		virtual int GetPersonaState() = 0;
		// sets the status, communicates to server, tells all friends
		virtual void SetPersonaState(int ePersonaState) = 0;

		virtual bool NotifyUIOfMenuChange(bool bShowAvatars, bool bSortByName, bool bShowOnlineOnly, bool bShowUntaggedFriends) = 0;

		// friend iteration
		virtual int32_t GetFriendCount(int iFriendFlags) = 0;
		virtual uint32_t GetFriendArray(SteamID *, int8_t *, int32_t, int32_t) = 0;
		virtual uint32_t GetFriendArrayInGame(SteamID *, int8_t *, int32_t) = 0;
		virtual SteamID GetFriendByIndex(int32_t iFriend, int iFriendFlags) = 0;

		virtual int32_t GetOnlineFriendCount() = 0;

		// gets the relationship to a user
		virtual int GetFriendRelationship(SteamID steamIDFriend) = 0;
		virtual int GetFriendPersonaState(SteamID steamIDFriend) = 0;
		// returns the name of a friend - guaranteed to not be NULL.
		virtual const char *GetFriendPersonaName(SteamID steamIDFriend) = 0;

		// gets the avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
		virtual int32_t GetSmallFriendAvatar(SteamID steamIDFriend) = 0;
		virtual int32_t GetMediumFriendAvatar(SteamID steamIDFriend) = 0;
		virtual int32_t GetLargeFriendAvatar(SteamID steamIDFriend) = 0;
		virtual int32_t BGetFriendAvatarURL(void *, size_t, int, int, int) = 0;

		// steam registry, accessed by friend
		virtual void SetFriendRegValue(SteamID steamIDFriend, const char *pchKey, const char *pchValue) = 0;
		virtual const char *GetFriendRegValue(SteamID steamIDFriend, const char *pchKey) = 0;

		virtual bool DeleteFriendRegValue(SteamID steamID, const char *pchKey) = 0;

		virtual bool GetFriendGamePlayed(SteamID steamID, void *pGamePlayInfo) = 0;
		virtual const char *GetFriendGamePlayedExtraInfo(SteamID steamIDFriend) = 0;

		virtual SteamID GetFriendGameServer(SteamID steamIDFriend) = 0;

		virtual int GetFriendPersonaStateFlags(SteamID steamIDFriend) = 0;
		virtual bool IsFriendGameOnConsole(SteamID steamIDFriend) = 0;
		virtual FriendSessionStateInfo_t GetFriendSessionStateInfo(SteamID steamIDFriend) = 0;
		virtual int GetFriendRestrictions(SteamID steamIDFriend) = 0;

		// accesses old friends names - returns an empty string when their are no more items in the history
		virtual const char *GetFriendPersonaNameHistory(SteamID steamIDFriend, int32_t iPersonaName) = 0;

		//virtual uint32_t GetFriendSteamLevel(SteamID steamIDFriend) = 0;

		virtual uint64_t RequestPersonaNameHistory(SteamID steamIDFriend) = 0;
		virtual const char * GetFriendPersonaNameHistoryAndDate(SteamID steamIDFriend, int32_t iPersonaName, int * puTime) = 0;

		virtual bool AddFriend(SteamID steamID) = 0;
		virtual bool RemoveFriend(SteamID steamID) = 0;
		virtual bool HasFriend(SteamID steamID, int iFriendFlags) = 0;

		// adds a friend by email address or account name - value returned in callback
		virtual int AddFriendByName(const char *pchEmailOrAccountName) = 0;

		virtual bool InviteFriendByEmail(const char *pchEmailAddress) = 0;

		virtual bool RequestUserInformation(SteamID steamIDUser, bool bRequireNameOnly) = 0;

		virtual bool SetIgnoreFriend(SteamID steamIDFriend, bool bIgnore) = 0;

		virtual bool ReportChatDeclined(SteamID steamID) = 0;


		virtual bool CreateFriendsGroup(const char* pchGroupName) = 0;
		virtual bool DeleteFriendsGroup(int16_t iGroupID) = 0;
		virtual bool RenameFriendsGroup(const char* pchNewGroupName, int16_t iGroupID) = 0;
		virtual bool AddFriendToGroup(SteamID steamID, int16_t iGroupID) = 0;
		virtual bool RemoveFriendFromGroup(SteamID steamID, int16_t iGroupID) = 0;
		virtual bool IsFriendMemberOfFriendsGroup(SteamID steamID, int16_t iGroupID) = 0;
		virtual int16_t GetFriendsGroupCount() = 0;
		virtual int16_t GetFriendsGroupIDByIndex(int16_t iGroupIndex) = 0;
		virtual const char * GetFriendsGroupName(int16_t iGroupID) = 0;
		virtual int16_t GetFriendsGroupMembershipCount(int16_t iGroupID) = 0;
		virtual SteamID GetFirstFriendsGroupMember(int16_t iGroupID) = 0;
		virtual SteamID GetNextFriendsGroupMember(int16_t iGroupID) = 0;
		virtual int16_t GetGroupFriendsMembershipCount(SteamID steamID) = 0;
		virtual int16_t GetFirstGroupFriendsMember(SteamID steamID) = 0;
		virtual int16_t GetNextGroupFriendsMember(SteamID steamID) = 0;

		virtual const char * GetPlayerNickname(SteamID playerSteamID) = 0;
		virtual bool SetPlayerNickname(SteamID playerSteamID, const char *cszNickname) = 0;

		virtual uint32_t GetFriendsSteamLevel(SteamID steamIDFriend) = 0;

		virtual int32_t GetChatMessagesCount(SteamID steamIDFriend) = 0;
		// chat message iteration
		// returns the number of bytes in the message, filling pvData with as many of those bytes as possible
		// returns 0 if the steamID or iChatID are invalid
		virtual int32_t GetChatMessage(SteamID steamIDFriend, int32_t iChatID, void *pvData, int32_t cubData, int *peChatEntryType, SteamID* pSteamIDChatter, int *puTime) = 0;

		// generic friend->friend message sending, takes a sized buffer
		virtual bool SendMsgToFriend(SteamID steamIDFriend, int eChatEntryType, const void *pvMsgBody, int32_t cubMsgBody) = 0;

		// clears the chat history - should be called when a chat dialog closes
		virtual void ClearChatHistory(SteamID steamIDFriend) = 0;

		virtual int32_t GetKnownClanCount() = 0;
		virtual SteamID GetKnownClanByIndex(int32_t iClan) = 0;
		virtual int32_t GetClanCount() = 0;
		virtual SteamID GetClanByIndex(int32_t iClan) = 0;

		virtual const char *GetClanName(SteamID steamIDClan) = 0;
		virtual const char *GetClanTag(SteamID steamIDClan) = 0;

		virtual bool GetFriendActivityCounts(int32_t *pnOnline, int32_t *pnInGame, bool bExcludeTaggedFriends) = 0;
		virtual bool GetClanActivityCounts(SteamID steamID, int32_t *pnOnline, int32_t *pnInGame, int32_t *pnChatting) = 0;

		virtual uint64_t DownloadClanActivityCounts(SteamID groupIDs[], int32_t nIds) = 0;
		virtual bool GetFriendsGroupActivityCounts(int16_t iGroupID, int32_t *pnOnline, int32_t *pnInGame) = 0;

		virtual bool IsClanPublic(SteamID steamID) = 0;

		virtual uint64_t JoinClanChatRoom(SteamID groupID) = 0;
		virtual bool LeaveClanChatRoom(SteamID groupID) = 0;
		virtual int32_t GetClanChatMemberCount(SteamID groupID) = 0;
		virtual SteamID GetChatMemberByIndex(SteamID groupID, int32_t iIndex) = 0;
		virtual bool SendClanChatMessage(SteamID groupID, const char *cszMessage) = 0;
		virtual int32_t GetClanChatMessage(SteamID groupID, int32_t iChatID, void *pvData, int32_t cubData, int *peChatEntryType, SteamID *pSteamIDChatter) = 0;
		virtual bool IsClanChatAdmin(SteamID groupID, SteamID userID) = 0;
		virtual bool IsClanChatWindowOpenInSteam(SteamID groupID) = 0;
		virtual bool OpenClanChatWindowInSteam(SteamID groupID) = 0;
		virtual bool CloseClanChatWindowInSteam(SteamID groupID) = 0;
		virtual bool SetListenForFriendsMessages(bool bListen) = 0;
		virtual bool ReplyToFriendMessage(SteamID friendID, const char *cszMessage) = 0;
		virtual int32_t GetFriendMessage(SteamID friendID, int32_t iChatID, void *pvData, int32_t cubData, int *peChatEntryType) = 0;

		virtual bool InviteFriendToClan(SteamID steamIDfriend, SteamID steamIDclan) = 0;
		virtual bool AcknowledgeInviteToClan(SteamID steamID, bool bAcceptOrDenyClanInvite) = 0;

		// iterators for any source
		virtual int32_t GetFriendCountFromSource(SteamID steamIDSource) = 0;
		virtual SteamID GetFriendFromSourceByIndex(SteamID steamIDSource, int32_t iFriend) = 0;
		virtual bool IsUserInSource(SteamID steamIDUser, SteamID steamIDSource) = 0;

		virtual int32_t GetCoplayFriendCount() = 0;
		virtual SteamID GetCoplayFriend(int32_t iCoplayEvent) = 0;

		virtual int GetFriendCoplayTime(SteamID steamIDFriend) = 0;
		virtual int GetFriendCoplayGame(SteamID steamIDFriend) = 0;

		virtual bool SetRichPresence(int nAppId, const char *pchKey, const char *pchValue) = 0;
		virtual void ClearRichPresence(int nAppId) = 0;
		virtual const char* GetFriendRichPresence(int nAppId, SteamID steamIDFriend, const char *pchKey) = 0;
		virtual int32_t GetFriendRichPresenceKeyCount(int nAppId, SteamID steamIDFriend) = 0;
		virtual const char* GetFriendRichPresenceKeyByIndex(int nAppId, SteamID steamIDFriend, int32_t iIndex) = 0;

		virtual void RequestFriendRichPresence(int nAppId, SteamID steamIDFriend) = 0;

		virtual bool JoinChatRoom(SteamID steamIDChat) = 0;
		virtual void LeaveChatRoom(SteamID steamIDChat) = 0;

		virtual bool InviteUserToChatRoom(SteamID steamIDChat, SteamID steamIDInvitee) = 0;

		virtual bool SendChatMsg(SteamID steamIDChat, int eChatEntryType, const void *pvMsgBody, int32_t cubMsgBody) = 0;

		virtual int32_t GetChatRoomMessagesCount(SteamID steamIDChat) = 0;

		virtual int32_t GetChatRoomEntry(SteamID steamIDChat, int32_t iChatID, SteamID *steamIDuser, void *pvData, int32_t cubData, int *peChatEntryType) = 0;

		virtual void ClearChatRoomHistory(SteamID steamID) = 0;

		virtual bool SerializeChatRoomDlg(SteamID steamIDChat, void const* pvHistory, int32_t cubHistory) = 0;
		virtual int32_t GetSizeOfSerializedChatRoomDlg(SteamID steamIDChat) = 0;
		virtual bool GetSerializedChatRoomDlg(SteamID steamIDChat, void* pvHistory, int32_t cubBuffer, int32_t* pcubData) = 0;
		virtual bool ClearSerializedChatRoomDlg(SteamID steamIDChat) = 0;

		virtual bool KickChatMember(SteamID steamIDChat, SteamID steamIDUserToActOn) = 0;
		virtual bool BanChatMember(SteamID steamIDChat, SteamID steamIDUserToActOn) = 0;
		virtual bool UnBanChatMember(SteamID steamIDChat, SteamID steamIDUserToActOn) = 0;

		virtual bool SetChatRoomType(SteamID steamIDChat, int eLobbyType) = 0;
		virtual bool GetChatRoomLockState(SteamID steamIDChat, bool *pbLocked) = 0;
		virtual bool GetChatRoomPermissions(SteamID steamIDChat, uint32_t *prgfChatRoomPermissions) = 0;

		virtual bool SetChatRoomModerated(SteamID steamIDChat, bool bModerated) = 0;
		virtual bool BChatRoomModerated(SteamID steamIDChat) = 0;

		virtual bool NotifyChatRoomDlgsOfUIChange(SteamID steamIDChat, bool bShowAvatars, bool bBeepOnNewMsg, bool bShowSteamIDs, bool bShowTimestampOnNewMsg) = 0;

		virtual bool TerminateChatRoom(SteamID steamIDChat) = 0;

		virtual int32_t GetChatRoomCount() = 0;
		virtual SteamID GetChatRoomByIndex(int32_t iChatRoom) = 0;

		virtual const char *GetChatRoomName(SteamID steamIDChat) = 0;

		virtual bool BGetChatRoomMemberDetails(SteamID steamIDChat, SteamID steamIDUser, uint32_t* prgfChatMemberDetails, uint32_t* prgfChatMemberDetailsLocal) = 0;

		virtual void CreateChatRoom(int eType, uint64_t ulGameID, const char *pchName, int eLobbyType, SteamID steamIDClan, SteamID steamIDFriendChat, SteamID steamIDInvited, uint32_t rgfChatPermissionOfficer, uint32_t rgfChatPermissionMember, uint32_t rgfChatPermissionAll) = 0;

		virtual void VoiceCallNew(SteamID steamIDLocalPeer, SteamID steamIDRemotePeer) = 0;
		virtual void VoiceCall(SteamID steamIDLocalPeer, SteamID steamIDRemotePeer) = 0;
		virtual void VoiceHangUp(SteamID steamIDLocalPeer, int hVoiceCall) = 0;

		virtual void SetVoiceSpeakerVolume(float flVolume) = 0;
		virtual void SetVoiceMicrophoneVolume(float flVolume) = 0;

		virtual void SetAutoAnswer(bool bAutoAnswer) = 0;

		virtual void VoiceAnswer(int hVoiceCall) = 0;
		virtual void AcceptVoiceCall(SteamID steamIDLocalPeer, SteamID steamIDRemotePeer) = 0;

		virtual void VoicePutOnHold(int hVoiceCall, bool bLocalHold) = 0;
		virtual bool BVoiceIsLocalOnHold(int hVoiceCall) = 0;
		virtual bool BVoiceIsRemoteOnHold(int hVoiceCall) = 0;

		virtual void SetDoNotDisturb(bool bDoNotDisturb) = 0;

		virtual void EnableVoiceNotificationSounds(bool bEnable) = 0;

		virtual void SetPushToTalkEnabled(bool bEnable) = 0;
		virtual bool IsPushToTalkEnabled() = 0;

		virtual void SetPushToTalkKey(int32_t nVirtualKey) = 0;
		virtual int32_t GetPushToTalkKey() = 0;

		virtual bool IsPushToTalkKeyDown() = 0;

		virtual void EnableVoiceCalibration(bool bState) = 0;
		virtual bool IsVoiceCalibrating() = 0;
		virtual float GetVoiceCalibrationSamplePeak() = 0;

		virtual void SetMicBoost(bool bBoost) = 0;
		virtual bool GetMicBoost() = 0;

		virtual bool HasHardwareMicBoost() = 0;

		virtual const char *GetMicDeviceName() = 0;

		virtual void StartTalking(int hVoiceCall) = 0;
		virtual void EndTalking(int hVoiceCall) = 0;

		virtual bool VoiceIsValid(int hVoiceCall) = 0;

		virtual void SetAutoReflectVoice(bool bState) = 0;

		virtual int GetCallState(int hVoiceCall) = 0;

		virtual float GetVoiceMicrophoneVolume() = 0;
		virtual float GetVoiceSpeakerVolume() = 0;

		virtual float TimeSinceLastVoiceDataReceived(int hVoiceCall) = 0;
		virtual float TimeSinceLastVoiceDataSend(int hVoiceCall) = 0;

		virtual bool BCanSend(int hVoiceCall) = 0;
		virtual bool BCanReceive(int hVoiceCall) = 0;

		virtual float GetEstimatedBitsPerSecond(int hVoiceCall, bool bIncoming) = 0;
		virtual float GetPeakSample(int hVoiceCall, bool bIncoming) = 0;

		virtual void SendResumeRequest(int hVoiceCall) = 0;

		virtual void OpenChatDialog(SteamID steamID) = 0;

		virtual void StartChatRoomVoiceSpeaking(SteamID steamIDChat, SteamID steamIDMember) = 0;
		virtual void EndChatRoomVoiceSpeaking(SteamID steamIDChat, SteamID steamIDMember) = 0;

		virtual int GetFriendLastLogonTime(SteamID steamIDFriend) = 0;
		virtual int GetFriendLastLogoffTime(SteamID steamIDFriend) = 0;

		virtual int32_t GetChatRoomVoiceTotalSlotCount(SteamID steamIDChat) = 0;
		virtual int32_t GetChatRoomVoiceUsedSlotCount(SteamID steamIDChat) = 0;
		virtual SteamID GetChatRoomVoiceUsedSlot(SteamID steamIDChat, int32_t iSlot) = 0;
		virtual int GetChatRoomVoiceStatus(SteamID steamIDChat, SteamID steamIDSpeaker) = 0;

		virtual bool BChatRoomHasAvailableVoiceSlots(SteamID steamIDChat) = 0;

		virtual bool BIsChatRoomVoiceSpeaking(SteamID steamIDChat, SteamID steamIDSpeaker) = 0;

		virtual float GetChatRoomPeakSample(SteamID steamIDChat, SteamID steamIDSpeaker, bool bIncoming) = 0;

		virtual void ChatRoomVoiceRetryConnections(SteamID steamIDChat) = 0;

		virtual void SetPortTypes(uint32_t unFlags) = 0;

		virtual void ReinitAudio() = 0;

		virtual void SetInGameVoiceSpeaking(SteamID steamIDUser, bool bSpeaking) = 0;

		virtual bool IsInGameVoiceSpeaking() = 0;

		virtual void ActivateGameOverlay(const char *pchDialog) = 0;
		virtual void ActivateGameOverlayToUser(const char *pchDialog, SteamID steamID) = 0;
		virtual void ActivateGameOverlayToWebPage(const char *pchURL) = 0;
		virtual void ActivateGameOverlayToStore(int nAppId, int eFlag) = 0;
		virtual void ActivateGameOverlayInviteDialog(SteamID steamIDLobby) = 0;

		virtual void NotifyGameOverlayStateChanged(bool bActive) = 0;
		virtual void NotifyGameServerChangeRequested(const char *pchServerAddress, const char *pchPassword) = 0;
		virtual bool NotifyLobbyJoinRequested(int nAppId, SteamID steamIDLobby, SteamID steamIDFriend) = 0;
		virtual bool NotifyRichPresenceJoinRequested(int nAppId, SteamID steamIDFriend, const char *pchConnectString) = 0;

		virtual int GetClanRelationship(SteamID steamIDclan) = 0;

		virtual int GetFriendClanRank(SteamID steamIDUser, SteamID steamIDClan) = 0;

		virtual bool VoiceIsAvailable() = 0;

		virtual void TestVoiceDisconnect(int hVoiceCall) = 0;
		virtual void TestChatRoomPeerDisconnect(SteamID steamIDChat, SteamID steamIDSpeaker) = 0;
		virtual void TestVoicePacketLoss(float flFractionOfIncomingPacketsToDrop) = 0;

		virtual int FindFriendVoiceChatHandle(SteamID steamIDFriend) = 0;

		virtual void RequestFriendsWhoPlayGame(int gameID) = 0;
		virtual uint32_t GetCountFriendsWhoPlayGame(int gameID) = 0;

		virtual SteamID GetFriendWhoPlaysGame(uint32_t iIndex, int gameID) = 0;
		virtual void SetPlayedWith(SteamID steamIDUserPlayedWith) = 0;

		virtual uint64_t RequestClanOfficerList(SteamID steamIDClan) = 0;
		virtual SteamID GetClanOwner(SteamID steamIDClan) = 0;
		virtual int32_t GetClanOfficerCount(SteamID steamIDClan) = 0;
		virtual SteamID GetClanOfficerByIndex(SteamID steamIDClan, int32_t iOfficer) = 0;

		virtual int GetUserRestrictions() = 0;

		virtual uint64_t RequestFriendProfileInfo(SteamID steamIDFriend) = 0;
		// Available keys: TimeCreated, RealName, CityName, StateName, CountryName, Headline, Playtime, Summary
		virtual const char* GetFriendProfileInfo(SteamID steamIDFriend, const char* pchKey) = 0;

		virtual bool InviteUserToGame(SteamID steamIDFriend, const char *pchConnectString) = 0;

		virtual int32_t GetOnlineConsoleFriendCount() = 0;

		virtual uint64_t RequestTrade(SteamID steamIDPartner) = 0;
		virtual void TradeResponse(uint32_t unTradeRequestID, bool bAccept) = 0;
		virtual void CancelTradeRequest(SteamID steamIDPartner) = 0;

		virtual bool HideFriend(SteamID steamIDFriend, bool bHide) = 0;
		virtual const char * GetFriendFacebookName(SteamID steamIDFriend) = 0;
		virtual uint64_t GetFriendFacebookID(SteamID steamIDFriend) = 0;

		virtual uint64_t GetFollowerCount(SteamID steamID) = 0;
		virtual uint64_t IsFollowing(SteamID steamID) = 0;
		virtual uint64_t EnumerateFollowingList(uint32_t uStartIndex) = 0;

		virtual void RequestFriendMessageHistory(SteamID steamIDFriend) = 0;
		virtual void RequestFriendMessageHistoryForOfflineMessages() = 0;

		virtual int32_t GetCountFriendsWithOfflineMessages() = 0;
		virtual uint32_t GetFriendWithOfflineMessage(int32_t iFriend) = 0;
		virtual void ClearFriendHasOfflineMessage(uint32_t uFriend) = 0;

		virtual void RequestEmoticonList() = 0;
		virtual int32_t GetEmoticonCount() = 0;
		virtual const char *GetEmoticonName(int32_t iEmoticon) = 0;

		virtual void ClientLinkFilterInit() = 0;
		virtual uint32_t LinkDisposition(const char *) = 0;
	};
}
