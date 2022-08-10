#include <STDInclude.hpp>

using namespace Components; 

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	const char* Friends::GetPersonaName()
	{
		return Dvar::Var("name").get<const char*>();
	}

	void Friends::SetPersonaName(const char *pchPersonaName)
	{
		Dvar::Var("name").set(pchPersonaName);
	}

	int Friends::GetPersonaState()
	{
		return 1;
	}

	int Friends::GetFriendCount(int eFriendFlags)
	{
		return 0;
	}

	SteamID Friends::GetFriendByIndex(int iFriend, int iFriendFlags)
	{
		return {};
	}

	int Friends::GetFriendRelationship(SteamID steamIDFriend)
	{
		return 0;
	}

	int Friends::GetFriendPersonaState(SteamID steamIDFriend)
	{
		return 0;
	}

	const char* Friends::GetFriendPersonaName(SteamID steamIDFriend)
	{
		return "";
	}

	int Friends::GetFriendAvatar(SteamID steamIDFriend, int eAvatarSize)
	{
		return 0;
	}

	bool Friends::GetFriendGamePlayed(SteamID steamIDFriend, FriendGameInfo *pFriendGameInfo)
	{
		return false;
	}

	const char* Friends::GetFriendPersonaNameHistory(SteamID steamIDFriend, int iPersonaName)
	{
		return "";
	}

	bool Friends::HasFriend(SteamID steamIDFriend, int eFriendFlags)
	{
		return false;
	}

	int Friends::GetClanCount()
	{
		return 0;
	}

	SteamID Friends::GetClanByIndex(int iClan)
	{
		return SteamID();
	}

	const char *Friends::GetClanName(SteamID steamIDClan)
	{
		return "3arc";
	}

	int Friends::GetFriendCountFromSource(SteamID steamIDSource)
	{
		return 0;
	}

	SteamID Friends::GetFriendFromSourceByIndex(SteamID steamIDSource, int iFriend)
	{
		return {};
	}

	bool Friends::IsUserInSource(SteamID steamIDUser, SteamID steamIDSource)
	{
		return false;
	}

	void Friends::SetInGameVoiceSpeaking(SteamID steamIDUser, bool bSpeaking)
	{
	}

	void Friends::ActivateGameOverlay(const char *pchDialog)
	{
	}

	void Friends::ActivateGameOverlayToUser(const char *pchDialog, SteamID steamID)
	{
	}

	void Friends::ActivateGameOverlayToWebPage(const char *pchURL)
	{
	}

	void Friends::ActivateGameOverlayToStore(unsigned int nAppID)
	{
	}

	void Friends::SetPlayedWith(SteamID steamIDUserPlayedWith)
	{
	}
}

STEAM_IGNORE_WARNINGS_END
