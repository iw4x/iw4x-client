namespace Components
{
	class Network : public Component
	{
	public:
		class Address
		{
		public:
			Address() {};
			Address(std::string addrString);
			Address(Game::netadr_t addr) : address(addr) {}
			Address(Game::netadr_t* addr) : Address(*addr) {}
			Address(const Address& obj) : address(obj.address) {};
			bool operator!=(const Address &obj) { return !(*this == obj); };
			bool operator==(const Address &obj);

			void SetPort(unsigned short port);
			unsigned short GetPort();

			void SetIP(DWORD ip);
			void SetIP(Game::netIP_t ip);
			Game::netIP_t GetIP();

			void SetType(Game::netadrtype_t type);
			Game::netadrtype_t GetType();

			Game::netadr_t* Get();
			const char* GetString();

			bool IsLocal();
			bool IsSelf();

		private:
			Game::netadr_t address;
		};

		typedef void(*Callback)(Address address, std::string data);

		Network();
		~Network();
		const char* GetName() { return "Network"; };

		static void Handle(std::string packet, Callback callback);

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
		static SOCKET TcpSocket;
		static std::string SelectedPacket;
		static std::map<std::string, Callback> PacketHandlers;
		static int PacketInterceptionHandler(const char* packet);
		static void DeployPacket(Game::netadr_t from, Game::msg_t* msg);
		static void DeployPacketStub();
	};
}
