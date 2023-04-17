#include <STDInclude.hpp>
#include "Components/Modules/Auth.hpp"

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	int User::GetHSteamUser()
	{
		return NULL;
	}

	bool User::LoggedOn()
	{
		return true;
	}

	SteamID User::GetSteamID()
	{
		static unsigned __int64 idBits = 0;

		SteamID id;

		if (!idBits)
		{
			if (Components::ZoneBuilder::IsEnabled())
			{
				idBits = *reinterpret_cast<unsigned __int64*>(const_cast<char*>("DEDICATE"));
			}
			else if (Components::Singleton::IsFirstInstance() && !Components::Dedicated::IsEnabled()) // ECDSA guid
			{
				idBits = Components::Auth::GetKeyHash();
			}
			else // Random guid
			{
				idBits = (static_cast<unsigned __int64>(Game::Sys_Milliseconds()) << 32) | timeGetTime();
			}
		}

		id.bits = idBits;
		return id;
	}

	int User::InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure)
	{
		return 0;
	}

	void User::TerminateGameConnection(unsigned int unIPServer, unsigned short usPortServer)
	{
	}

	void User::TrackAppUsageEvent(SteamID gameID, int eAppUsageEvent, const char *pchExtraInfo)
	{
	}

	bool User::GetUserDataFolder(char *pchBuffer, int cubBuffer)
	{
		return false;
	}

	void User::StartVoiceRecording()
	{
	}

	void User::StopVoiceRecording()
	{
	}

	int User::GetCompressedVoice(void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten)
	{
		return 0;
	}

	int User::DecompressVoice(void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten)
	{
		return 0;
	}

	unsigned int User::GetAuthSessionTicket(void *pTicket, int cbMaxTicket, unsigned int *pcbTicket)
	{
		return 0;
	}

	int User::BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, SteamID steamID)
	{
		return 0;
	}

	void User::EndAuthSession(SteamID steamID)
	{
	}

	void User::CancelAuthTicket(unsigned int hAuthTicket)
	{
	}

	unsigned int User::UserHasLicenseForApp(SteamID steamID, unsigned int appID)
	{
		return 0;
	}
}

STEAM_IGNORE_WARNINGS_END
