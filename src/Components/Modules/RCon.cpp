#include "STDInclude.hpp"

namespace Components
{
	RCon::Container RCon::BackdoorContainer;
	Utils::Cryptography::ECDSA::Key RCon::BackdoorKey;

	std::string RCon::Password;

	RCon::RCon()
	{
		// TODO: Maybe execute that for clients as well, when we use triangular natting.
		if (!Dedicated::IsDedicated()) return;

		// TODO: Load public key
		RCon::BackdoorKey.Set("");

		RCon::BackdoorContainer.timestamp = 0;

		Network::Handle("rconRequest", [] (Network::Address address, std::string data)
		{
			RCon::BackdoorContainer.address = address;
			RCon::BackdoorContainer.challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
			RCon::BackdoorContainer.timestamp = Game::Com_Milliseconds();

			Network::SendCommand(address, "rconAuthorization", RCon::BackdoorContainer.challenge);
		});

		Network::Handle("rconExecute", [] (Network::Address address, std::string data)
		{
			if (address != RCon::BackdoorContainer.address) return; // Invalid IP
			if (!RCon::BackdoorContainer.timestamp || (Game::Com_Milliseconds() - RCon::BackdoorContainer.timestamp) > (1000 * 10)) return; // Timeout
			RCon::BackdoorContainer.timestamp = 0;

			Proto::RCon::Command command;
			command.ParseFromString(data);

			if (Utils::Cryptography::ECDSA::VerifyMessage(RCon::BackdoorKey, RCon::BackdoorContainer.challenge, command.signature()))
			{
				RCon::BackdoorContainer.output.clear();
				Logger::PipeOutput([] (std::string output)
				{
					RCon::BackdoorContainer.output.append(output);
				});

				Command::Execute(command.commands(), true);

				Logger::PipeOutput(nullptr);

				Network::SendCommand(address, "rconResponse", RCon::BackdoorContainer.output);
				RCon::BackdoorContainer.output.clear();
			}
		});

		Command::Add("rcon", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			std::string operation = params[1];
			if (operation == "login")
			{
				if (params.Length() < 3) return;
				RCon::Password = params[2];
			}
			else if (operation == "logout")
			{
				RCon::Password.clear();
			}
			else
			{
				if (!RCon::Password.empty() && *reinterpret_cast<int*>(0xB2C540) >= 5) // Get our state
				{
					Network::Address target(reinterpret_cast<Game::netadr_t*>(0xA5EA44));

					if (target.IsValid())
					{
						Network::SendCommand(target, "rcon", RCon::Password + " " + params.Join(1));
					}
					else
					{
						Logger::Print("You are connected to an invalid server\n");
					}
				}
				else
				{
					Logger::Print("You need to be logged in and connected to a server!\n");
				}
			}
		});
	}

	RCon::~RCon()
	{
		RCon::Password.clear();
	}
}
