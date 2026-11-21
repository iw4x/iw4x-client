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
}
