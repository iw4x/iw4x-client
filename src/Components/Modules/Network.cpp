#include <STDInclude.hpp>

namespace Components
{
	// Packet interception
	std::unordered_map<std::string, Network::networkCallback> Network::CL_Callbacks;
	std::unordered_map<std::string, Network::networkRawCallback> Network::CL_RawCallbacks;

	Network::Address::Address()
	{
		ZeroMemory(&this->address, sizeof(Game::netadr_t));
		this->setType(Game::NA_BAD);
	}

	Network::Address::Address(const std::string& addrString)
	{
		ZeroMemory(&this->address, sizeof(Game::netadr_t));
		Game::NET_StringToAdr(addrString.data(), &this->address);
	}

	Network::Address::Address(sockaddr* addr)
	{
		ZeroMemory(&this->address, sizeof(Game::netadr_t));
		Game::SockadrToNetadr(addr, &this->address);
	}

	bool Network::Address::operator==(const Address& obj) const
	{
		return Game::NET_CompareAdr(this->address, obj.address);
	}

	void Network::Address::setPort(unsigned short port)
	{
		this->address.port = ::htons(port);
	}

	unsigned short Network::Address::getPort() const
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

	Game::netIP_t Network::Address::getIP() const
	{
		return this->address.ip;
	}

	void Network::Address::setType(Game::netadrtype_t type)
	{
		this->address.type = type;
	}

	Game::netadrtype_t Network::Address::getType() const
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

	const Game::netadr_t* Network::Address::get() const noexcept
	{
		return &this->address;
	}

	const char* Network::Address::getCString() const
	{
		return Game::NET_AdrToString(this->address);
	}

	std::string Network::Address::getString() const
	{
		return std::string{ this->getCString() };
	}

	bool Network::Address::isLocal() const noexcept
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

	bool Network::Address::isSelf() const noexcept
	{
		if (Game::NET_IsLocalAddress(this->address)) return true; // Loopback
		if (this->getPort() != GetPort()) return false; // Port not equal

		for (int i = 0; i < *Game::numIP; ++i)
		{
			if (this->getIP().full == Game::localIP[i].full)
			{
				return true;
			}
		}

		return false;
	}

	bool Network::Address::isLoopback() const noexcept
	{
		if (this->getIP().full == 0x100007f) // 127.0.0.1
		{
			return true;
		}

		return Game::NET_IsLocalAddress(this->address);
	}

	bool Network::Address::isValid() const noexcept
	{
		return (this->getType() != Game::NA_BAD && this->getType() >= Game::NA_BOT && this->getType() <= Game::NA_IP);
	}

	void Network::Send(Game::netsrc_t type, const Address& target, const std::string& data)
	{
		// Do not use NET_OutOfBandPrint. It only supports non-binary data!

		std::string rawData;
		rawData.append("\xFF\xFF\xFF\xFF", 4);
		rawData.append(data);

		SendRaw(type, target, rawData);
	}

	void Network::Send(const Address& target, const std::string& data)
	{
		Send(Game::netsrc_t::NS_CLIENT1, target, data);
	}

	void Network::SendRaw(Game::netsrc_t type, const Address& target, const std::string& data)
	{
		if (!target.isValid()) return;

		// NET_OutOfBandData doesn't seem to work properly. Do not use it
		Game::Sys_SendPacket(type, data.size(), data.data(), *target.get());
	}

	void Network::SendRaw(const Address& target, const std::string& data)
	{
		SendRaw(Game::NS_CLIENT1, target, data);
	}

	void Network::SendCommand(Game::netsrc_t type, const Address& target, const std::string& command, const std::string& data)
	{
		// Use space as separator (possible separators are '\n', ' ').
		// Though, our handler only needs exactly 1 char as separator and doesn't care which char it is.
		// EDIT: Most 3rd party tools expect a line break, so let's use that instead!
		std::string packet;
		packet.append(command);
		packet.push_back('\n');
		packet.append(data);

		Send(type, target, packet);
	}

	void Network::SendCommand(const Address& target, const std::string& command, const std::string& data)
	{
		SendCommand(Game::NS_CLIENT1, target, command, data);
	}

	void Network::Broadcast(unsigned short port, const std::string& data)
	{
		Address target;

		target.setPort(port);
		target.setIP(INADDR_BROADCAST);
		target.setType(Game::netadrtype_t::NA_BROADCAST);

		Send(Game::netsrc_t::NS_CLIENT1, target, data);
	}

	void Network::BroadcastRange(unsigned int min, unsigned int max, const std::string& data)
	{
		for (unsigned int i = min; i < max; ++i)
		{
			Broadcast(static_cast<unsigned short>(i & 0xFFFF), data);
		}
	}

	void Network::BroadcastAll(const std::string& data)
	{
		BroadcastRange(100, 65536, data);
	}

	std::uint16_t Network::GetPort()
	{
		assert((*Game::port));
		assert((*Game::port)->current.unsignedInt <= std::numeric_limits<std::uint16_t>::max());
		return static_cast<std::uint16_t>((*Game::port)->current.unsignedInt);
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

	void Network::OnClientPacket(const std::string& command, const networkCallback& callback)
	{
		CL_Callbacks[Utils::String::ToLower(command)] = callback;
	}

	void Network::OnClientPacketRaw(const std::string& command, const networkRawCallback& callback)
	{
		CL_RawCallbacks[Utils::String::ToLower(command)] = callback;
	}

	bool Network::CL_HandleCommand(Game::netadr_t* address, const char* command, Game::msg_t* message)
	{
		const auto command_ = Utils::String::ToLower(command);

		const auto offset = command_.size() + 5;
		if (static_cast<std::size_t>(message->cursize) < offset)
		{
			return false;
		}

		if (const auto rawHandler = CL_RawCallbacks.find(command_); rawHandler != CL_RawCallbacks.end())
		{
			rawHandler->second(address, message);
			return true;
		}

		const auto handler = CL_Callbacks.find(command_);
		if (handler == CL_Callbacks.end())
		{
			// Normal handler was not found, return
			return false;
		}

		const std::string data{ reinterpret_cast<char*>(message->data) + offset, message->cursize - offset };

		auto target = Address{ address };
		handler->second(target, data);
		return true;
	}

	__declspec(naked) void Network::CL_HandleCommandStub()
	{
		__asm
		{
			lea eax, [esp + 0xC54] // address

			pushad

			push ebp // msg_t
			push edi // command name
			push eax // netadr_t pointer
			call CL_HandleCommand
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

		// Maximum size in NET_OutOfBandPrint
		Utils::Hook::Set<std::uint32_t>(0x4AEF08, 0x1FFFC);
		Utils::Hook::Set<std::uint32_t>(0x4AEFA3, 0x1FFFC);

		// Increase max port binding attempts from 10 to 100
		Utils::Hook::Set<std::uint8_t>(0x4FD48A, 100);

		// Increase cl_maxpackets dvar limit
		Utils::Hook::Set<std::uint8_t>(0x4050A1, 125);

		// Increase snaps (disabled for unknown reasons)
		//Utils::Hook::Set<BYTE>(0x405357, 40);

		// Set default value of snaps and cl_maxpackets dvar
		Utils::Hook::Set<std::uint8_t>(0x40535B, 30);
		Utils::Hook::Set<std::uint8_t>(0x4050A5, 125);

		// Parse port as short in Net_AddrToString
		Utils::Hook::Set<const char*>(0x4698E3, "%u.%u.%u.%u:%hu");

		// Prevent recvfrom error spam
		Utils::Hook(0x46531A, PacketErrorCheck, HOOK_JUMP).install()->quick();
		
		// Handle client packets
		Utils::Hook(0x5AA703, CL_HandleCommandStub, HOOK_JUMP).install()->quick();

		// Disable unused OOB packets handlers just to be sure
		Utils::Hook::Set<std::uint8_t>(0x5AA5B6, 0xEB); // CL_SteamServerAuth
		Utils::Hook::Set<std::uint8_t>(0x5AA69F, 0xEB); // echo
		Utils::Hook::Set<std::uint8_t>(0x5AAA82, 0xEB); // SP
		Utils::Hook::Set<std::uint8_t>(0x5A9F77, 0xEB); // CL_WeNowCantHearSomeone
		Utils::Hook::Set<std::uint8_t>(0x5A9F18, 0xEB); // CL_VoiceConnectionTestPacket
		Utils::Hook::Set<std::uint8_t>(0x5A9FF3, 0xEB); // CL_HandleRelayPacket

		// For security reasons check the sender of the 'print' OOB
		Utils::Hook::Set<std::uint8_t>(0x5AA729, 0xEB);

		// Com_GetProtocol
		Utils::Hook::Set<std::uint32_t>(0x4FB501, PROTOCOL);

		// Set the default, min and max of the protocol dvar
		Utils::Hook::Set<std::uint32_t>(0x4D36A9, PROTOCOL);
		Utils::Hook::Set<std::uint32_t>(0x4D36AE, PROTOCOL);
		Utils::Hook::Set<std::uint32_t>(0x4D36B3, PROTOCOL);

		// Internal version is 99, most servers should accept it
		Utils::Hook::Set<std::uint32_t>(0x463C61, 208); // getBuildNumberAsInt

		// LSP disabled
		Utils::Hook::Set<std::uint8_t>(0x435950, 0xC3); // LSP HELLO
		Utils::Hook::Set<std::uint8_t>(0x49C220, 0xC3); // We wanted to send a logging packet, but we haven't connected to LSP!
		Utils::Hook::Set<std::uint8_t>(0x4BD900, 0xC3); // main LSP response func
		Utils::Hook::Set<std::uint8_t>(0x682170, 0xC3); // Telling LSP that we're playing a private match
		Utils::Hook::Nop(0x4FD448, 5); // Don't create lsp_socket

		// Do not run UPNP stuff at all
		Utils::Hook::Set<std::uint8_t>(0x48A135, 0xC3);
		Utils::Hook::Nop(0x48A135 + 1, 4);

		Utils::Hook::Set<std::uint8_t>(0x48A151, 0xC3);
		Utils::Hook::Nop(0x48A151 + 1, 4);

		// Don't spam the console
		Utils::Hook(0x684080, Game::Com_DPrintf, HOOK_CALL).install()->quick();

		// Disable the IWNet IP detection (default 'got ipdetect' flag to 1)
		Utils::Hook::Set<std::uint8_t>(0x649D6F0, 1);

		OnClientPacket("resolveAddress", []([[maybe_unused]] const Address& address, [[maybe_unused]] const std::string& data)
		{
			SendRaw(address, address.getString());
		});

		OnClientPacketRaw("print", [](Game::netadr_t* address, Game::msg_t* msg)
		{
			auto* clc = Game::CL_GetLocalClientConnection(0);
			if (!Game::NET_CompareBaseAdr(clc->serverAddress, *address))
			{
				Logger::Debug("Ignoring stray 'print' network message from '{}'", Game::NET_AdrToString(*address));
				return;
			}

			char printBuf[2048]{};

			const auto* s = Game::MSG_ReadBigString(msg);
			Game::I_strncpyz(clc->serverMessage, s, sizeof(clc->serverMessage));
			Game::Com_sprintf(printBuf, sizeof(printBuf), "%s", s);
			Game::Com_PrintMessage(Game::CON_CHANNEL_CLIENT, printBuf, false);
		});
	}
}
