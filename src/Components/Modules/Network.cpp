#include "STDInclude.hpp"

namespace Components
{
	std::string Network::SelectedPacket;
	wink::signal<wink::slot<Network::CallbackRaw>> Network::StartupSignal;
	std::map<std::string, wink::slot<Network::Callback>> Network::PacketHandlers;

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
	}
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

		return false;
	}
	bool Network::Address::IsValid()
	{
		return (this->GetType() != Game::netadrtype_t::NA_BAD);
	}
	void Network::Address::Serialize(Proto::Network::Address* protoAddress)
	{
		protoAddress->set_ip(this->GetIP().full);
		protoAddress->set_port(htons(this->GetPort()) & 0xFFFF);
	}
	void Network::Address::Deserialize(const Proto::Network::Address& protoAddress)
	{
		this->SetIP(protoAddress.ip());
		this->SetPort(ntohs(static_cast<uint16_t>(protoAddress.port() & 0xFFFF)));
		this->SetType(Game::netadrtype_t::NA_IP);
	}

	void Network::Handle(std::string packet, Network::Callback* callback)
	{
		Network::PacketHandlers[Utils::StrToLower(packet)] = callback;
	}

	void Network::OnStart(Network::CallbackRaw* callback)
	{
		Network::StartupSignal.connect(callback);
	}

	void Network::Send(Game::netsrc_t type, Network::Address target, std::string data)
	{
		// NET_OutOfBandPrint only supports non-binary data!
		//Game::NET_OutOfBandPrint(type, *target.Get(), data.data());

		std::string rawData;
		rawData.append("\xFF\xFF\xFF\xFF", 4);
		rawData.append(data);
		//rawData.append("\0", 1);

		Network::SendRaw(type, target, rawData);
	}

	void Network::Send(Network::Address target, std::string data)
	{
		Network::Send(Game::netsrc_t::NS_CLIENT, target, data);
	}

	void Network::SendRaw(Game::netsrc_t type, Network::Address target, std::string data)
	{
		// NET_OutOfBandData doesn't seem to work properly
		//Game::NET_OutOfBandData(type, *target.Get(), data.data(), data.size());
		Game::Sys_SendPacket(type, data.size(), data.data(), *target.Get());
	}

	void Network::SendRaw(Network::Address target, std::string data)
	{
		Network::SendRaw(Game::netsrc_t::NS_CLIENT, target, data);
	}

	void Network::SendCommand(Game::netsrc_t type, Network::Address target, std::string command, std::string data)
	{
		// Use space a separator (possible separators are '\n', ' ').
		// Though, our handler only needs exactly 1 char as separator and doesn't which char it is
		std::string packet;
		packet.append(command);
		packet.append(" ", 1);
		packet.append(data);

		Network::Send(type, target, packet);
	}

	void Network::SendCommand(Network::Address target, std::string command, std::string data)
	{
		Network::SendCommand(Game::netsrc_t::NS_CLIENT, target, command, data);
	}

	void Network::Broadcast(unsigned short port, std::string data)
	{
		Address target;

		target.SetPort(port);
		target.SetIP(INADDR_BROADCAST);
		target.SetType(Game::netadrtype_t::NA_BROADCAST);

		Network::Send(Game::netsrc_t::NS_CLIENT, target, data);
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
		// Packet rate limit. 
		static uint32_t packets = 0;
		static int lastClean = 0;

		if ((Game::Com_Milliseconds() - lastClean) > 1'000)
		{
			packets = 0;
			lastClean = Game::Com_Milliseconds();
		}

		if ((++packets) > NETWORK_MAX_PACKETS_PER_SECOND)
		{
			return 1;
		}

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

	void Network::DeployPacket(Game::netadr_t* from, Game::msg_t* msg)
	{
		if (Network::PacketHandlers.find(Network::SelectedPacket) != Network::PacketHandlers.end())
		{
			std::string data;

			size_t offset = Network::SelectedPacket.size() + 4 + 1;

			if (static_cast<size_t>(msg->cursize) > offset)
			{
				data.append(msg->data + offset, msg->cursize - offset);
			}

			// Remove trailing 0x00 byte
			// Actually, don't remove it, it might be part of the packet. Send correctly formatted packets instead!
			//if (data.size() && !data[data.size() - 1]) data.pop_back();

			Network::PacketHandlers[Network::SelectedPacket](from, data);
		}
		else
		{
			Logger::Print("Error: Network packet intercepted, but handler is missing!\n");
		}
	}

	void Network::NetworkStart()
	{
		Network::StartupSignal();
	}

	void __declspec(naked) Network::NetworkStartStub()
	{
		__asm 
		{
			mov eax, 64D900h
			call eax
			jmp Network::NetworkStart
		}
	}

	void __declspec(naked) Network::DeployPacketStub()
	{
		__asm
		{
			lea eax, [esp + 0C54h]
			push ebp // Command
			push eax // Address pointer
			call Network::DeployPacket
			add esp, 8h
			mov al, 1
			pop edi
			pop esi
			pop ebp
			pop ebx
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

		// increase max port binding attempts from 10 to 100
		Utils::Hook::Set<BYTE>(0x4FD48A, 100);

		// Parse port as short in Net_AddrToString
		Utils::Hook::Set<char*>(0x4698E3, "%u.%u.%u.%u:%hu");

		// Install startup handler
		Utils::Hook(0x4FD4D4, Network::NetworkStartStub, HOOK_JUMP).Install()->Quick();

		// Install interception handler
		Utils::Hook(0x5AA709, Network::PacketInterceptionHandler, HOOK_CALL).Install()->Quick();

		// Install packet deploy hook
		Utils::Hook::RedirectJump(0x5AA713, Network::DeployPacketStub);
	}

	Network::~Network()
	{
		Network::SelectedPacket.clear();
		Network::PacketHandlers.clear();
		Network::StartupSignal.clear();
	}
}
