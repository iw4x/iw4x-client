#include <STDInclude.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	void GameServer::LogOn()
	{
	}

	void GameServer::LogOff()
	{
	}

	bool GameServer::LoggedOn()
	{
		return true;
	}

	bool GameServer::Secure()
	{
		return false;
	}

	SteamID GameServer::GetSteamID()
	{
		return SteamID();
	}

	bool GameServer::SendUserConnectAndAuthenticate(unsigned int unIPClient, const void *pvAuthBlob, unsigned int cubAuthBlobSize, SteamID *pSteamIDUser)
	{
		return true;
	}

	SteamID GameServer::CreateUnauthenticatedUserConnection()
	{
		return SteamID();
	}

	void GameServer::SendUserDisconnect(SteamID steamIDUser)
	{
	}

	bool GameServer::UpdateUserData(SteamID steamIDUser, const char *pchPlayerName, unsigned int uScore)
	{
		return true;
	}

	bool GameServer::SetServerType(unsigned int unServerFlags, unsigned int unGameIP, unsigned short unGamePort, unsigned short unSpectatorPort, unsigned short usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode)
	{
		return true;
	}

	void GameServer::UpdateServerStatus(int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName)
	{
	}

	void GameServer::UpdateSpectatorPort(unsigned short unSpectatorPort)
	{
	}

	void GameServer::SetGameType(const char *pchGameType)
	{
	}

	bool GameServer::GetUserAchievementStatus(SteamID steamID, const char *pchAchievementName)
	{
		return false;
	}

	void GameServer::GetGameplayStats()
	{
	}

	bool GameServer::RequestUserGroupStatus(SteamID steamIDUser, SteamID steamIDGroup)
	{
		return false;
	}

	unsigned int GameServer::GetPublicIP()
	{
		return 0;
	}

	void GameServer::SetGameData(const char *pchGameData)
	{
	}

	int GameServer::UserHasLicenseForApp(SteamID steamID, unsigned int appID)
	{
		return 0;
	}
}

STEAM_IGNORE_WARNINGS_END
