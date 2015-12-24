#include "..\STDInclude.hpp"

namespace Components
{
	std::string Network::SelectedPacket;
	std::map<std::string, Network::Callback> Network::PacketHandlers;

	Network::Address::Address(std::string addrString)
	{
		Game::NET_StringToAdr(addrString.data(), &this->address);
	}
	bool Network::Address::operator==(const Network::Address &obj)
	{
		return Game::NET_CompareAdr(this->address, obj.address);
	}
	void Network::Address::SetPort(unsigned short port)
	{
		this->address.port = port;
	};
	unsigned short Network::Address::GetPort()
	{
		return this->address.port;
	}
	Game::netadr_t* Network::Address::Get()
	{
		return &this->address;
	}
	const char* Network::Address::GetString()
	{
		return Game::NET_AdrToString(this->address);
	}

	void Network::Handle(std::string packet, Network::Callback callback)
	{
		Network::PacketHandlers[Utils::StrToLower(packet)] = callback;
	}

	void Network::Send(Game::netsrc_t type, Address target, std::string data)
	{
		Game::OOBPrintT(type, *target.Get(), data.data());
	}

	int Network::PacketInterceptionHandler(const char* packet)
	{
		// Check if custom handler exists
		for (auto i = Network::PacketHandlers.begin(); i != Network::PacketHandlers.end(); i++)
		{
			if (!_strnicmp(i->first.data(), packet, i->first.size()))
			{
				Network::SelectedPacket = i->first;
				return 0;
			}
		}

		// No interception
		return 1;
	}

	void Network::DeployPacket(Game::netadr_t from, Game::msg_t* msg)
	{
		if (Network::PacketHandlers.find(Network::SelectedPacket) != Network::PacketHandlers.end())
		{
			Network::PacketHandlers[Network::SelectedPacket](from, msg);
		}
		else
		{
			Logger::Print("Error: Network packet intercepted, but handler is missing!\n");
		}
	}

	void __declspec(naked) Network::DeployPacketStub()
	{
		__asm
		{
			push ebp //C54
			// esp = C54h?
			mov eax, [esp + 0C54h + 14h]
			push eax
			mov eax, [esp + 0C58h + 10h]
			push eax
			mov eax, [esp + 0C5Ch + 0Ch]
			push eax
			mov eax, [esp + 0C60h + 08h]
			push eax
			mov eax, [esp + 0C64h + 04h]
			push eax
			call Network::DeployPacket
			add esp, 14h
			add esp, 4h
			mov al, 1
			//C50
			pop edi //C4C
			pop esi //C48
			pop ebp //C44
			pop ebx //C40
			add esp, 0C40h
			retn
		}
	}

	Network::Network()
	{
		// maximum size in NET_OutOfBandPrint
		Utils::Hook::Set<DWORD>(0x4AEF08, 0x1FFFC);
		Utils::Hook::Set<DWORD>(0x4AEFA3, 0x1FFFC);

		// Install interception handler
		Utils::Hook(0x5AA709, Network::PacketInterceptionHandler, HOOK_CALL).Install()->Quick();

		// Install packet deploy hook
		Utils::Hook::Set<int>(0x5AA715, (DWORD)Network::DeployPacketStub - 0x5AA713 - 6);

		Network::Handle("infoResponse", [] (Address address, Game::msg_t* message)
		{
			OutputDebugStringA(Utils::VA("Inforesponse received: %s %s!", address.GetString(), message->data));
		});

		Network::Handle("getInfo", [] (Address address, Game::msg_t* message)
		{
			OutputDebugStringA(Utils::VA("getinfo received: %s!", address.GetString()));

			int clientCount = 0;

			for (int i = 0; i < *Game::svs_numclients; i++)
			{
				if (Game::svs_clients[i].state >= 3)
				{
					clientCount++;
				}
			}

			auto data = std::string(message->data);
			auto challenge = data.substr(data.find_first_of(" \n") + 1);
			challenge = challenge.substr(0, challenge.find_first_of(" \n"));

			Utils::InfoString info;
			info.Set("challenge",     challenge.data()); // TODO: Fill!
			info.Set("gamename",      "IW4");
			info.Set("hostname",      Dvar::Var("sv_hostname").Get<const char*>());
			info.Set("mapname",       Dvar::Var("mapname").Get<const char*>());
			info.Set("gametype",      Dvar::Var("g_gametype").Get<const char*>());
			info.Set("fs_game",       Dvar::Var("fs_game").Get<const char*>());
			info.Set("xuid",          Utils::VA("%llX", Steam::SteamUser()->GetSteamID().m_Bits));
			info.Set("clients",       Utils::VA("%i", clientCount));
			info.Set("sv_maxclients", Utils::VA("%i", *Game::svs_numclients));

			Network::Send(Game::NS_CLIENT, address, Utils::VA("infoResponse\n%s\n", info.Build().data()));
		});

		Command::Add("zob", [] (Command::Params params)
		{
			Network::Send(Game::NS_CLIENT, Network::Address("localhost:28960"), "getinfo xxx\n");
		});
	}

	Network::~Network()
	{
		Network::SelectedPacket.clear();
		Network::PacketHandlers.clear();
	}
}
