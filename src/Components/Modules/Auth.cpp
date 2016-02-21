#include "STDInclude.hpp"

namespace Components
{
	Auth::AuthInfo Auth::ClientAuthInfo[18];

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
				if (info->state == Auth::STATE_NEGOTIATING && (Game::Com_Milliseconds() - info->time) > 1000 * 3)
				{
					info->state = Auth::STATE_INVALID;
					info->time = Game::Com_Milliseconds();
					Game::SV_KickClientError(client, "XUID verification timed out!");
				}
				else if (info->state == Auth::STATE_UNKNOWN && info->time && (Game::Com_Milliseconds() - info->time) > 1000 * 2) // Wait 2 seconds (error delay)
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

	Auth::Auth()
	{
		// Only clients receive the auth request
		if (!Dedicated::IsDedicated()) 
		{
			Network::Handle("xuidAuthReq", [] (Network::Address address, std::string data)
			{
				Logger::Print("Received XUID authentication request from %s\n", address.GetString());

				// Only accept requests from the server we're connected to
				if (address != *Game::connectedHost) return;
				if (!Steam::User::GuidKey.IsValid()) return;

				Proto::Auth::Response response;
				response.set_publickey(Steam::User::GuidKey.GetPublicKey());
				response.set_signature(Utils::Cryptography::ECDSA::SignMessage(Steam::User::GuidKey, data));

				Network::SendCommand(address, "xuidAuthResp", response.SerializeAsString());
			});
		}

		Network::Handle("xuidAuthResp", [] (Network::Address address, std::string data)
		{
			Logger::Print("Received XUID authentication response from %s\n", address.GetString());

			Proto::Auth::Response response;
			response.ParseFromString(data);

			if (response.signature().empty()) return;
			if (response.publickey().empty()) return;

			for (int i = 0; i < *Game::svs_numclients; i++)
			{
				Game::client_t* client = &Game::svs_clients[i];
				Auth::AuthInfo* info = &Auth::ClientAuthInfo[i];

				if (client->state >= 3 && address == client->adr && info->state == Auth::STATE_NEGOTIATING)
				{
					unsigned int id = static_cast<unsigned int>(~0x110000100000000 & client->steamid);

					// Check if guid matches the certificate
					if (id != (Utils::OneAtATime(response.publickey().data(), response.publickey().size()) & ~0x80000000))
					{
						info->state = Auth::STATE_INVALID;
						Game::SV_KickClientError(client, "XUID doesn't match the certificate!");
					}
					else
					{
						info->publicKey.Set(response.publickey());

						if (Utils::Cryptography::ECDSA::VerifyMessage(info->publicKey, info->challenge, response.signature()))
						{
							info->state = Auth::STATE_VALID;
							Logger::Print("Verified XUID %llX from %s\n", client->steamid, address.GetString());
						}
						else
						{
							info->state = Auth::STATE_INVALID;
							Game::SV_KickClientError(client, "Challenge signature was invalid!");
						}
					}
				}
			}
		});

		// Install frame handlers
		Dedicated::OnFrame(Auth::Frame);
		Renderer::OnFrame(Auth::Frame);

		// Install registration hook
		Utils::Hook(0x478A12, Auth::RegisterClientStub, HOOK_JUMP).Install()->Quick();
	}

	Auth::~Auth()
	{

	}
}
