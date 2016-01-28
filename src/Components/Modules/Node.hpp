namespace Components
{
	class Node : public Component
	{
	public:
		Node();
		~Node();
		const char* GetName() { return "Node"; };

	private:
		enum EntryState
		{
			STATE_UNKNOWN,
			STATE_QUERYING,
			STATE_VALID,
			STATE_INVALID
		};

		struct NodeEntry
		{
			Network::Address address;
			EntryState state;
			int startTime;
			int endTime;
		};

		struct DediEntry
		{
			Network::Address address;
			EntryState state;
			int startTime;
			int endTime;
		};

#pragma pack(push, 1)
		struct AddressEntry
		{
			Game::netIP_t ip;
			unsigned short port;

			Network::Address toNetAddress()
			{
				Network::Address address;

				address.SetIP(this->ip);
				address.SetPort(ntohs(this->port));
				address.SetType(Game::netadrtype_t::NA_IP);

				return address;
			}

			void fromNetAddress(Network::Address address)
			{
				this->ip = address.GetIP();
				this->port = htons(address.GetPort());
			}
		};
#pragma pack(pop)

		static std::vector<NodeEntry> Nodes;
		static std::vector<DediEntry> Dedis;

		static void LoadNodes();
		static void LoadDedis();

		static void StoreNodes();
		static void StoreDedis();

		static void AddNode(Network::Address address, bool valid = false);
		static void AddDedi(Network::Address address);

		static void SendNodeList(Network::Address target);
		static void SendDediList(Network::Address target);
	};
}
