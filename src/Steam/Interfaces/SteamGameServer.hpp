#pragma once

namespace Steam
{
  // Note that the game calls the methods of the interface by their slots,
  // and x64 is built against a newer version of it than x86. So we lay it
  // out as Steamworks SDK 1.20 does SteamGameServer011, the earliest
  // version whose slots match the ones x64 calls.
  //
  class GameServer
  {
  public:
    virtual bool InitGameServer(unsigned int unIP, unsigned short usGamePort, unsigned short usQueryPort, unsigned int unFlags, unsigned int nGameAppId, const char *pchVersionString);
    virtual void SetProduct(const char *pszProduct);
    virtual void SetGameDescription(const char *pszGameDescription);
    virtual void SetModDir(const char *pszModDir);
    virtual void SetDedicatedServer(bool bDedicated);
    virtual void LogOn(const char *pszAccountName, const char *pszPassword);
    virtual void LogOnAnonymous();
    virtual void LogOff();
    virtual bool BLoggedOn();
    virtual bool BSecure();
    virtual SteamID* GetSteamID(SteamID* result);
    virtual bool WasRestartRequested();
    virtual void SetMaxPlayerCount(int cPlayersMax);
    virtual void SetBotPlayerCount(int cBotplayers);
    virtual void SetServerName(const char *pszServerName);
    virtual void SetMapName(const char *pszMapName);
    virtual void SetPasswordProtected(bool bPasswordProtected);
    virtual void SetSpectatorPort(unsigned short unSpectatorPort);
    virtual void SetSpectatorServerName(const char *pszSpectatorServerName);
    virtual void ClearAllKeyValues();
    virtual void SetKeyValue(const char *pKey, const char *pValue);
    virtual void SetGameTags(const char *pchGameTags);
    virtual void SetGameData(const char *pchGameData);
    virtual void SetRegion(const char *pszRegion);
    virtual bool SendUserConnectAndAuthenticate(unsigned int unIPClient, const void *pvAuthBlob, unsigned int cubAuthBlobSize, SteamID *pSteamIDUser);
    virtual SteamID* CreateUnauthenticatedUserConnection(SteamID* result);
    virtual void SendUserDisconnect(SteamID steamIDUser);
    virtual bool BUpdateUserData(SteamID steamIDUser, const char *pchPlayerName, unsigned int uScore);
    virtual unsigned int GetAuthSessionTicket(void *pTicket, int cbMaxTicket, unsigned int *pcbTicket);
    virtual int BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, SteamID steamID);
    virtual void EndAuthSession(SteamID steamID);
    virtual void CancelAuthTicket(unsigned int hAuthTicket);
    virtual int UserHasLicenseForApp(SteamID steamID, unsigned int appID);
    virtual bool RequestUserGroupStatus(SteamID steamIDUser, SteamID steamIDGroup);
    virtual void GetGameplayStats();
    virtual uint64_t GetServerReputation();
    virtual unsigned int GetPublicIP();
    virtual bool HandleIncomingPacket(const void *pData, int cbData, unsigned int srcIP, unsigned short srcPort);
    virtual int GetNextOutgoingPacket(void *pOut, int cbMaxOut, unsigned int *pNetAdr, unsigned short *pPort);
    virtual void EnableHeartbeats(bool bActive);
    virtual void SetHeartbeatInterval(int iHeartbeatInterval);
    virtual void ForceHeartbeat();
    virtual uint64_t AssociateWithClan(SteamID steamIDClan);
    virtual uint64_t ComputeNewPlayerCompatibility(SteamID steamIDNewPlayer);
  };
}
