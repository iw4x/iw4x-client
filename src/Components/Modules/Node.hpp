#define NODE_QUERY_INTERVAL    1000 * 60 * 2  // Query nodelist from nodes evry 2 minutes
#define NODE_QUERY_TIMEOUT     1000 * 30 * 1  // Invalidate nodes after 30 seconds without query response
#define NODE_INVALID_DELETE    1000 * 60 * 10 // Delete invalidated nodes after 10 minutes
#define NODE_FRAME_QUERY_LIMIT 1              // Limit of nodes to be queried per frame
#define NODE_PACKET_LIMIT      111            // Send 111 nodes per synchronization packet
#define NODE_STORE_INTERVAL    1000 * 60* 1   // Store nodes every minute
#define SESSION_TIMEOUT        1000 * 10      // 10 seconds session timeout

namespace Components
{
	class Node : public Component
	{
	public:
		Node();
		~Node();
		const char* GetName() { return "Node"; };
		bool UnitTest();

		static void SyncNodeList();
		static void AddNode(Network::Address address);

	private:
		enum EntryState
		{
			STATE_UNKNOWN,
			STATE_NEGOTIATING,
			STATE_VALID,
			STATE_INVALID,
		};

		struct NodeEntry
		{
			Network::Address address;
			std::string challenge;
			Utils::Cryptography::ECDSA::Key publicKey;
			EntryState state;

			bool registered;       // Do we consider this node as registered?

			int lastTime;          // Last time we heard anything from the server itself
			int lastHeard;         // Last time we heard something of the server at all (refs form other nodes)
			int lastListQuery;     // Last time we got the list of the node

			// This is only relevant for clients
			bool isDedi;
		};

		struct ClientSession
		{
			Network::Address address;
			std::string challenge;
			bool valid;
			//bool terminated; // Sessions can't explicitly be terminated, they can only timeout
			int lastTime;
		};

		static Utils::Cryptography::ECDSA::Key SignatureKey;

		static std::vector<NodeEntry> Nodes;
		static std::vector<ClientSession> Sessions;

		static void LoadNodes();
		static void StoreNodes(bool force);

		static void SendNodeList(Network::Address address);
		static NodeEntry* FindNode(Network::Address address); 
		static ClientSession* FindSession(Network::Address address);

		static void DeleteInvalidNodes();
		static void DeleteInvalidSessions();

		static void FrameHandler();

		static const char* GetStateName(EntryState state);
	};
}
