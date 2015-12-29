namespace Steam
{
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
}
