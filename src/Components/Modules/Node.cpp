#include "STDInclude.hpp"

namespace Components
{
	std::vector<Node::NodeEntry> Node::Nodes;
	std::vector<Node::DediEntry> Node::Dedis;

	void Node::LoadNodes()
	{
		std::string nodes = Utils::ReadFile("nodes.txt");
		auto list = Utils::Explode(nodes, '\n');

		for (auto entry : list)
		{
			Network::Address addr(entry);
			Node::AddNode(addr);
		}
	}

	void Node::LoadDedis()
	{
		
	}

	void Node::StoreNodes()
	{
		std::string nodes;

		for (auto node : Node::Nodes)
		{
			nodes.append(node.address.GetString());
			nodes.append("\n");
		}

		Utils::WriteFile("nodes.txt", nodes);
	}

	void Node::StoreDedis()
	{

	}

	void Node::AddNode(Network::Address address, bool valid)
	{
		//if (address.IsLocal() || address.IsSelf()) return;

		Node::NodeEntry entry;

		entry.startTime = 0;
		entry.endTime = (valid ? Game::Com_Milliseconds() : 0);
		entry.state = (valid ? Node::STATE_VALID : Node::STATE_UNKNOWN);
		entry.address = address;

		// Search if we already know that node
		bool duplicate = false;
		for (auto &ourEntry : Node::Nodes)
		{
			if (ourEntry.address == entry.address)
			{
				// Validate it
				if (valid)
				{
					ourEntry.state = Node::STATE_VALID;
					ourEntry.endTime = Game::Com_Milliseconds();
				}

				duplicate = true;
				break;
			}
		}

		// Insert if we don't
		if (!duplicate)
		{
			Node::Nodes.push_back(entry);
		}
	}

	void Node::AddDedi(Network::Address address)
	{
		Node::DediEntry entry;

		entry.startTime = 0;
		entry.endTime = 0;
		entry.state = Node::STATE_UNKNOWN;
		entry.address = address;

		// Search if we already know that node
		bool duplicate = false;
		for (auto ourEntry : Node::Nodes)
		{
			if (ourEntry.address == entry.address)
			{
				duplicate = true;
				break;
			}
		}

		// Insert if we don't
		if (!duplicate)
		{
			Node::Dedis.push_back(entry);
		}
	}

	void Node::SendNodeList(Network::Address target)
	{
		if (target.IsSelf()) return;

		std::vector<Node::AddressEntry> entries;

		for (auto entry : Node::Nodes)
		{
			if (entry.state != Node::STATE_INVALID) // Only send valid nodes, or shall we send invalid ones as well?
			{
				Node::AddressEntry thisAddress;
				thisAddress.fromNetAddress(entry.address);

				entries.push_back(thisAddress);
			}

			if (entries.size() >= 111)
			{
				std::string packet = "nodeNodeList\n";
				packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

				Network::SendRaw(target, packet);

				entries.clear();
			}
		}

		std::string packet = "nodeNodeList\n";
		packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

		Network::SendRaw(target, packet);
	}

	void Node::SendDediList(Network::Address target)
	{
		if (target.IsSelf()) return;

		std::vector<Node::AddressEntry> entries;

		for (auto entry : Node::Dedis)
		{
			if (entry.state == Node::STATE_VALID) // Only send valid dedis
			{
				Node::AddressEntry thisAddress;
				thisAddress.fromNetAddress(entry.address);

				entries.push_back(thisAddress);
			}
		}

		std::string packet = "nodeDediList\n";
		packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

		Network::SendRaw(target, packet);
	}

	Node::Node()
	{
#ifdef USE_NODE_STUFF
		Assert_Size(Node::AddressEntry, 6);

		Dvar::OnInit([] ()
		{
			Node::Nodes.clear();
			Node::LoadNodes();

			Node::Dedis.clear();
			Node::LoadDedis();
		});

		Network::Handle("nodeRequestLists", [] (Network::Address address, std::string data)
		{
			Logger::Print("Sending our lists to %s\n", address.GetString());

			// Consider this a node for now!
			//Node::AddNode(address, true);

			Node::SendNodeList(address);
			//Node::SendDediList(address);
		});

		Network::Handle("nodeNodeList", [] (Network::Address address, std::string data)
		{
			if (data.size() % sizeof(Node::AddressEntry))
			{
				Logger::Print("Received invalid node list from %s!\n", address.GetString());
				return;
			}

			unsigned int size = (data.size() / sizeof(Node::AddressEntry));

			Logger::Print("Received valid node list with %d entries from %s\n", size, address.GetString());

			// Insert the node itself and mark it as valid
			Node::AddNode(address, true);

			Node::AddressEntry* addresses = reinterpret_cast<Node::AddressEntry*>(const_cast<char*>(data.data()));
			for (unsigned int i = 0; i < size; ++i)
			{
				Node::AddNode(addresses[i].toNetAddress());
			}
		});

		Network::Handle("nodeDediList", [] (Network::Address address, std::string data)
		{
			if (data.size() % sizeof(Node::AddressEntry))
			{
				Logger::Print("Received invalid dedi list from %s!\n", address.GetString());
				return;
			}

			unsigned int size = (data.size() / sizeof(Node::AddressEntry));

			Logger::Print("Received valid dedi list with %d entries from %s\n", size, address.GetString());

			// Insert the node and mark it as valid
			Node::AddNode(address, true);

			Node::AddressEntry* addresses = reinterpret_cast<Node::AddressEntry*>(const_cast<char*>(data.data()));
			for (unsigned int i = 0; i < size; ++i)
			{
				Node::AddDedi(addresses[i].toNetAddress());
			}
		});

		Dedicated::OnFrame([] ()
		{
			int count = 0;

			// Send requests
			for (auto &node : Node::Nodes)
			{
				// Frame limit
				if (count >= 1) break; // Query only 1 node per frame (-> 3 packets sent per frame)

				if (node.state == Node::STATE_UNKNOWN || (/*node.state != Node::STATE_INVALID && */node.state != Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.endTime) > (1000 * 30)))
				{
					count++;

					node.startTime = Game::Com_Milliseconds();
					node.endTime = 0;
					node.state = Node::STATE_QUERYING;

					Logger::Print("Syncing with node %s...\n", node.address.GetString());

					// Request new lists
					Network::Send(node.address, "nodeRequestLists");

					// Send our lists
					Node::SendNodeList(node.address);
					//Node::SendDediList(node.address);
				}
			}

			// Mark invalid nodes
			for (auto &node : Node::Nodes)
			{
				if (node.state == Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.startTime) > (1000 * 10))
				{
					node.state = Node::STATE_INVALID;
					node.endTime = Game::Com_Milliseconds();
				}
			}

			count = 0;
		});

		Command::Add("listnodes", [] (Command::Params params)
		{
			Logger::Print("Nodes: %d\n", Node::Nodes.size());

			for (auto node : Node::Nodes)
			{
				Logger::Print("%s\n", node.address.GetString());
			}
		});

		Command::Add("addnode", [](Command::Params params)
		{
			if (params.Length() < 2) return;

			Node::AddNode(Network::Address(params[1]));
		});
#endif
	}

	Node::~Node()
	{
		Node::StoreNodes();
		Node::Nodes.clear();

		Node::StoreDedis();
		Node::Dedis.clear();
	}
}
