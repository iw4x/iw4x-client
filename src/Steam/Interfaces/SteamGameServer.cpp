#include "SteamGameServer.hpp"

namespace Steam
{
  bool GameServer::InitGameServer(unsigned int /*unIP*/, unsigned short /*usGamePort*/, unsigned short /*usQueryPort*/, unsigned int /*unFlags*/, unsigned int /*nGameAppId*/, const char* /*pchVersionString*/)
  {
    return true;
  }

  void GameServer::SetProduct(const char* /*pszProduct*/)
  {
  }

  void GameServer::SetGameDescription(const char* /*pszGameDescription*/)
  {
  }

  void GameServer::SetModDir(const char* /*pszModDir*/)
  {
  }

  void GameServer::SetDedicatedServer(bool /*bDedicated*/)
  {
  }

  void GameServer::LogOn(const char* /*pszAccountName*/, const char* /*pszPassword*/)
  {
  }

  void GameServer::LogOnAnonymous()
  {
  }

  void GameServer::LogOff()
  {
  }

  bool GameServer::BLoggedOn()
  {
    return true;
  }

  bool GameServer::BSecure()
  {
    return false;
  }

  SteamID* GameServer::GetSteamID(SteamID* result)
  {
    *result = SteamID();
    return result;
  }

  bool GameServer::WasRestartRequested()
  {
    return false;
  }

  void GameServer::SetMaxPlayerCount(int /*cPlayersMax*/)
  {
  }

  void GameServer::SetBotPlayerCount(int /*cBotplayers*/)
  {
  }

  void GameServer::SetServerName(const char* /*pszServerName*/)
  {
  }

  void GameServer::SetMapName(const char* /*pszMapName*/)
  {
  }

  void GameServer::SetPasswordProtected(bool /*bPasswordProtected*/)
  {
  }

  void GameServer::SetSpectatorPort(unsigned short /*unSpectatorPort*/)
  {
  }

  void GameServer::SetSpectatorServerName(const char* /*pszSpectatorServerName*/)
  {
  }

  void GameServer::ClearAllKeyValues()
  {
  }

  void GameServer::SetKeyValue(const char* /*pKey*/, const char* /*pValue*/)
  {
  }

  void GameServer::SetGameTags(const char* /*pchGameTags*/)
  {
  }

  void GameServer::SetGameData(const char* /*pchGameData*/)
  {
  }

  void GameServer::SetRegion(const char* /*pszRegion*/)
  {
  }

  bool GameServer::SendUserConnectAndAuthenticate(unsigned int /*unIPClient*/, const void* /*pvAuthBlob*/, unsigned int /*cubAuthBlobSize*/, SteamID* /*pSteamIDUser*/)
  {
    return true;
  }

  SteamID* GameServer::CreateUnauthenticatedUserConnection(SteamID* result)
  {
    *result = SteamID();
    return result;
  }

  void GameServer::SendUserDisconnect(SteamID /*steamIDUser*/)
  {
  }

  bool GameServer::BUpdateUserData(SteamID /*steamIDUser*/, const char* /*pchPlayerName*/, unsigned int /*uScore*/)
  {
    return true;
  }

  unsigned int GameServer::GetAuthSessionTicket(void* /*pTicket*/, int /*cbMaxTicket*/, unsigned int* /*pcbTicket*/)
  {
    return 0;
  }

  int GameServer::BeginAuthSession(const void* /*pAuthTicket*/, int /*cbAuthTicket*/, SteamID /*steamID*/)
  {
    return 0;
  }

  void GameServer::EndAuthSession(SteamID /*steamID*/)
  {
  }

  void GameServer::CancelAuthTicket(unsigned int /*hAuthTicket*/)
  {
  }

  int GameServer::UserHasLicenseForApp(SteamID /*steamID*/, unsigned int /*appID*/)
  {
    return 0;
  }

  bool GameServer::RequestUserGroupStatus(SteamID /*steamIDUser*/, SteamID /*steamIDGroup*/)
  {
    return false;
  }

  void GameServer::GetGameplayStats()
  {
  }

  uint64_t GameServer::GetServerReputation()
  {
    return 0;
  }

  unsigned int GameServer::GetPublicIP()
  {
    return 0;
  }

  bool GameServer::HandleIncomingPacket(const void* /*pData*/, int /*cbData*/, unsigned int /*srcIP*/, unsigned short /*srcPort*/)
  {
    return false;
  }

  int GameServer::GetNextOutgoingPacket(void* /*pOut*/, int /*cbMaxOut*/, unsigned int* /*pNetAdr*/, unsigned short* /*pPort*/)
  {
    return 0;
  }

  void GameServer::EnableHeartbeats(bool /*bActive*/)
  {
  }

  void GameServer::SetHeartbeatInterval(int /*iHeartbeatInterval*/)
  {
  }

  void GameServer::ForceHeartbeat()
  {
  }

  uint64_t GameServer::AssociateWithClan(SteamID /*steamIDClan*/)
  {
    return 0;
  }

  uint64_t GameServer::ComputeNewPlayerCompatibility(SteamID /*steamIDNewPlayer*/)
  {
    return 0;
  }
}
