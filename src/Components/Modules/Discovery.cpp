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
		Dvar::Register<int>("net_discoveryPortRangeMin", 25000, 0, 65535, Game::dvar_flag::DVAR_ARCHIVE, "Minimum scan range port for local server discovery");
		Dvar::Register<int>("net_discoveryPortRangeMax", 35000, 1, 65536, Game::dvar_flag::DVAR_ARCHIVE, "Maximum scan range port for local server discovery");

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

					Logger::Print("Discovery sent within %dms, awaiting responses...\n", Game::Sys_Milliseconds() - start);

					Discovery::IsPerforming = false;
				}

				std::this_thread::sleep_for(50ms);
			}
		});

		Network::Handle("discovery", [](Network::Address address, std::string data)
		{
			if (address.isSelf()) return;

			if (!address.isLocal())
			{
				Logger::Print("Received discovery request from non-local address: %s\n", address.getCString());
				return;
			}

			Logger::Print("Received discovery request from %s\n", address.getCString());
			Network::SendCommand(address, "discoveryResponse", data);
		});

		Network::Handle("discoveryResponse", [](Network::Address address, std::string data)
		{
			if (address.isSelf()) return;

			if (!address.isLocal())
			{
				Logger::Print("Received discovery response from non-local address: %s\n", address.getCString());
				return;
			}

			if (Utils::ParseChallenge(data) != Discovery::Challenge)
			{
				Logger::Print("Received discovery with invalid challenge from: %s\n", address.getCString());
				return;
			}

			Logger::Print("Received discovery response from: %s\n", address.getCString());

			if (ServerList::IsOfflineList())
			{
				ServerList::InsertRequest(address);
			}
		});

		// This is placed here in case the anticheat has been disabled!
		// Make sure this is called after the memory scan!
#ifndef DISABLE_ANTICHEAT
		Utils::Hook(0x5ACB9E, []() // Somewhere in the renderer, past the scan check
		{
			AntiCheat::ScanIntegrityCheck();
			return Utils::Hook::Call<void()>(0x4AA720)();
		}, HOOK_CALL).install()->quick();
#endif
	}

	Discovery::~Discovery()
	{

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
