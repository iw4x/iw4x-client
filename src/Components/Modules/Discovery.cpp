#include "STDInclude.hpp"

using namespace std::literals;

namespace Components
{
	Discovery::Container Discovery::DiscoveryContainer = { false, false, nullptr };

	void Discovery::Perform()
	{
		Discovery::DiscoveryContainer.Perform = true;
	}

	Discovery::Discovery()
	{
		Dvar::Register<int>("net_discoveryPortRangeMin", 25000, 0, 65535, Game::dvar_flag::DVAR_FLAG_SAVED, "Minimum scan range port for local server discovery");
		Dvar::Register<int>("net_discoveryPortRangeMax", 35000, 1, 65536, Game::dvar_flag::DVAR_FLAG_SAVED, "Maximum scan range port for local server discovery");

		Discovery::DiscoveryContainer.Perform = false;
		Discovery::DiscoveryContainer.Terminate = false;
		Discovery::DiscoveryContainer.Thread = new std::thread([] ()
		{
			while (!Discovery::DiscoveryContainer.Terminate)
			{
				if (Discovery::DiscoveryContainer.Perform)
				{
					int start = Game::Com_Milliseconds();

					Logger::Print("Starting local server discovery...\n");

					Discovery::DiscoveryContainer.Challenge = Utils::VA("%d", Utils::Cryptography::Rand::GenerateInt());

					//Network::BroadcastAll("discovery\n");
					unsigned int minPort = Dvar::Var("net_discoveryPortRangeMin").Get<unsigned int>();
					unsigned int maxPort = Dvar::Var("net_discoveryPortRangeMax").Get<unsigned int>();
					Network::BroadcastRange(minPort, maxPort, Utils::VA("discovery\n%s", Discovery::DiscoveryContainer.Challenge.data()));

					Logger::Print("Discovery sent within %dms, awaiting responses...\n", Game::Com_Milliseconds() - start);

					Discovery::DiscoveryContainer.Perform = false;
				}

				std::this_thread::sleep_for(50ms);
			}
		});

		Network::Handle("discovery", [] (Network::Address address, std::string data)
		{
			if (address.IsSelf()) return;

			if (!address.IsLocal())
			{
				Logger::Print("Received discovery request from non-local address: %s\n", address.GetString());
				return;
			}

			Logger::Print("Received discovery request from %s\n", address.GetString());
			Network::Send(address, Utils::VA("discoveryResponse\n%s", data.data()));
		});

		Network::Handle("discoveryResponse", [] (Network::Address address, std::string data)
		{
			if (address.IsSelf()) return;

			if (!address.IsLocal())
			{
				Logger::Print("Received discovery response from non-local address: %s\n", address.GetString());
				return;
			}

			if (Utils::ParseChallenge(data) != Discovery::DiscoveryContainer.Challenge)
			{
				Logger::Print("Received discovery with invalid challenge from: %s\n", address.GetString());
				return;
			}

			Logger::Print("Received discovery response from: %s\n", address.GetString());

			if (ServerList::IsOfflineList())
			{
				ServerList::InsertRequest(address, true);
			}
		});
	}

	Discovery::~Discovery()
	{
		Discovery::DiscoveryContainer.Perform = false;
		Discovery::DiscoveryContainer.Terminate = true;

		if (Discovery::DiscoveryContainer.Thread)
		{
			Discovery::DiscoveryContainer.Thread->join();
			delete Discovery::DiscoveryContainer.Thread;
			Discovery::DiscoveryContainer.Thread = nullptr;
		}
	}
}
