#pragma once

namespace Steam
{
  // Note that x64 asks ISteamApps whether the DLC map packs are installed,
  // which x86 has no import for. We declare STEAMAPPS_INTERFACE_VERSION008
  // up to BIsDlcInstalled(), the last slot the game calls.
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
  };
}
