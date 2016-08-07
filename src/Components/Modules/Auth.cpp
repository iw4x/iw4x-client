#include "STDInclude.hpp"

namespace Components
{
	Auth::TokenIncrementing Auth::TokenContainer;

	Utils::Cryptography::Token Auth::GuidToken;
	Utils::Cryptography::Token Auth::ComputeToken;
	Utils::Cryptography::ECC::Key Auth::GuidKey;

	void Auth::SendConnectDataStub(Game::netsrc_t sock, Game::netadr_t adr, const char *format, int len)
	{
		// Ensure our certificate is loaded
		Steam::SteamUser()->GetSteamID();
		if (!Auth::GuidKey.IsValid())
		{
			Logger::SoftError("Connecting failed: Guid key is invalid!");
			return;
		}

		std::string connectString(format, len);
		Game::SV_Cmd_TokenizeString(connectString.data());

		Command::Params params(true);

		if (params.Length() < 3)
		{
			Game::SV_Cmd_EndTokenizedString();
			Logger::SoftError("Connecting failed: Command parsing error!");
			return;
		}

		Utils::InfoString infostr(params[2]);
		std::string challenge = infostr.Get("challenge");

		if (challenge.empty())
		{
			Game::SV_Cmd_EndTokenizedString();
			Logger::SoftError("Connecting failed: Challenge parsing error!");
			return;
		}

		Game::SV_Cmd_EndTokenizedString();

		Proto::Auth::Connect connectData;
		connectData.set_token(Auth::GuidToken.ToString());
		connectData.set_publickey(Auth::GuidKey.GetPublicKey());
		connectData.set_signature(Utils::Cryptography::ECC::SignMessage(Auth::GuidKey, challenge));
		connectData.set_infostring(connectString);

		Network::SendCommand(sock, adr, "connect", connectData.SerializeAsString());
	}

	void Auth::ParseConnectData(Game::msg_t* msg, Game::netadr_t addr)
	{
		Network::Address address(addr);

		// Parse proto data
		Proto::Auth::Connect connectData;
		if (msg->cursize <= 12 || !connectData.ParseFromString(std::string(&msg->data[12], msg->cursize - 12)))
		{
			Network::Send(address, "error\nInvalid connect packet!");
			return;
		}

#if DEBUG
		// Simply connect, if we're in debug mode, we ignore all security checks
		if (!connectData.infostring().empty())
		{
			Game::SV_Cmd_EndTokenizedString();
			Game::SV_Cmd_TokenizeString(connectData.infostring().data());
			Game::SV_DirectConnect(*address.Get());
		}
		else
		{
			Network::Send(address, "error\nInvalid infostring data!");
		}
#else
		// Validate proto data
		if (connectData.signature().empty() || connectData.publickey().empty() || connectData.token().empty() || connectData.infostring().empty())
		{
			Network::Send(address, "error\nInvalid connect data!");
			return;
		}

		// Setup new cmd params
		Game::SV_Cmd_EndTokenizedString();
		Game::SV_Cmd_TokenizeString(connectData.infostring().data());

		// Access the params
		Command::Params params(true);

		// Ensure there are enough params
		if (params.Length() < 3)
		{
			Network::Send(address, "error\nInvalid connect string!");
			return;
		}

		// Parse the infostring
		Utils::InfoString infostr(params[2]);

		// Read the required data
		std::string steamId = infostr.Get("xuid");
		std::string challenge = infostr.Get("challenge");

		if (steamId.empty() || challenge.empty())
		{
			Network::Send(address, "error\nInvalid connect data!");
			return;
		}

		// Parse the id
		unsigned __int64 xuid = strtoull(steamId.data(), nullptr, 16);
		unsigned int id = static_cast<unsigned int>(~0x110000100000000 & xuid);

		if ((xuid & 0xFFFFFFFF00000000) != 0x110000100000000 || id != (Utils::Cryptography::JenkinsOneAtATime::Compute(connectData.publickey()) & ~0x80000000))
		{
			Network::Send(address, "error\nXUID doesn't match the certificate!");
			return;
		}

		// Verify the signature
		Utils::Cryptography::ECC::Key key;
		key.Set(connectData.publickey());

		if (!key.IsValid() || !Utils::Cryptography::ECC::VerifyMessage(key, challenge, connectData.signature()))
		{
			Network::Send(address, "error\nChallenge signature was invalid!");
			return;
		}

		// Verify the security level
		uint32_t ourLevel = static_cast<uint32_t>(Dvar::Var("sv_securityLevel").Get<int>());
		uint32_t userLevel = Auth::GetZeroBits(connectData.token(), connectData.publickey());

		if (userLevel < ourLevel)
		{
			Network::Send(address, fmt::sprintf("error\nYour security level (%d) is lower than the server's security level (%d)", userLevel, ourLevel));
			return;
		}

		Logger::Print("Verified XUID %llX (%d) from %s\n", xuid, userLevel, address.GetCString());
		Game::SV_DirectConnect(*address.Get());
#endif
	}

	void __declspec(naked) Auth::DirectConnectStub()
	{
		__asm
		{
			push esi
			call Auth::ParseConnectData
			pop esi

			mov edi, 6265FEh
			jmp edi
		}
	}

	unsigned int Auth::GetKeyHash()
	{
		Auth::LoadKey();
		return (Utils::Cryptography::JenkinsOneAtATime::Compute(Auth::GuidKey.GetPublicKey()));
	}

	void Auth::StoreKey()
	{
		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Proto::Auth::Certificate cert;
			cert.set_token(Auth::GuidToken.ToString());
			cert.set_ctoken(Auth::ComputeToken.ToString());
			cert.set_privatekey(Auth::GuidKey.Export(PK_PRIVATE));

			Utils::IO::WriteFile("players/guid.dat", cert.SerializeAsString());
		}
	}

	void Auth::LoadKey(bool force)
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;
		if (!force && Auth::GuidKey.IsValid()) return;

		Proto::Auth::Certificate cert;
		if (cert.ParseFromString(::Utils::IO::ReadFile("players/guid.dat")))
		{
			Auth::GuidKey.Import(cert.privatekey(), PK_PRIVATE);
			Auth::GuidToken = cert.token();
			Auth::ComputeToken = cert.ctoken();
		}
		else
		{
			Auth::GuidKey.Free();
		}

		if (!Auth::GuidKey.IsValid())
		{
			Auth::GuidToken.Clear();
			Auth::ComputeToken.Clear();
			Auth::GuidKey = Utils::Cryptography::ECC::GenerateKey(512);
			Auth::StoreKey();
		}
	}

	uint32_t Auth::GetSecurityLevel()
	{
		return Auth::GetZeroBits(Auth::GuidToken, Auth::GuidKey.GetPublicKey());
	}

	void Auth::IncreaseSecurityLevel(uint32_t level, std::string command)
	{
		if (Auth::GetSecurityLevel() >= level) return;

		if (!Auth::TokenContainer.generating)
		{
			Auth::TokenContainer.cancel = false;
			Auth::TokenContainer.targetLevel = level;
			Auth::TokenContainer.command = command;

			// Open menu
			Command::Execute("openmenu security_increase_popmenu", true);

			// Start thread
			Auth::TokenContainer.thread = std::thread([&level] ()
			{
				Auth::TokenContainer.generating = true;
				Auth::TokenContainer.hashes = 0;
				Auth::TokenContainer.startTime = Game::Sys_Milliseconds();
				Auth::IncrementToken(Auth::GuidToken, Auth::ComputeToken, Auth::GuidKey.GetPublicKey(), Auth::TokenContainer.targetLevel, &Auth::TokenContainer.cancel, &Auth::TokenContainer.hashes);
				Auth::TokenContainer.generating = false;

				if (Auth::TokenContainer.cancel)
				{
					Logger::Print("Token incrementation thread terminated\n");
				}
			});
		}
	}

	uint32_t Auth::GetZeroBits(Utils::Cryptography::Token token, std::string publicKey)
	{
		std::string message = publicKey + token.ToString();
		std::string hash = Utils::Cryptography::SHA512::Compute(message, false);

		uint32_t bits = 0;

		for (unsigned int i = 0; i < hash.size(); ++i)
		{
			if (hash[i] == '\0')
			{
				bits += 8;
				continue;
			}

			uint8_t value = static_cast<uint8_t>(hash[i]);
			for (int j = 7; j >= 0; --j)
			{
				if ((value >> j) & 1)
				{
					return bits;
				}

				++bits;
			}
		}

		return bits;
	}

	void Auth::IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, std::string publicKey, uint32_t zeroBits, bool* cancel, uint64_t* count)
	{
		if (zeroBits > 512) return; // Not possible, due to SHA512

		if (computeToken < token)
		{
			computeToken = token;
		}

		// Check if we already have the desired security level
		uint32_t lastLevel = Auth::GetZeroBits(token, publicKey);
		uint32_t level = lastLevel;
		if (level >= zeroBits) return;

		do
		{
			++computeToken;
			if (count) ++(*count);
			level = Auth::GetZeroBits(computeToken, publicKey);

			// Store level if higher than the last one
			if (level >= lastLevel)
			{
				token = computeToken;
				lastLevel = level;
			}

			// Allow canceling that shit
			if (cancel && *cancel) return;
		}
		while (level < zeroBits);

		token = computeToken;
	}

	Auth::Auth()
	{
		Auth::TokenContainer.cancel = false;
		Auth::TokenContainer.generating = false;

		Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", "");

		Auth::LoadKey(true);

		// Register dvar
		Dvar::Register<int>("sv_securityLevel", 23, 0, 512, Game::dvar_flag::DVAR_FLAG_SERVERINFO, "Security level for GUID certificates (POW)");

		// Install registration hook
		Utils::Hook(0x6265F9, Auth::DirectConnectStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x41D3E3, Auth::SendConnectDataStub, HOOK_CALL).Install()->Quick();

		// Guid command
		Command::Add("guid", [] (Command::Params params)
		{
			Logger::Print("Your guid: %llX\n", Steam::SteamUser()->GetSteamID().Bits);
		});

		Command::Add("securityLevel", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				uint32_t level = Auth::GetZeroBits(Auth::GuidToken, Auth::GuidKey.GetPublicKey());
				Logger::Print("Your current security level is %d\n", level);
				Logger::Print("Your security token is: %s\n", Utils::String::DumpHex(Auth::GuidToken.ToString(), "").data());
				Logger::Print("Your computation token is: %s\n", Utils::String::DumpHex(Auth::ComputeToken.ToString(), "").data());

				Toast::Show("cardicon_locked", "^5Security Level", fmt::sprintf("Your security level is %d", level), 3000);
			}
			else
			{
				uint32_t level = static_cast<uint32_t>(atoi(params[1]));
				Auth::IncreaseSecurityLevel(level);
			}
		});

		UIScript::Add("security_increase_cancel", [] ()
		{
			Auth::TokenContainer.cancel = true;
			Logger::Print("Token incrementation process canceled!\n");
		});
	}

	Auth::~Auth()
	{
		Auth::TokenContainer.cancel = true;
		Auth::TokenContainer.generating = false;

		// Terminate thread
		if (Auth::TokenContainer.thread.joinable())
		{
			Auth::TokenContainer.thread.join();
		}

		Auth::StoreKey();
	}

	bool Auth::UnitTest()
	{
		bool success = true;

		printf("Testing logical token operators:\n");

		Utils::Cryptography::Token token1;
		Utils::Cryptography::Token token2;
		++token1, token2++; // Test incrementation operator

		printf("Operator == : ");
		if (token1 == token2 && !(++token1 == token2)) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		printf("Operator != : ");
		if (token1 != token2 && !(++token2 != token1)) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		printf("Operator >= : ");
		if (token1 >= token2 && ++token1 >= token2) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		printf("Operator >  : ");
		if (token1 > token2) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		printf("Operator <= : ");
		if (token1 <= ++token2 && token1 <= ++token2) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		printf("Operator <  : ");
		if (token1 < token2) printf("Success\n");
		else
		{
			printf("Error\n");
			success = false;
		}

		return success;
	}
}
