#include "SteamApps.hpp"

namespace Steam
{
  bool Apps::BIsSubscribed()
  {
    return true;
  }

  bool Apps::BIsLowViolence()
  {
    return false;
  }

  bool Apps::BIsCybercafe()
  {
    return false;
  }

  bool Apps::BIsVACBanned()
  {
    return false;
  }

  const char* Apps::GetCurrentGameLanguage()
  {
    return "english";
  }

  const char* Apps::GetAvailableGameLanguages()
  {
    return "english";
  }

  bool Apps::BIsSubscribedApp([[maybe_unused]] unsigned int appID)
  {
    return true;
  }

  // Note that on x86 the game registers the original maps alone and leaves
  // the DLC map packs to us. So we report none installed, which keeps x64
  // from registering DLC_2 and DLC_3 itself.
  //
  bool Apps::BIsDlcInstalled([[maybe_unused]] unsigned int appID)
  {
    return false;
  }

  unsigned int Apps::GetEarliestPurchaseUnixTime(unsigned int /*nAppID*/)
  {
    return 0;
  }

  bool Apps::BIsSubscribedFromFreeWeekend()
  {
    return false;
  }

  int Apps::GetDLCCount()
  {
    return 0;
  }

  bool Apps::BGetDLCDataByIndex(int /*iDLC*/, unsigned int* /*pAppID*/, bool* /*pbAvailable*/, char* /*pchName*/, int /*cchNameBufferSize*/)
  {
    return false;
  }

  void Apps::InstallDLC(unsigned int /*nAppID*/)
  {
  }

  void Apps::UninstallDLC(unsigned int /*nAppID*/)
  {
  }

  void Apps::RequestAppProofOfPurchaseKey(unsigned int /*nAppID*/)
  {
  }

  uint64_t Apps::RegisterActivationCode(const char* /*pchActivationCode*/)
  {
    return 0;
  }
}
