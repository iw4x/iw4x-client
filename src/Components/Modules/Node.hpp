#define HEARTBEAT_DEADLINE 1000 * 60 * 10  // Invalidate servers after 10 minutes without heartbeat
#define HEARTBEAT_INTERVAL 1000 * 60 * 3   // Send heartbeats to each node every 3 minutes

#define NODE_VALIDITY_EXPIRE 1000 * 60 * 2 // Revalidate nodes after 2 minutes
#define DEDI_VALIDITY_EXPIRE 1000 * 60 * 2 // Revalidate dedis after 2 minutes

#define NODE_QUERY_TIMEOUT 1000 * 30 * 1   // Invalidate nodes after 30 seconds without query response
#define DEDI_QUERY_TIMEOUT 1000 * 10 * 1   // Invalidate dedis after 10 seconds without query response

#define HEARTBEATS_FRAME_LIMIT 1           // Limit of heartbeats sent to nodes per frame
#define NODE_FRAME_QUERY_LIMIT 1           // Limit of nodes to be queried per frame
#define DEDI_FRAME_QUERY_LIMIT 1           // Limit of dedis to be queried per frame

namespace Components
{
	class Node : public Component
	{
	public:
		Node();
		~Node();
		const char* GetName() { return "Node"; };

		static void ValidateDedi(Network::Address address, Utils::InfoString info);

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
			int lastTime;
			int lastHeartbeat;
		};

		struct DediEntry
		{
			Network::Address address;
			std::string challenge;
			EntryState state;
			int lastTime;
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
		static void StoreNodes();

		static void AddNode(Network::Address address, bool valid = false);
		static void AddDedi(Network::Address address, bool dirty = false);

		static void SendNodeList(Network::Address target);
		static void SendDediList(Network::Address target);

		static void DeleteInvalidNodes();
		static void DeleteInvalidDedis();
	};
}
