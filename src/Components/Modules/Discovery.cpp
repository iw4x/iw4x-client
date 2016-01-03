#include "..\..\STDInclude.hpp"

namespace Components
{
	void Discovery::Perform()
	{
		static bool performing = false;
		if (performing) return;

		std::async([] ()
		{
			performing = true;
			int start = Game::Com_Milliseconds();
			
			Logger::Print("Starting local server discovery...\n");

			//Network::BroadcastAll("discovery\n");
			Network::BroadcastRange(28960, 38960, "discovery\n");

			Logger::Print("Discovery sent within %dms, awaiting responses...\n", Game::Com_Milliseconds() - start);
			performing = false;
		});
	}

	Discovery::Discovery()
	{
		Network::Handle("discovery", [] (Network::Address address, std::string data)
		{
			if (address.IsSelf()) return;

			if (!address.IsLocal())
			{
				Logger::Print("Received discovery request from non-local address: %s\n", address.GetString());
				return;
			}

			Logger::Print("Received discovery request from %s\n", address.GetString());
			Network::Send(address, "discoveryResponse\n");
		});

		Network::Handle("discoveryResponse", [] (Network::Address address, std::string data)
		{
			if (address.IsSelf()) return;

			if (!address.IsLocal())
			{
				Logger::Print("Received discovery response from non-local address: %s\n", address.GetString());
				return;
			}

			Logger::Print("Received discovery response from %s\n", address.GetString());

			if (ServerList::IsOfflineList())
			{
				OutputDebugStringA("Inserting!");
				ServerList::InsertRequest(address, true);
			}
		});
	}

	Discovery::~Discovery()
	{
		
	}
}
