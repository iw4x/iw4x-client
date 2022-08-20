#include <STDInclude.hpp>

namespace Components
{
	bool Discovery::IsTerminating = false;
	bool Discovery::IsPerforming = false;
	std::thread Discovery::Thread;
	std::string Discovery::Challenge;

	void Discovery::Perform()
	{
		Discovery::IsPerforming = true;
	}

	Discovery::Discovery()
	{
		Dvar::Register<int>("net_discoveryPortRangeMin", 25000, 0, 65535, Game::DVAR_ARCHIVE, "Minimum scan range port for local server discovery");
		Dvar::Register<int>("net_discoveryPortRangeMax", 35000, 1, 65536, Game::DVAR_ARCHIVE, "Maximum scan range port for local server discovery");

		// An additional thread prevents lags
		// Not sure if that's the best way though
		Discovery::IsPerforming = false;
		Discovery::IsTerminating = false;
		Discovery::Thread = std::thread([]()
		{
			while (!Discovery::IsTerminating)
			{
				if (Discovery::IsPerforming)
				{
					int start = Game::Sys_Milliseconds();

					Logger::Print("Starting local server discovery...\n");

					Discovery::Challenge = Utils::Cryptography::Rand::GenerateChallenge();

					unsigned int minPort = Dvar::Var("net_discoveryPortRangeMin").get<unsigned int>();
					unsigned int maxPort = Dvar::Var("net_discoveryPortRangeMax").get<unsigned int>();
					Network::BroadcastRange(minPort, maxPort, Utils::String::VA("discovery %s", Discovery::Challenge.data()));

					Logger::Print("Discovery sent within {}ms, awaiting responses...\n", Game::Sys_Milliseconds() - start);

					Discovery::IsPerforming = false;
				}

				std::this_thread::sleep_for(50ms);
			}
		});

		Network::OnClientPacket("discovery", [](Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			if (address.isSelf()) return;

			if (!address.isLocal())
			{
				Logger::Print("Received discovery request from non-local address: {}\n", address.getString());
				return;
			}

			Logger::Print("Received discovery request from {}\n", address.getString());
			Network::SendCommand(address, "discoveryResponse", data);
		});

		Network::OnClientPacket("discoveryResponse", [](Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			if (address.isSelf()) return;

			if (!address.isLocal())
			{
				Logger::Print("Received discovery response from non-local address: {}\n", address.getString());
				return;
			}

			if (Utils::ParseChallenge(data) != Discovery::Challenge)
			{
				Logger::Print("Received discovery with invalid challenge from: {}\n", address.getString());
				return;
			}

			Logger::Print("Received discovery response from: {}\n", address.getString());

			if (ServerList::IsOfflineList())
			{
				ServerList::InsertRequest(address);
			}
		});
	}

	void Discovery::preDestroy()
	{
		Discovery::IsPerforming = false;
		Discovery::IsTerminating = true;

		if (Discovery::Thread.joinable())
		{
			Discovery::Thread.join();
		}
	}
}
