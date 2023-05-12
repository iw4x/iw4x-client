#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include <proto/auth.pb.h>

#include "Auth.hpp"
#include "Bans.hpp"
#include "Bots.hpp"
#include "Friends.hpp"
#include "Toast.hpp"

namespace Components
{
	Auth::TokenIncrementing Auth::TokenContainer;

	Utils::Cryptography::Token Auth::GuidToken;
	Utils::Cryptography::Token Auth::ComputeToken;
	Utils::Cryptography::ECC::Key Auth::GuidKey;

	std::vector<std::uint64_t> Auth::BannedUids =
	{
		0xf4d2c30b712ac6e3,
		0xf7e33c4081337fa3,
		0x6f5597f103cc50e9
	};

	bool Auth::HasAccessToReservedSlot;
	
	void Auth::Frame()
	{
		if (TokenContainer.generating)
		{
			static double mseconds = 0;
			static Utils::Time::Interval interval;

			if (interval.elapsed(500ms))
			{
				interval.update();

				int diff = Game::Sys_Milliseconds() - TokenContainer.startTime;
				double hashPMS = (TokenContainer.hashes * 1.0) / diff;
				double requiredHashes = std::pow(2, TokenContainer.targetLevel + 1) - TokenContainer.hashes;
				mseconds = requiredHashes / hashPMS;
				if (mseconds < 0) mseconds = 0;
			}

			Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", Utils::String::VA("Increasing security level from %d to %d (est. %s)",GetSecurityLevel(), TokenContainer.targetLevel, Utils::String::FormatTimeSpan(static_cast<int>(mseconds)).data()));
		}
		else if (TokenContainer.thread.joinable())
		{
			TokenContainer.thread.join();
			TokenContainer.generating = false;

			StoreKey();
			Logger::Debug("Security level is {}",GetSecurityLevel());
			Command::Execute("closemenu security_increase_popmenu", false);

			if (!TokenContainer.cancel)
			{
				if (TokenContainer.command.empty())
				{
					Game::ShowMessageBox(Utils::String::VA("Your new security level is %d", GetSecurityLevel()), "Success");
				}
				else
				{
					Toast::Show("cardicon_locked", "Success", Utils::String::VA("Your new security level is %d", GetSecurityLevel()), 5000);
					Command::Execute(TokenContainer.command, false);
				}
			}

			TokenContainer.cancel = false;
		}
	}

	void Auth::SendConnectDataStub(Game::netsrc_t sock, Game::netadr_t adr, const char* format, int len)
	{
		// Ensure our certificate is loaded
		Steam::SteamUser()->GetSteamID();
		if (!GuidKey.isValid())
		{
			Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Guid key is invalid!");
			return;
		}

		if (std::find(BannedUids.begin(), BannedUids.end(), Steam::SteamUser()->GetSteamID().bits) != BannedUids.end())
		{
			GenerateKey();
			Logger::Error(Game::ERR_SERVERDISCONNECT, "Your online profile is invalid. A new key has been generated.");
			return;
		}

		std::string connectString(format, len);
		Game::SV_Cmd_TokenizeString(connectString.data());

		Command::ServerParams params;

		if (params.size() < 3)
		{
			Game::SV_Cmd_EndTokenizedString();
			Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Command parsing error!");
			return;
		}

		Utils::InfoString infostr(params[2]);
		std::string challenge = infostr.get("challenge");

		if (challenge.empty())
		{
			Game::SV_Cmd_EndTokenizedString();
			Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Challenge parsing error!");
			return;
		}

		if (Steam::Enabled() && !Friends::IsInvisible() && !Dvar::Var("cl_anonymous").get<bool>() && Steam::Proxy::SteamUser_)
		{
			infostr.set("realsteamId", Utils::String::VA("%llX", Steam::Proxy::SteamUser_->GetSteamID().bits));
		}

		// Build new connect string
		connectString.clear();
		connectString.append(params[0]);
		connectString.append(" ");
		connectString.append(params[1]);
		connectString.append(" ");
		connectString.append("\"" + infostr.build() + "\"");

		Game::SV_Cmd_EndTokenizedString();

		Proto::Auth::Connect connectData;
		connectData.set_token(GuidToken.toString());
		connectData.set_publickey(GuidKey.getPublicKey());
		connectData.set_signature(Utils::Cryptography::ECC::SignMessage(GuidKey, challenge));
		connectData.set_infostring(connectString);

		Network::SendCommand(sock, adr, "connect", connectData.SerializeAsString());
	}

	void Auth::ParseConnectData(Game::msg_t* msg, Game::netadr_t* addr)
	{
		Network::Address address(addr);

		// Parse proto data
		Proto::Auth::Connect connectData;
		if (msg->cursize <= 12 || !connectData.ParseFromString(std::string(reinterpret_cast<char*>(&msg->data[12]), msg->cursize - 12)))
		{
			Network::Send(address, "error\nInvalid connect packet!");
			return;
		}

		// Simply connect, if we're in debug mode, we ignore all security checks
#ifndef DEBUG
		if (address.isLoopback())
#endif
		{
			if (!connectData.infostring().empty())
			{
				Game::SV_Cmd_EndTokenizedString();
				Game::SV_Cmd_TokenizeString(connectData.infostring().data());
				Game::SV_DirectConnect(*address.get());
			}
			else
			{
				Network::Send(address, "error\nInvalid infostring data!");
			}
		}
#ifndef DEBUG
		else
		{
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
			Command::ServerParams params;

			// Ensure there are enough params
			if (params.size() < 3)
			{
				Network::Send(address, "error\nInvalid connect string!");
				return;
			}

			// Parse the infostring
			Utils::InfoString infostr(params.get(2));

			// Read the required data
			const auto steamId = infostr.get("xuid");
			const auto challenge = infostr.get("challenge");

			if (steamId.empty() || challenge.empty())
			{
				Network::Send(address, "error\nInvalid connect data!");
				return;
			}

			// Parse the id
			const auto xuid = std::strtoull(steamId.data(), nullptr, 16);

			SteamID guid;
			guid.bits = xuid;

			if (Bans::IsBanned({guid, address.getIP()}))
			{
				Network::Send(address, "error\nEXE_ERR_BANNED_PERM");
				return;
			}

			if (std::find(BannedUids.begin(), BannedUids.end(), xuid) != BannedUids.end())
			{
				Network::Send(address, "error\nYour online profile is invalid. Delete your players folder and restart ^2IW4x^7.");
				return;
			}

			if (xuid != GetKeyHash(connectData.publickey()))
			{
				Network::Send(address, "error\nXUID doesn't match the certificate!");
				return;
			}

			// Verify the signature
			Utils::Cryptography::ECC::Key key;
			key.set(connectData.publickey());

			if (!key.isValid() || !Utils::Cryptography::ECC::VerifyMessage(key, challenge, connectData.signature()))
			{
				Network::Send(address, "error\nChallenge signature was invalid!");
				return;
			}

			// Verify the security level
			auto ourLevel = Dvar::Var("sv_securityLevel").get<unsigned int>();
			auto userLevel = GetZeroBits(connectData.token(), connectData.publickey());

			if (userLevel < ourLevel)
			{
				Network::Send(address, Utils::String::VA("error\nYour security level (%d) is lower than the server's security level (%d)", userLevel, ourLevel));
				return;
			}

			Logger::Debug("Verified XUID {:#X} ({}) from {}", xuid, userLevel, address.getString());
			Game::SV_DirectConnect(*address.get());
		}
#endif
	}

	__declspec(naked) void Auth::DirectConnectStub()
	{
		__asm
		{
			pushad
			lea eax, [esp + 20h]
			push eax
			push esi
			call ParseConnectData
			pop esi
			pop eax
			popad

			push 6265FEh
			retn
		}
	}

	char* Auth::Info_ValueForKeyStub(const char* s, const char* key)
	{
		auto* value = Game::Info_ValueForKey(s, key);

		HasAccessToReservedSlot = std::strcmp((*Game::sv_privatePassword)->current.string, value) == 0;

		// This stub runs right before the 'server is full check' so we can call this here
		Bots::SV_DirectConnect_Full_Check();

		return value;
	}

	__declspec(naked) void Auth::DirectConnectPrivateClientStub()
	{
		__asm
		{
			push eax

			mov al, HasAccessToReservedSlot
			test al, al

			pop eax

			je noAccess

			// Set the number of private clients to 0 if the client has the right password
			xor eax, eax
			jmp safeContinue

		noAccess:
			mov eax, dword ptr [edx + 0x10]

		safeContinue:
			// Game code skipped by hook
			add esp, 0xC

			push 0x460FB3
			ret
		}
	}

	unsigned __int64 Auth::GetKeyHash(const std::string& key)
	{
		std::string hash = Utils::Cryptography::SHA1::Compute(key);

		if (hash.size() >= 8)
		{
			return *reinterpret_cast<unsigned __int64*>(const_cast<char*>(hash.data()));
		}

		return 0;
	}

	unsigned __int64 Auth::GetKeyHash()
	{
		LoadKey();
		return GetKeyHash(GuidKey.getPublicKey());
	}

	void Auth::StoreKey()
	{
		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled() && GuidKey.isValid())
		{
			Proto::Auth::Certificate cert;
			cert.set_token(GuidToken.toString());
			cert.set_ctoken(ComputeToken.toString());
			cert.set_privatekey(GuidKey.serialize(PK_PRIVATE));

			Utils::IO::WriteFile("players/guid.dat", cert.SerializeAsString());
		}
	}

	void Auth::GenerateKey()
	{
		GuidToken.clear();
		ComputeToken.clear();
		GuidKey = Utils::Cryptography::ECC::GenerateKey(512);
		StoreKey();
	}

	void Auth::LoadKey(bool force)
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;
		if (!force && GuidKey.isValid()) return;

		Proto::Auth::Certificate cert;
		if (cert.ParseFromString(::Utils::IO::ReadFile("players/guid.dat")))
		{
			GuidKey.deserialize(cert.privatekey());
			GuidToken = cert.token();
			ComputeToken = cert.ctoken();
		}
		else
		{
			GuidKey.free();
		}

		if (!GuidKey.isValid())
		{
			Auth::GenerateKey();
		}
	}

	uint32_t Auth::GetSecurityLevel()
	{
		return GetZeroBits(GuidToken, GuidKey.getPublicKey());
	}

	void Auth::IncreaseSecurityLevel(uint32_t level, const std::string& command)
	{
		if (GetSecurityLevel() >= level) return;

		if (!TokenContainer.generating)
		{
			TokenContainer.cancel = false;
			TokenContainer.targetLevel = level;
			TokenContainer.command = command;

			// Open menu
			Command::Execute("openmenu security_increase_popmenu", true);

			// Start thread
			TokenContainer.thread = std::thread([&level]()
			{
				TokenContainer.generating = true;
				TokenContainer.hashes = 0;
				TokenContainer.startTime = Game::Sys_Milliseconds();
				IncrementToken(GuidToken, ComputeToken, GuidKey.getPublicKey(), TokenContainer.targetLevel, &TokenContainer.cancel, &TokenContainer.hashes);
				TokenContainer.generating = false;

				if (TokenContainer.cancel)
				{
					Logger::Print("Token incrementation thread terminated\n");
				}
			});
		}
	}

	uint32_t Auth::GetZeroBits(Utils::Cryptography::Token token, const std::string& publicKey)
	{
		std::string message = publicKey + token.toString();
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

	void Auth::IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, const std::string& publicKey, uint32_t zeroBits, bool* cancel, uint64_t* count)
	{
		if (zeroBits > 512) return; // Not possible, due to SHA512

		if (computeToken < token)
		{
			computeToken = token;
		}

		// Check if we already have the desired security level
		uint32_t lastLevel = GetZeroBits(token, publicKey);
		uint32_t level = lastLevel;
		if (level >= zeroBits) return;

		do
		{
			++computeToken;
			if (count) ++(*count);
			level = GetZeroBits(computeToken, publicKey);

			// Store level if higher than the last one
			if (level >= lastLevel)
			{
				token = computeToken;
				lastLevel = level;
			}

			// Allow canceling that shit
			if (cancel && *cancel) return;
		} while (level < zeroBits);

		token = computeToken;
	}

	Auth::Auth()
	{
		TokenContainer.cancel = false;
		TokenContainer.generating = false;

		HasAccessToReservedSlot = false;

		Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", "");

		// Load the key
		LoadKey(true);
		Steam::SteamUser()->GetSteamID();

		Scheduler::Loop(Frame, Scheduler::Pipeline::MAIN);

		// Register dvar
		Dvar::Register<int>("sv_securityLevel", 23, 0, 512, Game::DVAR_SERVERINFO, "Security level for GUID certificates (POW)");

		// Install registration hook
		Utils::Hook(0x6265F9, DirectConnectStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x460EF5, Info_ValueForKeyStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x460FAD, DirectConnectPrivateClientStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x460FAD + 5, 1);

		Utils::Hook(0x41D3E3, SendConnectDataStub, HOOK_CALL).install()->quick();

		// SteamIDs can only contain 31 bits of actual 'id' data.
		// The other 33 bits are steam internal data like universe and so on.
		// Using only 31 bits for fingerprints is pretty insecure.
		// The function below verifies the integrity steam's part of the SteamID.
		// Patching that check allows us to use 64 bit for fingerprints.
		Utils::Hook::Set<std::uint32_t>(0x4D0D60, 0xC301B0);

		// Guid command
		Command::Add("guid", []
		{
			Logger::Print("Your guid: {:#X}\n", Steam::SteamUser()->GetSteamID().bits);
		});

		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Command::Add("securityLevel", [](const Command::Params* params)
			{
				if (params->size() < 2)
				{
					const auto level = GetZeroBits(GuidToken, GuidKey.getPublicKey());
					Logger::Print("Your current security level is {}\n", level);
					Logger::Print("Your security token is: {}\n", Utils::String::DumpHex(GuidToken.toString(), ""));
					Logger::Print("Your computation token is: {}\n", Utils::String::DumpHex(ComputeToken.toString(), ""));

					Toast::Show("cardicon_locked", "^5Security Level", Utils::String::VA("Your security level is %d", level), 3000);
				}
				else
				{
					const auto level = std::strtoul(params->get(1), nullptr, 10);
					IncreaseSecurityLevel(level);
				}
			});
		}

		UIScript::Add("security_increase_cancel", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			TokenContainer.cancel = true;
			Logger::Print("Token incrementation process canceled!\n");
		});
	}

	Auth::~Auth()
	{
		StoreKey();
	}

	void Auth::preDestroy()
	{
		TokenContainer.cancel = true;
		TokenContainer.generating = false;

		if (TokenContainer.thread.joinable())
		{
			TokenContainer.thread.join();
		}
	}

	bool Auth::unitTest()
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
