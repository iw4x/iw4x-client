#pragma once

namespace Steam
{
	class GameServer
	{
	public:
		virtual void LogOn();
		virtual void LogOff();
		virtual bool LoggedOn();
		virtual bool Secure();
		virtual SteamID GetSteamID();
		virtual bool SendUserConnectAndAuthenticate(unsigned int unIPClient, const void *pvAuthBlob, unsigned int cubAuthBlobSize, SteamID *pSteamIDUser);
		virtual SteamID CreateUnauthenticatedUserConnection();
		virtual void SendUserDisconnect(SteamID steamIDUser);
		virtual bool UpdateUserData(SteamID steamIDUser, const char *pchPlayerName, unsigned int uScore);
		virtual bool SetServerType(unsigned int unServerFlags, unsigned int unGameIP, unsigned short unGamePort, unsigned short unSpectatorPort, unsigned short usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode);
		virtual void UpdateServerStatus(int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName);
		virtual void UpdateSpectatorPort(unsigned short unSpectatorPort);
		virtual void SetGameType(const char *pchGameType);
		virtual bool GetUserAchievementStatus(SteamID steamID, const char *pchAchievementName);
		virtual void GetGameplayStats();
		virtual bool RequestUserGroupStatus(SteamID steamIDUser, SteamID steamIDGroup);
		virtual unsigned int GetPublicIP();
		virtual void SetGameData(const char *pchGameData);
		virtual int UserHasLicenseForApp(SteamID steamID, unsigned int appID);
	};
}
