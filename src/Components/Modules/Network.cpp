#include "..\..\STDInclude.hpp"

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
	void Network::Address::SetIP(DWORD ip)
	{
		*(DWORD*)this->address.ip = ip;
	}
	DWORD Network::Address::GetIP()
	{
		return *(DWORD*)this->address.ip;
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

	void Network::Send(Address target, std::string data)
	{
		Network::Send(Game::netsrc_t::NS_CLIENT, target, data);
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
			size_t offset = Network::SelectedPacket.size() + 4 + 1;
			Network::PacketHandlers[Network::SelectedPacket](from, std::string(msg->data + offset, msg->cursize - offset));
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
	}

	Network::~Network()
	{
		Network::SelectedPacket.clear();
		Network::PacketHandlers.clear();
	}
}
