#pragma once

namespace Steam
{
  // As with the other interfaces, x64 calls a newer version of this one
  // than x86 does. We lay it out the way Steamworks SDK 1.20 does
  // SteamUser016, the earliest version whose slots match the ones that x64
  // calls (see Friends for why the earliest).
  //
  class User
  {
  public:
    virtual int GetHSteamUser();
    virtual bool BLoggedOn();
    virtual SteamID* GetSteamID(SteamID* result);

    virtual int InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure);
    virtual void TerminateGameConnection(unsigned int unIPServer, unsigned short usPortServer);
    virtual void TrackAppUsageEvent(SteamID gameID, int eAppUsageEvent, const char *pchExtraInfo = "");
    virtual bool GetUserDataFolder(char *pchBuffer, int cubBuffer);
    virtual void StartVoiceRecording();
    virtual void StopVoiceRecording();
    virtual int GetAvailableVoice(unsigned int *pcbCompressed, unsigned int *pcbUncompressed, unsigned int nUncompressedVoiceDesiredSampleRate);
    virtual int GetVoice(bool bWantCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, unsigned int cbUncompressedDestBufferSize, unsigned int *nUncompressBytesWritten, unsigned int nUncompressedVoiceDesiredSampleRate);
    virtual int DecompressVoice(const void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, unsigned int nDesiredSampleRate);
    virtual unsigned int GetVoiceOptimalSampleRate();
    virtual unsigned int GetAuthSessionTicket(void *pTicket, int cbMaxTicket, unsigned int *pcbTicket);
    virtual int BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, SteamID steamID);
    virtual void EndAuthSession(SteamID steamID);
    virtual void CancelAuthTicket(unsigned int hAuthTicket);
    virtual unsigned int UserHasLicenseForApp(SteamID steamID, unsigned int appID);
    virtual bool BIsBehindNAT();
    virtual void AdvertiseGame(SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer);
    virtual uint64_t RequestEncryptedAppTicket(void *pDataToInclude, int cbDataToInclude);
    virtual bool GetEncryptedAppTicket(void *pTicket, int cbMaxTicket, unsigned int *pcbTicket);
    virtual void LogOn(bool bInteractive);
    virtual void LogOnAndLinkSteamAccountToPSN(bool bInteractive, const char *pchUserName, const char *pchPassword);
    virtual void LogOnAndCreateNewSteamAccountIfNeeded(bool bInteractive);
    virtual SteamID* GetConsoleSteamID(SteamID* result);

    SteamID GetSteamID()
    {
      SteamID result;
      GetSteamID(&result);
      return result;
    }
  };
}
