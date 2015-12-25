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
			Address(const Address& obj) { this->address = obj.address; };
			bool operator!=(const Address &obj) { return !(*this == obj); };
			bool operator==(const Address &obj);

			void SetPort(unsigned short port);
			unsigned short GetPort();
			Game::netadr_t* Get();
			const char* GetString();


		private:
			Game::netadr_t address;
		};

		typedef void(*Callback)(Address address, std::string data);

		Network();
		~Network();
		const char* GetName() { return "Network"; };

		static void Handle(std::string packet, Callback callback);
		static void Send(Game::netsrc_t type, Address target, std::string data);

	private:
		static std::string SelectedPacket;
		static std::map<std::string, Callback> PacketHandlers;
		static int PacketInterceptionHandler(const char* packet);
		static void DeployPacket(Game::netadr_t from, Game::msg_t* msg);
		static void DeployPacketStub();
	};
}
