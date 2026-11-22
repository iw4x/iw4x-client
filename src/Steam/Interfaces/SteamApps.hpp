#pragma once

namespace Steam
{
  // Note that x64 asks ISteamApps whether the DLC map packs are installed,
  // which x86 has no import for. We lay it out the way Steamworks SDK 1.20
  // does STEAMAPPS_INTERFACE_VERSION005, the same as the game's other
  // interfaces.
  //
  class Apps
  {
  public:
    virtual bool BIsSubscribed();
    virtual bool BIsLowViolence();
    virtual bool BIsCybercafe();
    virtual bool BIsVACBanned();
    virtual const char* GetCurrentGameLanguage();
    virtual const char* GetAvailableGameLanguages();
    virtual bool BIsSubscribedApp(unsigned int appID);
    virtual bool BIsDlcInstalled(unsigned int appID);
    virtual unsigned int GetEarliestPurchaseUnixTime(unsigned int nAppID);
    virtual bool BIsSubscribedFromFreeWeekend();
    virtual int GetDLCCount();
    virtual bool BGetDLCDataByIndex(int iDLC, unsigned int *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize);
    virtual void InstallDLC(unsigned int nAppID);
    virtual void UninstallDLC(unsigned int nAppID);
    virtual void RequestAppProofOfPurchaseKey(unsigned int nAppID);
    virtual uint64_t RegisterActivationCode(const char *pchActivationCode);
  };
}
