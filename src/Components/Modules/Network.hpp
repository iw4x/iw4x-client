#pragma once

namespace Components
{
	class Network : public Component
	{
	public:
		class Address
		{
		public:
			Address() { setType(Game::netadrtype_t::NA_BAD); }
			Address(const std::string& addrString);
			Address(sockaddr* addr);
			Address(sockaddr addr) : Address(&addr) {}
			Address(sockaddr_in addr) : Address(&addr) {}
			Address(sockaddr_in* addr) : Address(reinterpret_cast<sockaddr*>(addr)) {}
			Address(Game::netadr_t addr) : address(addr) {}
			Address(Game::netadr_t* addr) : Address(*addr) {}
			Address(const Address& obj) = default;
			bool operator!=(const Address &obj) const { return !(*this == obj); }
			bool operator==(const Address &obj) const;

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
			const char* getCString() const;
			std::string getString() const;

			bool isLocal();
			bool isSelf();
			bool isValid();
			bool isLoopback();

		private:
			Game::netadr_t address;
		};

		typedef void(CallbackRaw)();

		using NetworkCallback = std::function<void(Address&, const std::string&)>;

		Network();

		static unsigned short GetPort();

		static void OnStart(Utils::Slot<CallbackRaw> callback);
		
		// Send quake-styled binary data
		static void Send(Address target, const std::string& data);
		static void Send(Game::netsrc_t type, Address target, const std::string& data);

		// Allows sending raw data without quake header
		static void SendRaw(Address target, const std::string& data);
		static void SendRaw(Game::netsrc_t type, Address target, const std::string& data);

		// Send quake-style command using binary data
		static void SendCommand(Address target, const std::string& command, const std::string& data = "");
		static void SendCommand(Game::netsrc_t type, Address target, const std::string& command, const std::string& data = "");

		static void Broadcast(unsigned short port, const std::string& data);
		static void BroadcastRange(unsigned int min, unsigned int max, const std::string& data);
		static void BroadcastAll(const std::string& data);

		static void OnPacket(const std::string& command, const NetworkCallback& callback);

	private:
		static std::string SelectedPacket;
		static Utils::Signal<CallbackRaw> StartupSignal;
		static std::unordered_map<std::string, NetworkCallback> Callbacks;

		static void NetworkStart();
		static void NetworkStartStub();

		static void PacketErrorCheck();

		static void SV_ExecuteClientMessageStub(Game::client_t* client, Game::msg_t* msg);

		static bool HandleCommand(Game::netadr_t* address, const char* command, const Game::msg_t* message);

		static void CL_HandleCommandStub();
	};
}

template <>
struct std::hash<Components::Network::Address>
{
	std::size_t operator()(const Components::Network::Address& k) const
	{
		return (std::hash<std::string>()(k.getString()));
	}
};
