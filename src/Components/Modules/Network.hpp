#pragma once

#define NETWORK_MAX_PACKETS_PER_SECOND 100'000

namespace Components
{
	class Network : public Component
	{
	public:
		class Address
		{
		public:
			Address() { setType(Game::netadrtype_t::NA_BAD); };
			Address(std::string addrString);
			Address(sockaddr* addr);
			Address(sockaddr addr) : Address(&addr) {}
			Address(sockaddr_in addr) : Address(&addr) {}
			Address(sockaddr_in* addr) : Address(reinterpret_cast<sockaddr*>(addr)) {}
			Address(Game::netadr_t addr) : address(addr) {}
			Address(Game::netadr_t* addr) : Address(*addr) {}
			Address(const Address& obj) : address(obj.address) {};
			Address(const Proto::Network::Address& addr) { this->deserialize(addr); };
			bool operator!=(const Address &obj) { return !(*this == obj); };
			bool operator==(const Address &obj);

			void setPort(unsigned short port);
			unsigned short getPort();

			void setIP(DWORD ip);
			void setIP(Game::netIP_t ip);
			Game::netIP_t getIP();

			void setType(Game::netadrtype_t type);
			Game::netadrtype_t getType();

			sockaddr getSockAddr();
			void toSockAddr(sockaddr* addr);
			void toSockAddr(sockaddr_in* addr);
			Game::netadr_t* get();
			const char* getCString();
			std::string getString();

			bool isLocal();
			bool isSelf();
			bool isValid();
			bool isLoopback();

			void serialize(Proto::Network::Address* protoAddress);
			void deserialize(const Proto::Network::Address& protoAddress);

		private:
			Game::netadr_t address;
		};

		typedef void(Callback)(Address address, std::string data);
		typedef void(CallbackRaw)();

		Network();
		~Network();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Network"; };
#endif

		static void Handle(std::string packet, Utils::Slot<Callback> callback);
		static void OnStart(Utils::Slot<CallbackRaw> callback);

		// Send quake-styled binary data
		static void Send(Address target, std::string data);
		static void Send(Game::netsrc_t type, Address target, std::string data);

		// Allows sending raw data without quake header
		static void SendRaw(Address target, std::string data);
		static void SendRaw(Game::netsrc_t type, Address target, std::string data);

		// Send quake-style command using binary data
		static void SendCommand(Address target, std::string command, std::string data = "");
		static void SendCommand(Game::netsrc_t type, Address target, std::string command, std::string data = "");

		static void Broadcast(unsigned short port, std::string data);
		static void BroadcastRange(unsigned int min, unsigned int max, std::string data);
		static void BroadcastAll(std::string data);

	private:
		static std::string SelectedPacket;
		static Utils::Signal<CallbackRaw> StartupSignal;
		static std::map<std::string, Utils::Slot<Callback>> PacketHandlers;

		static int PacketInterceptionHandler(const char* packet);
		static void DeployPacket(Game::netadr_t* from, Game::msg_t* msg);
		static void DeployPacketStub();

		static void NetworkStart();
		static void NetworkStartStub();

		static void PacketErrorCheck();
	};
}
