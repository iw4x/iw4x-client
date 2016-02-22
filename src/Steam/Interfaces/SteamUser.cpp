#include "STDInclude.hpp"

namespace Steam
{
	::Utils::Cryptography::Token User::GuidToken;
	::Utils::Cryptography::ECDSA::Key User::GuidKey;

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
		static unsigned int subId = 0;

		SteamID id;

		if (!subId)
		{
			if (Components::Dedicated::IsDedicated()) // Dedi guid
			{
				subId = ~0xDED1CA7E;
			}
			else if (Components::Singleton::IsFirstInstance()) // Hardware guid
			{
				if (!User::GuidKey.IsValid())
				{
					Proto::Auth::Certificate cert;

					if (cert.ParseFromString(::Utils::ReadFile("players/guid.dat")))
					{
						User::GuidKey.Import(cert.privatekey(), PK_PRIVATE);
						User::GuidToken = cert.token();
					}
					
					if (!User::GuidKey.IsValid())
					{
						User::GuidToken.Clear();
						User::GuidKey = ::Utils::Cryptography::ECDSA::GenerateKey(512);

						cert.set_token(User::GuidToken.ToString());
						cert.set_privatekey(User::GuidKey.Export(PK_PRIVATE));

						::Utils::WriteFile("players/guid.dat", cert.SerializeAsString());
					}
				}

				std::string publicKey = User::GuidKey.GetPublicKey();
				subId = ::Utils::OneAtATime(publicKey.data(), publicKey.size());
			}
			else // Random guid
			{
				subId = (Game::Com_Milliseconds() + timeGetTime());
			}

			subId &= ~0x80000000; // Ensure it's positive
		}

		id.Bits = 0x110000100000000 | subId;
		return id;
	}

	int User::InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure)
	{
		Components::Logger::Print("%s\n", __FUNCTION__);
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
