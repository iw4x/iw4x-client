#include <STDInclude.hpp>
#include <proto/rcon.pb.h>

#include "Events.hpp"
#include "RCon.hpp"
#include "Party.hpp"

namespace Components
{
	std::unordered_map<std::uint32_t, int> RCon::RateLimit;

	std::vector<std::size_t> RCon::RConAddresses;

	std::string RCon::Password;

	Dvar::Var RCon::RConPassword;
	Dvar::Var RCon::RConLogRequests;
	Dvar::Var RCon::RConTimeout;

	std::string RCon::RConOutputBuffer;

	void RCon::AddCommands()
	{
		Command::Add("rcon", [](const Command::Params* params)
		{
			if (params->size() < 2) return;

			const auto* operation = params->get(1);
			if (std::strcmp(operation, "login") == 0)
			{
				if (params->size() < 3) return;
				Password = params->get(2);
				return;
			}

			if (std::strcmp(operation, "logout") == 0)
			{
				Password.clear();
				return;
			}

			auto* addr = reinterpret_cast<Game::netadr_t*>(0xA5EA44);
			if (Password.empty())
			{
				Logger::Print("You need to be logged in and connected to a server!\n");
			}

			Network::Address target(addr);
			if (!target.isValid() || target.getIP().full == 0)
			{
				target = Party::Target();
			}

			if (target.isValid())
			{
				Network::SendCommand(target, "rcon", Password + " " + params->join(1));
				return;
			}

			Logger::Print("You are connected to an invalid server\n");
		});

		Command::Add("rconSafe", [](const Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("Usage: {} <command>\n", params->get(0));
				return;
			}

			const auto command = params->join(1);

			auto* addr = reinterpret_cast<Game::netadr_t*>(0xA5EA44);
			Network::Address target(addr);
			if (!target.isValid() || target.getIP().full == 0)
			{
				target = Party::Target();
			}

			if (!target.isValid())
			{
				Logger::Print("You are connected to an invalid server\n");
				return;
			}

			const auto& key = CryptoKeyRSA::GetPrivateKey();
			const auto signature = Utils::Cryptography::RSA::SignMessage(key, command);

			Proto::RCon::Command directive;
			directive.set_command(command);
			directive.set_signature(signature);

			Network::SendCommand(target, "rconSafe", directive.SerializeAsString());
		});

		Command::AddSV("RconWhitelistAdd", [](const Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("Usage: %s <ip-address>\n", params->get(0));
				return;
			}

			Network::Address address(params->get(1));
			const auto hash = std::hash<std::uint32_t>()(*reinterpret_cast<const std::uint32_t*>(&address.getIP().bytes[0]));

			if (address.isValid() && std::ranges::find(RConAddresses, hash) == RConAddresses.end())
			{
				RConAddresses.push_back(hash);
			}
		});
	}

	bool RCon::IsRateLimitCheckDisabled()
	{
		static std::optional<bool> flag;
		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("disable-rate-limit-check"));
		}
		return flag.value();
	}

	bool RCon::RateLimitCheck(const Network::Address& address, const int time)
	{
		const auto ip = address.getIP();
		const auto lastTime = RateLimit[ip.full];

		if (lastTime && (time - lastTime) < RConTimeout.get<int>())
		{
			return false; // Flooding
		}

		RateLimit[ip.full] = time;
		return true;
	}

	void RCon::RateLimitCleanup(const int time)
	{
		for (auto i = RateLimit.begin(); i != RateLimit.end();)
		{
			// No longer at risk of flooding, remove
			if ((time - i->second) > RConTimeout.get<int>())
			{
				i = RateLimit.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void RCon::RConExecutor(const Network::Address& address, std::string data)
	{
		Utils::String::Trim(data);

		const auto pos = data.find_first_of(' ');
		if (pos == std::string::npos)
		{
			Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
			Logger::Print(Game::CON_CHANNEL_NETWORK, "Invalid RCon request from {}\n", Network::AdrToString(address));
			return;
		}

		auto password = data.substr(0, pos);
		auto command = data.substr(pos + 1);

		// B3 sends the password inside quotes :S
		if (!password.empty() && password[0] == '"' && password.back() == '"')
		{
			password.pop_back();
			password.erase(password.begin());
		}

		const auto svPassword = RConPassword.get<std::string>();
		if (svPassword.empty())
		{
			Logger::Print(Game::CON_CHANNEL_NETWORK, "RCon request from {} dropped. No password set!\n", address.getString());
			return;
		}

		if (svPassword != password)
		{
			Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
			Logger::Print(Game::CON_CHANNEL_NETWORK, "Invalid RCon password sent from {}\n", Network::AdrToString(address));
			return;
		}

		RConOutputBuffer.clear();

#ifndef _DEBUG
		if (RConLogRequests.get<bool>())
#endif
		{
			Logger::Print(Game::CON_CHANNEL_NETWORK, "Executing RCon request from {}: {}\n", Network::AdrToString(address), command);
		}

		Logger::PipeOutput([](const std::string& output)
		{
			RConOutputBuffer.append(output);
		});

		Command::Execute(command, true);

		Logger::PipeOutput(nullptr);

		Network::SendCommand(address, "print", RConOutputBuffer);
		RConOutputBuffer.clear();
	}

	void RCon::RConSafeExecutor(const Network::Address& address, std::string command)
	{
		RConOutputBuffer.clear();

#ifndef _DEBUG
		if (RConLogRequests.get<bool>())
#endif
		{
			Logger::Print(Game::CON_CHANNEL_NETWORK, "Executing Safe RCon request from {}: {}\n", address.getString(), command);
		}

		Logger::PipeOutput([](const std::string& output)
		{
			RConOutputBuffer.append(output);
		});

		Command::Execute(command, true);

		Logger::PipeOutput(nullptr);

		Network::SendCommand(address, "print", RConOutputBuffer);
		RConOutputBuffer.clear();
	}

	RCon::RCon()
	{
		Events::OnSVInit(AddCommands);

		if (!Dedicated::IsEnabled())
		{
			return;
		}

		Events::OnDvarInit([]
		{
			RConPassword =  Dvar::Register<const char*>("rcon_password", "", Game::DVAR_NONE, "The password for rcon");
			RConLogRequests = Dvar::Register<bool>("rcon_log_requests", false, Game::DVAR_NONE, "Print remote commands in log");
			RConTimeout = Dvar::Register<int>("rcon_timeout", 500, 100, 10000, Game::DVAR_NONE, "");
		});

		Network::OnClientPacket("rcon", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			const auto hash = std::hash<std::uint32_t>()(*reinterpret_cast<const std::uint32_t*>(&address.getIP().bytes[0]));
			if (!RConAddresses.empty() && std::ranges::find(RConAddresses, hash) == RConAddresses.end())
			{
				return;
			}

			const auto time = Game::Sys_Milliseconds();
			if (!IsRateLimitCheckDisabled() && !RateLimitCheck(address, time))
			{
				Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
				return;
			}

			RateLimitCleanup(time);

			auto rconData = data;
			Scheduler::Once([address, s = std::move(rconData)]
			{
				RConExecutor(address, s);
			}, Scheduler::Pipeline::MAIN);
		});

		Network::OnClientPacket("rconSafe", [](const Network::Address& address, [[maybe_unused]] const std::string& data) -> void
		{
			const auto hash = std::hash<std::uint32_t>()(*reinterpret_cast<const std::uint32_t*>(&address.getIP().bytes[0]));
			if (!RConAddresses.empty() && std::ranges::find(RConAddresses, hash) == RConAddresses.end())
			{
				return;
			}

			const auto time = Game::Sys_Milliseconds();
			if (!IsRateLimitCheckDisabled() && !RateLimitCheck(address, time))
			{
				Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
				return;
			}

			RateLimitCleanup(time);

			if (!CryptoKeyRSA::HasPublicKey())
			{
				return;
			}

			auto& key = CryptoKeyRSA::GetPublicKey();
			if (!key.isValid())
			{
				Logger::PrintError(Game::CON_CHANNEL_NETWORK, "RSA public key is invalid\n");
				return;
			}

			Proto::RCon::Command directive;
			if (!directive.ParseFromString(data))
			{
				Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
				Logger::PrintError(Game::CON_CHANNEL_NETWORK, "Unable to parse secure command from {}\n", Network::AdrToString(address));
				return;
			}

			if (!Utils::Cryptography::RSA::VerifyMessage(key, directive.command(), directive.signature()))
			{
				Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
				Logger::PrintError(Game::CON_CHANNEL_NETWORK, "RSA signature verification failed for message from {}\n", Network::AdrToString(address));
				return;
			}

			std::string rconData = directive.command();
			Scheduler::Once([address, s = std::move(rconData)]
			{
				RConSafeExecutor(address, s);
			}, Scheduler::Pipeline::MAIN);
		});
	}

	Utils::Cryptography::RSA::Key RCon::CryptoKeyRSA::LoadPublicKey()
	{
		Utils::Cryptography::RSA::Key key;
		std::string data;

		if (!Utils::IO::ReadFile("./rsa-public.key", &data))
		{
			return key;
		}

		key.set(data);
		return key;
	}

	Utils::Cryptography::RSA::Key RCon::CryptoKeyRSA::GetPublicKeyInternal()
	{
		auto key = LoadPublicKey();
		return key;
	}

	Utils::Cryptography::RSA::Key& RCon::CryptoKeyRSA::GetPublicKey()
	{
		static auto key = GetPublicKeyInternal();
		return key;
	}

	bool RCon::CryptoKeyRSA::LoadPrivateKey(Utils::Cryptography::RSA::Key& key)
	{
		std::string data;
		if (!Utils::IO::ReadFile("./rsa-private.key", &data))
		{
			return false;
		}

		key.set(data);
		return key.isValid();
	}

	Utils::Cryptography::RSA::Key RCon::CryptoKeyRSA::GenerateKeyPair()
	{
		auto key = Utils::Cryptography::RSA::GenerateKey(4096);
		if (!key.isValid())
		{
			throw std::runtime_error("Failed to generate RSA key!");
		}

		if (!Utils::IO::WriteFile("./rsa-private.key", key.serialize(PK_PRIVATE)))
		{
			throw std::runtime_error("Failed to write RSA private key!");
		}

		if (!Utils::IO::WriteFile("./rsa-public.key", key.serialize(PK_PUBLIC)))
		{
			throw std::runtime_error("Failed to write RSA public key!");
		}

		return key;
	}

	Utils::Cryptography::RSA::Key RCon::CryptoKeyRSA::LoadOrGeneratePrivateKey()
	{
		Utils::Cryptography::RSA::Key key;
		if (LoadPrivateKey(key))
		{
			return key;
		}

		return GenerateKeyPair();
	}

	Utils::Cryptography::RSA::Key RCon::CryptoKeyRSA::GetPrivateKeyInternal()
	{
		auto key = LoadOrGeneratePrivateKey();
		return key;
	}

	Utils::Cryptography::RSA::Key& RCon::CryptoKeyRSA::GetPrivateKey()
	{
		static auto key = GetPrivateKeyInternal();
		return key;
	}

	bool RCon::CryptoKeyRSA::HasPublicKey()
	{
		return Utils::IO::FileExists("./rsa-public.key");
	}
}
