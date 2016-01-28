#include "STDInclude.hpp"

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
		this->address.port = htons(port);
	};
	unsigned short Network::Address::GetPort()
	{
		return ntohs(this->address.port);
	}
	void Network::Address::SetIP(DWORD ip)
	{
		this->address.ip.full = ip;
	}
	void Network::Address::SetIP(Game::netIP_t ip)
	{
		this->address.ip = ip;
	}
	Game::netIP_t Network::Address::GetIP()
	{
		return this->address.ip;
	}
	void Network::Address::SetType(Game::netadrtype_t type)
	{
		this->address.type = type;
	}
	Game::netadrtype_t Network::Address::GetType()
	{
		return this->address.type;
	}
	Game::netadr_t* Network::Address::Get()
	{
		return &this->address;
	}
	const char* Network::Address::GetString()
	{
		return Game::NET_AdrToString(this->address);
	}
	bool Network::Address::IsLocal()
	{
		// According to: https://en.wikipedia.org/wiki/Private_network

		// 10.X.X.X
		if (this->GetIP().bytes[0] == 10) return true;

		// 192.168.X.X
		if (this->GetIP().bytes[0] == 192 && this->GetIP().bytes[1] == 168) return true;

		// 172.16.X.X - 172.31.X.X
		if (this->GetIP().bytes[0] == 172 && (this->GetIP().bytes[1] >= 16) && (this->GetIP().bytes[1] < 32)) return true;

		// TODO: Maybe check for matching localIPs and subnet mask

		return false;
	}
	bool Network::Address::IsSelf()
	{
		if (Game::NET_IsLocalAddress(this->address)) return true; // Loopback
		if (this->GetPort() != (Dvar::Var("net_port").Get<int>() & 0xFFFF)) return false; // Port not equal

		for (int i = 0; i < *Game::numIP; ++i)
		{
			if (this->GetIP().full == Game::localIP[i].full)
			{
				return true;
			}
		}

		// TODO: Check the external IP as well!

		return false;
	}

	void Network::Handle(std::string packet, Network::Callback callback)
	{
		Network::PacketHandlers[Utils::StrToLower(packet)] = callback;
	}

	void Network::Send(Game::netsrc_t type, Address target, std::string data)
	{
		Game::NET_OutOfBandPrint(type, *target.Get(), data.data());
	}

	void Network::Send(Address target, std::string data)
	{
		Network::Send(Game::netsrc_t::NS_CLIENT, target, data);
	}

	void Network::SendRaw(Game::netsrc_t type, Address target, std::string data)
	{
		DWORD header = 0xFFFFFFFF;

		std::string rawData;
		rawData.append(reinterpret_cast<char*>(&header), 4);
		rawData.append(data.begin(), data.end());
		rawData.append("\0", 1);

		// NET_OutOfBandData doesn't seem to work properly
		//Game::NET_OutOfBandData(type, *target.Get(), data.data(), data.size());
		Game::Sys_SendPacket(type, rawData.size(), rawData.data(), *target.Get());
	}

	void Network::SendRaw(Address target, std::string data)
	{
		Network::SendRaw(Game::netsrc_t::NS_CLIENT, target, data);
	}

	void Network::Broadcast(unsigned short port, std::string data)
	{
		Address target;

		target.SetPort(port);
		target.SetIP(INADDR_BROADCAST);
		target.SetType(Game::netadrtype_t::NA_BROADCAST);

		Network::SendRaw(Game::netsrc_t::NS_CLIENT, target, data);
	}

	void Network::BroadcastRange(unsigned int min, unsigned int max, std::string data)
	{
		for (unsigned int i = min; i < max; ++i)
		{
			Network::Broadcast(static_cast<unsigned short>(i & 0xFFFF), data);
		}
	}

	void Network::BroadcastAll(std::string data)
	{
		Network::BroadcastRange(100, 65536, data);
	}

	int Network::PacketInterceptionHandler(const char* packet)
	{
		std::string packetCommand = packet;
		auto pos = packetCommand.find_first_of("\\\n ");
		if (pos != std::string::npos)
		{
			packetCommand = packetCommand.substr(0, pos);
		}

		packetCommand = Utils::StrToLower(packetCommand);

		// Check if custom handler exists
		for (auto i = Network::PacketHandlers.begin(); i != Network::PacketHandlers.end(); ++i)
		{
			if (Utils::StrToLower(i->first) == packetCommand)
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
			size_t offset = Network::SelectedPacket.size() + 4 + 1;

			std::string data(msg->data + offset, msg->cursize - offset);

			// Remove trailing 0x00 byte
			if (data.size() && !data[data.size() - 1]) data.pop_back();

			Network::PacketHandlers[Network::SelectedPacket](from, data);
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
		Assert_Size(Game::netadr_t, 20);

		// maximum size in NET_OutOfBandPrint
		Utils::Hook::Set<DWORD>(0x4AEF08, 0x1FFFC);
		Utils::Hook::Set<DWORD>(0x4AEFA3, 0x1FFFC);

		// Parse port as short in Net_AddrToString
		Utils::Hook::Set<char*>(0x4698E3, "%u.%u.%u.%u:%hu");

		// Install interception handler
		Utils::Hook(0x5AA709, Network::PacketInterceptionHandler, HOOK_CALL).Install()->Quick();

		// Install packet deploy hook
		Utils::Hook::Set<int>(0x5AA715, (DWORD)Network::DeployPacketStub - 0x5AA713 - 6);
	}

	Network::~Network()
	{
		Network::SelectedPacket.clear();
		Network::PacketHandlers.clear();
	}
}
