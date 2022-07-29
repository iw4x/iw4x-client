#include <STDInclude.hpp>

namespace Components
{
	std::string Network::SelectedPacket;
	Utils::Signal<Network::CallbackRaw> Network::StartupSignal;
	// Packet interception
	std::unordered_map<std::string, Network::NetworkCallback> Network::Callbacks;

	Network::Address::Address(const std::string& addrString)
	{
		Game::NET_StringToAdr(addrString.data(), &this->address);
	}

	Network::Address::Address(sockaddr* addr)
	{
		Game::SockadrToNetadr(addr, &this->address);
	}

	bool Network::Address::operator==(const Network::Address& obj) const
	{
		return Game::NET_CompareAdr(this->address, obj.address);
	}

	void Network::Address::setPort(unsigned short port)
	{
		this->address.port = htons(port);
	}

	unsigned short Network::Address::getPort()
	{
		return ntohs(this->address.port);
	}

	void Network::Address::setIP(DWORD ip)
	{
		this->address.ip.full = ip;
	}

	void Network::Address::setIP(Game::netIP_t ip)
	{
		this->address.ip = ip;
	}

	Game::netIP_t Network::Address::getIP()
	{
		return this->address.ip;
	}

	void Network::Address::setType(Game::netadrtype_t type)
	{
		this->address.type = type;
	}

	Game::netadrtype_t Network::Address::getType()
	{
		return this->address.type;
	}

	sockaddr Network::Address::getSockAddr()
	{
		sockaddr addr;
		this->toSockAddr(&addr);
		return addr;
	}

	void Network::Address::toSockAddr(sockaddr* addr)
	{
		if (addr)
		{
			Game::NetadrToSockadr(&this->address, addr);
		}
	}

	void Network::Address::toSockAddr(sockaddr_in* addr)
	{
		this->toSockAddr(reinterpret_cast<sockaddr*>(addr));
	}

	Game::netadr_t* Network::Address::get()
	{
		return &this->address;
	}

	const char* Network::Address::getCString() const
	{
		return Game::NET_AdrToString(this->address);
	}

	std::string Network::Address::getString() const
	{
		return this->getCString();
	}

	bool Network::Address::isLocal()
	{
		// According to: https://en.wikipedia.org/wiki/Private_network

		// 10.X.X.X
		if (this->getIP().bytes[0] == 10) return true;

		// 192.168.X.X
		if (this->getIP().bytes[0] == 192 && this->getIP().bytes[1] == 168) return true;

		// 172.16.X.X - 172.31.X.X
		if (this->getIP().bytes[0] == 172 && (this->getIP().bytes[1] >= 16) && (this->getIP().bytes[1] < 32)) return true;

		// 127.0.0.1
		if (this->getIP().full == 0x0100007F) return true;

		// TODO: Maybe check for matching localIPs and subnet mask

		return false;
	}

	bool Network::Address::isSelf()
	{
		if (Game::NET_IsLocalAddress(this->address)) return true; // Loopback
		if (this->getPort() != Network::GetPort()) return false; // Port not equal

		for (int i = 0; i < *Game::numIP; ++i)
		{
			if (this->getIP().full == Game::localIP[i].full)
			{
				return true;
			}
		}

		return false;
	}

	bool Network::Address::isLoopback()
	{
		if (this->getIP().full == 0x100007f) // 127.0.0.1
		{
			return true;
		}

		return Game::NET_IsLocalAddress(this->address);
	}

	bool Network::Address::isValid()
	{
		return (this->getType() != Game::netadrtype_t::NA_BAD && this->getType() >= Game::netadrtype_t::NA_BOT && this->getType() <= Game::netadrtype_t::NA_IP);
	}

	void Network::OnStart(Utils::Slot<Network::CallbackRaw> callback)
	{
		Network::StartupSignal.connect(callback);
	}

	void Network::Send(Game::netsrc_t type, Network::Address target, const std::string& data)
	{
		// NET_OutOfBandPrint only supports non-binary data!
		//Game::NET_OutOfBandPrint(type, *target.Get(), data.data());

		std::string rawData;
		rawData.append("\xFF\xFF\xFF\xFF", 4);
		rawData.append(data);
		//rawData.append("\0", 1);

		Network::SendRaw(type, target, rawData);
	}

	void Network::Send(Network::Address target, const std::string& data)
	{
		Network::Send(Game::netsrc_t::NS_CLIENT1, target, data);
	}

	void Network::SendRaw(Game::netsrc_t type, Network::Address target, const std::string& data)
	{
		if (!target.isValid()) return;

		// NET_OutOfBandData doesn't seem to work properly
		//Game::NET_OutOfBandData(type, *target.Get(), data.data(), data.size());
		Game::Sys_SendPacket(type, data.size(), data.data(), *target.get());
	}

	void Network::SendRaw(Network::Address target, const std::string& data)
	{
		Network::SendRaw(Game::netsrc_t::NS_CLIENT1, target, data);
	}

	void Network::SendCommand(Game::netsrc_t type, Network::Address target, const std::string& command, const std::string& data)
	{
		// Use space as separator (possible separators are '\n', ' ').
		// Though, our handler only needs exactly 1 char as separator and doesn't care which char it is.
		// EDIT: Most 3rd party tools expect a line break, so let's use that instead!
		std::string packet;
		packet.append(command);
		packet.append("\n", 1);
		packet.append(data);

		Network::Send(type, target, packet);
	}

	void Network::SendCommand(Network::Address target, const std::string& command, const std::string& data)
	{
		Network::SendCommand(Game::netsrc_t::NS_CLIENT1, target, command, data);
	}

	void Network::Broadcast(unsigned short port, const std::string& data)
	{
		Address target;

		target.setPort(port);
		target.setIP(INADDR_BROADCAST);
		target.setType(Game::netadrtype_t::NA_BROADCAST);

		Network::Send(Game::netsrc_t::NS_CLIENT1, target, data);
	}

	void Network::BroadcastRange(unsigned int min, unsigned int max, const std::string& data)
	{
		for (unsigned int i = min; i < max; ++i)
		{
			Network::Broadcast(static_cast<unsigned short>(i & 0xFFFF), data);
		}
	}

	void Network::BroadcastAll(const std::string& data)
	{
		Network::BroadcastRange(100, 65536, data);
	}

	void Network::NetworkStart()
	{
		Network::StartupSignal();
	}

	unsigned short Network::GetPort()
	{
		return static_cast<unsigned short>(Dvar::Var(0x64A3004).get<unsigned int>());
	}

	__declspec(naked) void Network::NetworkStartStub()
	{
		__asm
		{
			mov eax, 64D900h
			call eax
			jmp Network::NetworkStart
		}
	}

	__declspec(naked) void Network::PacketErrorCheck()
	{
		__asm
		{
			cmp eax, 2746h
			jz returnIgnore

			cmp eax, WSAENETRESET
			jz returnIgnore

			push 465325h
			retn

		returnIgnore:
			push 4654C6h
			retn
		}
	}

	void Network::SV_ExecuteClientMessageStub(Game::client_t* client, Game::msg_t* msg)
	{
		if (client->reliableAcknowledge < 0)
		{
			Logger::Print(Game::conChannel_t::CON_CHANNEL_NETWORK, "Negative reliableAcknowledge from {} - cl->reliableSequence is {}, reliableAcknowledge is {}\n",
							client->name, client->reliableSequence, client->reliableAcknowledge);
			client->reliableAcknowledge = client->reliableSequence;
			Network::SendCommand(Game::NS_SERVER, client->netchan.remoteAddress, "error", "EXE_LOSTRELIABLECOMMANDS");
			return;
		}

		Utils::Hook::Call<void(Game::client_t*, Game::msg_t*)>(0x414D40)(client, msg);
	}

	void Network::OnPacket(const std::string& command, const NetworkCallback& callback)
	{
		Network::Callbacks[Utils::String::ToLower(command)] = callback;
	}

	bool Network::HandleCommand(Game::netadr_t* address, const char* command, const Game::msg_t* message)
	{
		const auto command_ = Utils::String::ToLower(command);
		const auto handler = Network::Callbacks.find(command_);

		const auto offset = command_.size() + 5;
		if (static_cast<std::size_t>(message->cursize) < offset || handler == Network::Callbacks.end())
		{
			return false;
		}

		const std::string data(message->data + offset, message->cursize - offset);

		Address address_ = address;
		handler->second(address_, data);
		return true;
	}

	__declspec(naked) void Network::CL_HandleCommandStub()
	{
		__asm
		{
			lea eax, [esp + 0xC54] // address

			pushad

			push ebp // msg_t
			push edi // Command name
			push eax // netadr_t pointer
			call Network::HandleCommand
			add esp, 0xC

			test al, al

			popad

			jz unhandled

			// Exit CL_DispatchConnectionlessPacket
			push 0x5A9E0E
			retn

		unhandled:
			// Proceed
			push 0x5AA719
			retn
		}
	}

	Network::Network()
	{
		AssertSize(Game::netadr_t, 20);

		// maximum size in NET_OutOfBandPrint
		Utils::Hook::Set<DWORD>(0x4AEF08, 0x1FFFC);
		Utils::Hook::Set<DWORD>(0x4AEFA3, 0x1FFFC);

		// increase max port binding attempts from 10 to 100
		Utils::Hook::Set<BYTE>(0x4FD48A, 100);

		// increase cl_maxpackets limit
		Utils::Hook::Set<BYTE>(0x4050A1, 125);

		// increase snaps
		//Utils::Hook::Set<BYTE>(0x405357, 40);

		// default maxpackets and snaps
		Utils::Hook::Set<BYTE>(0x40535B, 30);
		Utils::Hook::Set<BYTE>(0x4050A5, 125);

		// Parse port as short in Net_AddrToString
		Utils::Hook::Set<const char*>(0x4698E3, "%u.%u.%u.%u:%hu");

		// Install startup handler
		Utils::Hook(0x4FD4D4, Network::NetworkStartStub, HOOK_JUMP).install()->quick();

		// Prevent recvfrom error spam
		Utils::Hook(0x46531A, Network::PacketErrorCheck, HOOK_JUMP).install()->quick();

		// Fix server freezer exploit
		Utils::Hook(0x626996, Network::SV_ExecuteClientMessageStub, HOOK_CALL).install()->quick();
		
		// Handle client packets
		Utils::Hook(0x5AA703, Network::CL_HandleCommandStub, HOOK_JUMP).install()->quick();

		// Disable unused OOB packets handlers just to be sure
		Utils::Hook::Set<BYTE>(0x5AA5B6, 0xEB); // CL_SteamServerAuth
		Utils::Hook::Set<BYTE>(0x5AA69F, 0xEB); // echo
		Utils::Hook::Set<BYTE>(0x5AAA82, 0xEB); // SP

		Network::OnPacket("resolveAddress", [](const Address& address, [[maybe_unused]] const std::string& data)
		{
			Network::SendRaw(address, address.getString());
		});
	}
}
