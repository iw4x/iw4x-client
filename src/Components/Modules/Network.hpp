#pragma once

namespace Components
{
	class Network : public Component
	{
	public:
		class Address
		{
		public:
			Address();
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
			[[nodiscard]] unsigned short getPort() const;

			void setIP(DWORD ip);
			void setIP(Game::netIP_t ip);
			[[nodiscard]] Game::netIP_t getIP() const;

			void setType(Game::netadrtype_t type);
			[[nodiscard]] Game::netadrtype_t getType() const;

			[[nodiscard]] sockaddr getSockAddr();
			void toSockAddr(sockaddr* addr);
			void toSockAddr(sockaddr_in* addr);
			[[nodiscard]] const Game::netadr_t* get() const noexcept;
			[[nodiscard]] const char* getCString() const;
			[[nodiscard]] std::string getString() const;

			[[nodiscard]] bool isLocal() const noexcept;
			[[nodiscard]] bool isSelf()  const noexcept;
			[[nodiscard]] bool isValid() const noexcept;
			[[nodiscard]] bool isLoopback() const noexcept;

		private:
			Game::netadr_t address;
		};

		using networkCallback = std::function<void(Address&, const std::string&)>;
		using networkRawCallback = std::function<void(Game::netadr_t*, Game::msg_t* msg)>;

		Network();

		static std::uint16_t GetPort();
		
		// Send quake-styled binary data
		static void Send(const Address& target, const std::string& data);
		static void Send(Game::netsrc_t type, const Address& target, const std::string& data);

		// Allows sending raw data without quake header
		static void SendRaw(const Address& target, const std::string& data);
		static void SendRaw(Game::netsrc_t type, const Address& target, const std::string& data);

		// Send quake-style command using binary data
		static void SendCommand(const Address& target, const std::string& command, const std::string& data = {});
		static void SendCommand(Game::netsrc_t type, const Address& target, const std::string& command, const std::string& data = {});

		static void Broadcast(unsigned short port, const std::string& data);
		static void BroadcastRange(unsigned int min, unsigned int max, const std::string& data);
		static void BroadcastAll(const std::string& data);

		static void OnClientPacket(const std::string& command, const networkCallback& callback);
		static void OnClientPacketRaw(const std::string& command, const networkRawCallback& callback);

	private:
		static std::unordered_map<std::string, networkCallback> CL_Callbacks;
		static std::unordered_map<std::string, networkRawCallback> CL_RawCallbacks;

		static void PacketErrorCheck();

		static bool CL_HandleCommand(Game::netadr_t* address, const char* command, Game::msg_t* message);

		static void CL_HandleCommandStub();
	};
}

template <>
struct std::hash<Components::Network::Address>
{
	std::size_t operator()(const Components::Network::Address& x) const noexcept
	{
		return std::hash<std::uint32_t>()(*reinterpret_cast<const std::uint32_t*>(&x.getIP().bytes[0])) ^ std::hash<std::uint16_t>()(x.getPort());
	}
};
