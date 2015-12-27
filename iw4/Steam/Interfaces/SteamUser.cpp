#include "..\..\STDInclude.hpp"

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
		static int subId = 0;

		SteamID id;

		if (!subId)
		{
			if (Components::Dedicated::IsDedicated()) // Dedi guid
			{
				subId = -0xDED1CADE;
			}
			else if (Components::Singleton::IsFirstInstance()) // Hardware guid
			{
				DATA_BLOB Data[2];
				Data[0].pbData = (BYTE *)"AAAAAAAAAA";
				Data[0].cbData = 10;

				CryptProtectData(&Data[0], NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &Data[1]);

				subId = ::Utils::OneAtATime((char*)Data[1].pbData, 52);
			}
			else // Random guid
			{
				subId = (Game::Com_Milliseconds() + timeGetTime());
			}
		}

		id.Bits = 0x110000100000000 | subId;
		return id;
	}

	int User::InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure)
	{
		// TODO: Generate auth ticket!
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
		return 1;
	}
}
