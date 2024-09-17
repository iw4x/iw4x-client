#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include "Discovery.hpp"
#include "ServerList.hpp"

namespace Components
{
	bool Discovery::IsTerminating = false;
	bool Discovery::IsPerforming = false;
	std::thread Discovery::Thread;
	std::string Discovery::Challenge;

	Dvar::Var Discovery::NetDiscoveryPortRangeMin;
	Dvar::Var Discovery::NetDiscoveryPortRangeMax;

	void Discovery::Perform()
	{
		IsPerforming = true;
	}

	Discovery::Discovery()
	{
		NetDiscoveryPortRangeMin = Dvar::Register<int>("net_discoveryPortRangeMin", 25000, 0, 65535, Game::DVAR_NONE, "Minimum scan range port for local server discovery");
		NetDiscoveryPortRangeMax = Dvar::Register<int>("net_discoveryPortRangeMax", 35000, 1, 65536, Game::DVAR_NONE, "Maximum scan range port for local server discovery");

		// An additional thread prevents lags
		// Not sure if that's the best way though
		IsPerforming = false;
		IsTerminating = false;
		Thread = std::thread([]
		{
			Com_InitThreadData();

			while (!IsTerminating)
			{
				if (IsPerforming)
				{
					const auto start = Game::Sys_Milliseconds();

					Logger::Print("Starting local server discovery...\n");

					Challenge = Utils::Cryptography::Rand::GenerateChallenge();

					const auto minPort = NetDiscoveryPortRangeMin.get<unsigned int>();
					const auto maxPort = NetDiscoveryPortRangeMax.get<unsigned int>();
					Network::BroadcastRange(minPort, maxPort, std::format("discovery {}", Challenge));

					Logger::Print("Discovery sent within {}ms, awaiting responses...\n", Game::Sys_Milliseconds() - start);

					IsPerforming = false;
				}

				Game::Sys_Sleep(50);
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

			if (Utils::ParseChallenge(data) != Challenge)
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
		IsPerforming = false;
		IsTerminating = true;

		if (Thread.joinable())
		{
			Thread.join();
		}
	}
}
