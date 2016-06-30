#include "STDInclude.hpp"

namespace Components
{
	Auth::AuthInfo Auth::ClientAuthInfo[18];
	Auth::TokenIncrementing Auth::TokenContainer;

	Utils::Cryptography::Token Auth::GuidToken;
	Utils::Cryptography::Token Auth::ComputeToken;
	Utils::Cryptography::ECC::Key Auth::GuidKey;

	void Auth::Frame()
	{
#ifndef DEBUG
		for (int i = 0; i < *Game::svs_numclients; i++)
		{
			Game::client_t* client = &Game::svs_clients[i];
			Auth::AuthInfo* info = &Auth::ClientAuthInfo[i];

			// State must be 5 or greater here, as otherwise the client will crash when being kicked.
			// That's due to the hunk being freed by that time, but it hasn't been reallocated, therefore all future allocations will cause a crash.
			// Additionally, the game won't catch the errors and simply lose the connection, so we even have to add a delay to send the data.

			// Not sure if that's potentially unsafe, though.
			// Players faking their GUID will be connected for 5 seconds, which allows them to fuck up everything.
			// I think we have to perform the verification when clients are still in state 3, but for now it works.

			// I think we even have to lock the client into state 3 until the verification is done.
			// Intercepting the entire connection process to perform the authentication within state 3 solely is necessary, due to having a timeout. 
			// Not sending a response might allow the player to connect for a few seconds (<= 5) until the timeout is reached.
			if (client->state >= 5)
			{
				if (info->state == Auth::STATE_NEGOTIATING && (Game::Sys_Milliseconds() - info->time) > 1000 * 5)
				{
					info->state = Auth::STATE_INVALID;
					info->time = Game::Sys_Milliseconds();
					Game::SV_KickClientError(client, "XUID verification timed out!");
				}
				else if (info->state == Auth::STATE_UNKNOWN && info->time && (Game::Sys_Milliseconds() - info->time) > 1000 * 5) // Wait 5 seconds (error delay)
				{
					if ((client->steamid & 0xFFFFFFFF00000000) != 0x110000100000000)
					{
						info->state = Auth::STATE_INVALID;
						info->time = Game::Sys_Milliseconds();
						Game::SV_KickClientError(client, "Your XUID is invalid!");
					}
					else
					{
						Logger::Print("Sending XUID authentication request to %s\n", Network::Address(client->addr).GetCString());

						info->state = Auth::STATE_NEGOTIATING;
						info->time = Game::Sys_Milliseconds();
						info->challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
						Network::SendCommand(client->addr, "xuidAuthReq", info->challenge);
					}
				}
				else if (info->state == Auth::STATE_UNKNOWN && !info->time)
				{
					info->time = Game::Sys_Milliseconds();
				}
			}
		}
#endif

		if (Auth::TokenContainer.generating)
		{
			static int lastCalc = 0;
			static double mseconds = 0;

			if (!lastCalc || (Game::Sys_Milliseconds() - lastCalc) > 500)
			{
				lastCalc = Game::Sys_Milliseconds();

				int diff = Game::Sys_Milliseconds() - Auth::TokenContainer.startTime;
				double hashPMS = (Auth::TokenContainer.hashes * 1.0) / diff;
				double requiredHashes = std::pow(2, Auth::TokenContainer.targetLevel + 1) - Auth::TokenContainer.hashes;
				mseconds = requiredHashes / hashPMS;
				if (mseconds < 0) mseconds = 0;
			}

			Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", Utils::VA("Increasing security level from %d to %d (est. %s)", Auth::GetSecurityLevel(), Auth::TokenContainer.targetLevel, Utils::FormatTimeSpan(static_cast<int>(mseconds)).data()));
		}
		else if(Auth::TokenContainer.thread.joinable())
		{
			Auth::TokenContainer.thread.join();
			Auth::TokenContainer.generating = false;

			Auth::StoreKey();
			Logger::Print("Security level is %d\n", Auth::GetSecurityLevel());
			Command::Execute("closemenu security_increase_popmenu", false);

			if (!Auth::TokenContainer.cancel)
			{
				if (Auth::TokenContainer.command.empty())
				{
					Game::MessageBox(Utils::VA("Your new security level is %d", Auth::GetSecurityLevel()), "Success");
				}
				else
				{
					Command::Execute(Auth::TokenContainer.command, false);
				}
			}

			Auth::TokenContainer.cancel = false;
		}
	}

	void Auth::RegisterClient(int clientNum)
	{
		if (clientNum >= 18) return;

		Network::Address address(Game::svs_clients[clientNum].addr);

		if (address.GetType() == Game::netadrtype_t::NA_BOT)
		{
			Auth::ClientAuthInfo[clientNum].state = Auth::STATE_VALID;
		}
		else
		{
			Logger::Print("Registering client %s\n", address.GetCString());
			Auth::ClientAuthInfo[clientNum].time = 0;
			Auth::ClientAuthInfo[clientNum].state = Auth::STATE_UNKNOWN;
		}
	}

	void __declspec(naked) Auth::RegisterClientStub()
	{
		__asm
		{
			push esi
			call Auth::RegisterClient
			pop esi

			imul esi, 366Ch
			mov eax, 478A18h
			jmp eax
		}
	}

	unsigned int Auth::GetKeyHash()
	{
		Auth::LoadKey();
		return (Utils::Cryptography::JenkinsOneAtATime::Compute(Auth::GuidKey.GetPublicKey()));
	}

	void Auth::StoreKey()
	{
		if (!Dedicated::IsDedicated() && !ZoneBuilder::IsEnabled())
		{
			Proto::Auth::Certificate cert;
			cert.set_token(Auth::GuidToken.ToString());
			cert.set_ctoken(Auth::ComputeToken.ToString());
			cert.set_privatekey(Auth::GuidKey.Export(PK_PRIVATE));

			Utils::WriteFile("players/guid.dat", cert.SerializeAsString());
		}
	}

	void Auth::LoadKey(bool force)
	{
		if (Dedicated::IsDedicated() || ZoneBuilder::IsEnabled()) return;
		if (!force && Auth::GuidKey.IsValid()) return;

		Proto::Auth::Certificate cert;
		if (cert.ParseFromString(::Utils::ReadFile("players/guid.dat")))
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

		// Only clients receive the auth request
		if (!Dedicated::IsDedicated()) 
		{
			Network::Handle("xuidAuthReq", [] (Network::Address address, std::string data)
			{
				Logger::Print("Received XUID authentication request from %s\n", address.GetCString());

				// Only accept requests from the server we're connected to
				if (address != *Game::connectedHost) return;

				// Ensure our certificate is loaded
				Steam::SteamUser()->GetSteamID();
				if (!Auth::GuidKey.IsValid()) return;

				Proto::Auth::Response response;
				response.set_token(Auth::GuidToken.ToString());
				response.set_publickey(Auth::GuidKey.GetPublicKey());
				response.set_signature(Utils::Cryptography::ECC::SignMessage(Auth::GuidKey, data));

				Network::SendCommand(address, "xuidAuthResp", response.SerializeAsString());
			});
		}

		Network::Handle("xuidAuthResp", [] (Network::Address address, std::string data)
		{
			Logger::Print("Received XUID authentication response from %s\n", address.GetCString());

			for (int i = 0; i < *Game::svs_numclients; i++)
			{
				Game::client_t* client = &Game::svs_clients[i];
				Auth::AuthInfo* info = &Auth::ClientAuthInfo[i];

				if (client->state >= 3 && address == client->addr && info->state == Auth::STATE_NEGOTIATING)
				{
					Proto::Auth::Response response;
					unsigned int id = static_cast<unsigned int>(~0x110000100000000 & client->steamid);

					// Check if response is valid
					if (!response.ParseFromString(data) || response.signature().empty() || response.publickey().empty() || response.token().empty())
					{
						info->state = Auth::STATE_INVALID;
						Game::SV_KickClientError(client, "XUID authentication response was invalid!");
					}

					// Check if guid matches the certificate
					else if (id != (Utils::Cryptography::JenkinsOneAtATime::Compute(response.publickey()) & ~0x80000000))
					{
						info->state = Auth::STATE_INVALID;
						Game::SV_KickClientError(client, "XUID doesn't match the certificate!");
					}

					// Verify GUID using the signature and certificate
					else
					{
						info->publicKey.Set(response.publickey());

						if (Utils::Cryptography::ECC::VerifyMessage(info->publicKey, info->challenge, response.signature()))
						{
							uint32_t ourLevel = static_cast<uint32_t>(Dvar::Var("sv_securityLevel").Get<int>());
							uint32_t userLevel = Auth::GetZeroBits(response.token(), response.publickey());

							if (userLevel >= ourLevel)
							{
								info->state = Auth::STATE_VALID;
								Logger::Print("Verified XUID %llX (%d) from %s\n", client->steamid, userLevel, address.GetCString());
							}
							else
							{
								info->state = Auth::STATE_INVALID;
								Game::SV_KickClientError(client, Utils::VA("Your security level (%d) is lower than the server's security level (%d)", userLevel, ourLevel));
							}
						}
						else
						{
							info->state = Auth::STATE_INVALID;
							Game::SV_KickClientError(client, "Challenge signature was invalid!");
						}
					}

					break;
				}
			}
		});

		// Install frame handlers
		QuickPatch::OnFrame(Auth::Frame);

		// Register dvar
		Dvar::Register<int>("sv_securityLevel", 23, 0, 512, Game::dvar_flag::DVAR_FLAG_SERVERINFO, "Security level for GUID certificates (POW)");

#ifndef DEBUG
		// Install registration hook
		Utils::Hook(0x478A12, Auth::RegisterClientStub, HOOK_JUMP).Install()->Quick();
#endif

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
				Logger::Print("Your security token is: %s\n", Utils::DumpHex(Auth::GuidToken.ToString(), "").data());
				Logger::Print("Your computation token is: %s\n", Utils::DumpHex(Auth::ComputeToken.ToString(), "").data());

				Toast::Show("cardicon_locked", "^5Security Level", Utils::VA("Your security level is %d", level), 3000);
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
