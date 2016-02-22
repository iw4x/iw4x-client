#include "STDInclude.hpp"

namespace Components
{
	Auth::AuthInfo Auth::ClientAuthInfo[18];

	Utils::Cryptography::Token Auth::GuidToken;
	Utils::Cryptography::ECDSA::Key Auth::GuidKey;

	void Auth::Frame()
	{
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
				if (info->state == Auth::STATE_NEGOTIATING && (Game::Com_Milliseconds() - info->time) > 1000 * 5)
				{
					info->state = Auth::STATE_INVALID;
					info->time = Game::Com_Milliseconds();
					Game::SV_KickClientError(client, "XUID verification timed out!");
				}
				else if (info->state == Auth::STATE_UNKNOWN && info->time && (Game::Com_Milliseconds() - info->time) > 1000 * 5) // Wait 5 seconds (error delay)
				{
					Logger::Print("Sending XUID authentication request to %s\n", Network::Address(client->adr).GetString());

					info->state = Auth::STATE_NEGOTIATING;
					info->time = Game::Com_Milliseconds();
					info->challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
					Network::SendCommand(client->adr, "xuidAuthReq", info->challenge);
				}
				else if (info->state == Auth::STATE_UNKNOWN && !info->time)
				{
					info->time = Game::Com_Milliseconds();
				}
			}
		}
	}

	void Auth::RegisterClient(int clientNum)
	{
		if (clientNum >= 18) return;

		Network::Address address(Game::svs_clients[clientNum].adr);

		if (address.GetType() == Game::netadrtype_t::NA_BOT)
		{
			Auth::ClientAuthInfo[clientNum].state = Auth::STATE_VALID;
		}
		else
		{
			Logger::Print("Registering client %s\n", address.GetString());
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
		std::string key = Auth::GuidKey.GetPublicKey();
		return (Utils::OneAtATime(key.data(), key.size()));
	}

	void Auth::StoreKey()
	{
		Proto::Auth::Certificate cert;
		cert.set_token(Auth::GuidToken.ToString());
		cert.set_privatekey(Auth::GuidKey.Export(PK_PRIVATE));

		Utils::WriteFile("players/guid.dat", cert.SerializeAsString());
	}

	void Auth::LoadKey(bool force)
	{
		if (!force && Auth::GuidKey.IsValid()) return;

		Proto::Auth::Certificate cert;
		if (cert.ParseFromString(::Utils::ReadFile("players/guid.dat")))
		{
			Auth::GuidKey.Import(cert.privatekey(), PK_PRIVATE);
			Auth::GuidToken = cert.token();
		}
		else
		{
			Auth::GuidKey.Free();
		}

		if (!Auth::GuidKey.IsValid())
		{
			Auth::GuidToken.Clear();
			Auth::GuidKey = Utils::Cryptography::ECDSA::GenerateKey(512);
			Auth::StoreKey();
		}
	}

	uint32_t Auth::GetSecurityLevel()
	{
		return Auth::GetZeroBits(Auth::GuidToken, Auth::GuidKey.GetPublicKey());
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

				bits++;
			}
		}

		return bits;
	}

	void Auth::IncrementToken(Utils::Cryptography::Token& token, std::string publicKey, uint32_t zeroBits)
	{
		if (zeroBits > 512) return; // Not possible, due to SHA512

		Utils::Cryptography::Token tempToken(token);

		while (Auth::GetZeroBits(tempToken, publicKey) < zeroBits)
		{
			++tempToken;
		}

		token = tempToken;
	}

	Auth::Auth()
	{
		Auth::LoadKey(true);

		// Only clients receive the auth request
		if (!Dedicated::IsDedicated()) 
		{
			Network::Handle("xuidAuthReq", [] (Network::Address address, std::string data)
			{
				Logger::Print("Received XUID authentication request from %s\n", address.GetString());

				// Only accept requests from the server we're connected to
				if (address != *Game::connectedHost) return;

				// Ensure our certificate is loaded
				Steam::SteamUser()->GetSteamID();
				if (!Auth::GuidKey.IsValid()) return;

				Proto::Auth::Response response;
				response.set_token(Auth::GuidToken.ToString());
				response.set_publickey(Auth::GuidKey.GetPublicKey());
				response.set_signature(Utils::Cryptography::ECDSA::SignMessage(Auth::GuidKey, data));

				Network::SendCommand(address, "xuidAuthResp", response.SerializeAsString());
			});
		}

		Network::Handle("xuidAuthResp", [] (Network::Address address, std::string data)
		{
			Logger::Print("Received XUID authentication response from %s\n", address.GetString());

			for (int i = 0; i < *Game::svs_numclients; i++)
			{
				Game::client_t* client = &Game::svs_clients[i];
				Auth::AuthInfo* info = &Auth::ClientAuthInfo[i];

				if (client->state >= 3 && address == client->adr && info->state == Auth::STATE_NEGOTIATING)
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
					else if (id != (Utils::OneAtATime(response.publickey().data(), response.publickey().size()) & ~0x80000000))
					{
						info->state = Auth::STATE_INVALID;
						Game::SV_KickClientError(client, "XUID doesn't match the certificate!");
					}

					// Verify GUID using the signature and certificate
					else
					{
						info->publicKey.Set(response.publickey());

						if (Utils::Cryptography::ECDSA::VerifyMessage(info->publicKey, info->challenge, response.signature()))
						{
							uint32_t ourLevel = static_cast<uint32_t>(Dvar::Var("sv_securityLevel").Get<int>());
							uint32_t userLevel = Auth::GetZeroBits(response.token(), response.publickey());

							if (userLevel >= ourLevel)
							{
								info->state = Auth::STATE_VALID;
								Logger::Print("Verified XUID %llX from %s\n", client->steamid, address.GetString());
							}
							else
							{
								info->state = Auth::STATE_INVALID;
								Game::SV_KickClientError(client, Utils::VA("Your security level (%d) does not match the server's security level (%d)", userLevel, ourLevel));
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
		Dedicated::OnFrame(Auth::Frame);
		Renderer::OnFrame(Auth::Frame);

		// Register dvar
		Dvar::Register<int>("sv_securityLevel", 20, 0, 512, Game::dvar_flag::DVAR_FLAG_SERVERINFO, "Security level for GUID certificates (POW)");

		// Install registration hook
		Utils::Hook(0x478A12, Auth::RegisterClientStub, HOOK_JUMP).Install()->Quick();

		// Guid command
		Command::Add("guid", [] (Command::Params params)
		{
			Logger::Print("Your guid: %llX\n", Steam::SteamUser()->GetSteamID().Bits);
		});

		Command::Add("securityLevel", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				Logger::Print("Your current security level is %d\n", Auth::GetZeroBits(Auth::GuidToken, Auth::GuidKey.GetPublicKey()));
			}
			else
			{
				uint32_t level = static_cast<uint32_t>(atoi(params[1]));
				Logger::Print("Incrementing security level from %d to %d...\n", Auth::GetSecurityLevel(), level);
				Auth::IncrementToken(Auth::GuidToken, Auth::GuidKey.GetPublicKey(), level);
				Logger::Print("Your new security level is %d\n", Auth::GetSecurityLevel());
			}
		});
	}

	Auth::~Auth()
	{
		Auth::StoreKey();
	}

	bool Auth::UnitTest()
	{
// 		Utils::Cryptography::Token t;
// 		auto _key = Utils::Cryptography::ECDSA::GenerateKey(512);
// 		Auth::IncrementToken(t, _key.GetPublicKey(), 22);
// 
// 		Utils::WriteFile("pubKey.dat", _key.GetPublicKey());
// 		Utils::WriteFile("token.dat", t.ToString());

/*
		Utils::Cryptography::Token t;
		for (int i = 0; i < 1'000'000; ++i, ++t)
		{
			printf("%s\n", Utils::DumpHex(t.ToString()).data());
		}
*/
// 		auto testSecurityLevel = [](size_t level, std::string key)
// 		{
// 			auto startTime = std::chrono::high_resolution_clock::now();
// 			Utils::Cryptography::Token t;
// 			Auth::IncrementToken(t, key, level);
// 			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
// 		};
// 
// 		for (int j = 10; j < 30; ++j)
// 		{
// 			printf("\nTesting security level %i:\n", j);
// 
// 			std::vector<long long> times;
// 
// 			for (int i = 0; i < 10; ++i)
// 			{
// 				auto key = Utils::Cryptography::ECDSA::GenerateKey(512);
// 
// 				auto time = testSecurityLevel(j, key.GetPublicKey());
// 				times.push_back(time);
// 				printf("\t%i: %llims\n", i, time);
// 			}
// 
// 			long long average = 0;
// 
// 			for (auto time : times)
// 			{
// 				average += time;
// 			}
// 
// 			average /= times.size();
// 
// 			printf("\n  Average: %llims\n", average);
// 		}

		return true;
	}
}
