#include "SteamUser.hpp"

#include "Components/Modules/Auth.hpp"

namespace Steam
{
  int User::GetHSteamUser()
  {
    return 0;
  }

  bool User::BLoggedOn()
  {
    return true;
  }

  SteamID* User::GetSteamID(SteamID* result)
  {
    static unsigned __int64 idBits = 0;

    if (!idBits)
    {
      if (Components::Singleton::IsFirstInstance() && !Components::Dedicated::IsEnabled()) // ECDSA guid
      {
        idBits = Components::Auth::GetKeyHash();
      }
      else // Random guid
      {
        idBits = (static_cast<unsigned __int64>(Game::Sys_Milliseconds()) << 32) | timeGetTime();
      }
    }

    result->bits = idBits;
    return result;
  }

  int User::InitiateGameConnection(void* /*pAuthBlob*/, int /*cbMaxAuthBlob*/, SteamID /*steamIDGameServer*/, unsigned int /*unIPServer*/, unsigned short /*usPortServer*/, bool /*bSecure*/)
  {
    return 0;
  }

  void User::TerminateGameConnection(unsigned int /*unIPServer*/, unsigned short /*usPortServer*/)
  {
  }

  void User::TrackAppUsageEvent(SteamID /*gameID*/, int /*eAppUsageEvent*/, const char* /*pchExtraInfo*/)
  {
  }

  bool User::GetUserDataFolder(char* /*pchBuffer*/, int /*cubBuffer*/)
  {
    return false;
  }

  void User::StartVoiceRecording()
  {
  }

  void User::StopVoiceRecording()
  {
  }

  int User::GetAvailableVoice(unsigned int* pcbCompressed, unsigned int* pcbUncompressed, unsigned int /*nUncompressedVoiceDesiredSampleRate*/)
  {
    if (pcbCompressed)
    {
      *pcbCompressed = 0;
    }

    if (pcbUncompressed)
    {
      *pcbUncompressed = 0;
    }

    return 0;
  }

  int User::GetVoice(bool /*bWantCompressed*/, void* /*pDestBuffer*/, unsigned int /*cbDestBufferSize*/, unsigned int* nBytesWritten, bool /*bWantUncompressed*/, void* /*pUncompressedDestBuffer*/, unsigned int /*cbUncompressedDestBufferSize*/, unsigned int* nUncompressBytesWritten, unsigned int /*nUncompressedVoiceDesiredSampleRate*/)
  {
    if (nBytesWritten)
    {
      *nBytesWritten = 0;
    }

    if (nUncompressBytesWritten)
    {
      *nUncompressBytesWritten = 0;
    }

    return 0;
  }

  int User::DecompressVoice(const void* /*pCompressed*/, unsigned int /*cbCompressed*/, void* /*pDestBuffer*/, unsigned int /*cbDestBufferSize*/, unsigned int* /*nBytesWritten*/, unsigned int /*nDesiredSampleRate*/)
  {
    return 0;
  }

  unsigned int User::GetVoiceOptimalSampleRate()
  {
    return 0;
  }

  unsigned int User::GetAuthSessionTicket(void* /*pTicket*/, int /*cbMaxTicket*/, unsigned int* /*pcbTicket*/)
  {
    return 0;
  }

  int User::BeginAuthSession(const void* /*pAuthTicket*/, int /*cbAuthTicket*/, SteamID /*steamID*/)
  {
    return 0;
  }

  void User::EndAuthSession(SteamID /*steamID*/)
  {
  }

  void User::CancelAuthTicket(unsigned int /*hAuthTicket*/)
  {
  }

  unsigned int User::UserHasLicenseForApp(SteamID /*steamID*/, unsigned int /*appID*/)
  {
    return 0;
  }

  bool User::BIsBehindNAT()
  {
    return false;
  }

  void User::AdvertiseGame(SteamID /*steamIDGameServer*/, unsigned int /*unIPServer*/, unsigned short /*usPortServer*/)
  {
  }

  uint64_t User::RequestEncryptedAppTicket(void* /*pDataToInclude*/, int /*cbDataToInclude*/)
  {
    return 0;
  }

  bool User::GetEncryptedAppTicket(void* /*pTicket*/, int /*cbMaxTicket*/, unsigned int* /*pcbTicket*/)
  {
    return false;
  }

  void User::LogOn(bool /*bInteractive*/)
  {
  }

  void User::LogOnAndLinkSteamAccountToPSN(bool /*bInteractive*/, const char* /*pchUserName*/, const char* /*pchPassword*/)
  {
  }

  void User::LogOnAndCreateNewSteamAccountIfNeeded(bool /*bInteractive*/)
  {
  }

  SteamID* User::GetConsoleSteamID(SteamID* result)
  {
    *result = {};
    return result;
  }
}
