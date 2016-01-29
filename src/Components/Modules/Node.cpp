#include "STDInclude.hpp"

namespace Components
{
	std::vector<Node::NodeEntry> Node::Nodes;
	std::vector<Node::DediEntry> Node::Dedis;

	void Node::LoadNodes()
	{
		std::string nodes = Utils::ReadFile("players/nodes.dat");

		// Invalid
		if (!nodes.size() || nodes.size() % 6) return;

		unsigned int size = (nodes.size() / sizeof(Node::AddressEntry));

		Node::AddressEntry* addresses = reinterpret_cast<Node::AddressEntry*>(const_cast<char*>(nodes.data()));
		for (unsigned int i = 0; i < size; ++i)
		{
			Node::AddNode(addresses[i].toNetAddress());
		}
	}

	void Node::StoreNodes()
	{
		std::vector<Node::AddressEntry> entries;

		for (auto entry : Node::Nodes)
		{
			if (entry.state == Node::STATE_VALID)
			{
				Node::AddressEntry thisAddress;
				thisAddress.fromNetAddress(entry.address);

				entries.push_back(thisAddress);
			}
		}

		std::string nodeStream(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

		CreateDirectoryW(L"players", NULL);
		Utils::WriteFile("players/nodes.dat", nodeStream);
	}

	void Node::AddNode(Network::Address address, bool valid)
	{
//#ifdef DEBUG
		if (address.IsSelf()) return;
// #else
// 		if (address.IsLocal() || address.IsSelf()) return;
// #endif

		Node::NodeEntry entry;

		entry.startTime = 0;
		entry.lastHeartbeat = 0;
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

	void Node::AddDedi(Network::Address address, bool dirty)
	{
		Node::DediEntry entry;

		entry.startTime = 0;
		entry.endTime = 0;
		entry.state = Node::STATE_UNKNOWN;
		entry.address = address;

		// Search if we already know that node
		bool duplicate = false;
		for (auto &ourEntry : Node::Dedis)
		{
			if (ourEntry.address == entry.address)
			{
				if (dirty)
				{
					ourEntry.endTime = Game::Com_Milliseconds();
					ourEntry.state = Node::STATE_UNKNOWN;
				}

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
			if (entry.state == Node::STATE_VALID) // Only send valid nodes, or shall we send invalid ones as well?
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

	void Node::ValidateDedi(Network::Address address, Utils::InfoString info)
	{
		for (auto &dedi : Node::Dedis)
		{
			if (dedi.address == address)
			{
				dedi.state = (info.Get("challenge") == dedi.challenge ? Node::STATE_VALID : Node::STATE_INVALID);
				dedi.endTime = Game::Com_Milliseconds();

				if (dedi.state == Node::STATE_VALID)
				{
					Logger::Print("Validated dedi %s\n", address.GetString());
				}
				break;
			}
		}
	}

	void Node::DeleteInvalidNodes()
	{
		std::vector<Node::NodeEntry> cleanNodes;

		for (auto node : Node::Nodes)
		{
			if (node.state != Node::STATE_INVALID)
			{
				cleanNodes.push_back(node);
			}
			else
			{
				Logger::Print("Removing invalid node %s\n", node.address.GetString());
			}
		}

		if (cleanNodes.size() != Node::Nodes.size())
		{
			Node::Nodes.clear();
			Utils::Merge(&Node::Nodes, cleanNodes);
		}
	}

	void Node::DeleteInvalidDedis()
	{
		std::vector<Node::DediEntry> cleanDedis;

		for (auto dedi : Node::Dedis)
		{
			if (dedi.state != Node::STATE_INVALID)
			{
				cleanDedis.push_back(dedi);
			}
			else
			{
				Logger::Print("Removing invalid dedi %s\n", dedi.address.GetString());
			}
		}

		if (cleanDedis.size() != Node::Dedis.size())
		{
			Node::Dedis.clear();
			Utils::Merge(&Node::Dedis, cleanDedis);
		}
	}

	Node::Node()
	{
//#ifdef USE_NODE_STUFF
		Assert_Size(Node::AddressEntry, 6);

		Dvar::OnInit([] ()
		{
			Node::Dedis.clear();
			Node::Nodes.clear();
			Node::LoadNodes();
		});

		if (Dedicated::IsDedicated())
		{
			QuickPatch::OnShutdown([] ()
			{
				for (auto node : Node::Nodes)
				{
					Network::Send(node.address, "deadline\n");
				}
			});
		}

		Network::Handle("nodeRequestLists", [] (Network::Address address, std::string data)
		{
			Logger::Print("Sending our lists to %s\n", address.GetString());

			// Consider this a node for now!
			//Node::AddNode(address, true);

			Node::SendNodeList(address);
			Node::SendDediList(address);
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

		Network::Handle("heartbeat", [] (Network::Address address, std::string data)
		{
			Logger::Print("Received heartbeat from %s\n", address.GetString());
			Node::AddDedi(address, true);
		});

		Network::Handle("deadline", [] (Network::Address address, std::string data)
		{
			Logger::Print("Invalidation message received from %s\n", address.GetString());

			for (auto &dedi : Node::Dedis)
			{
				if (dedi.address == address)
				{
					dedi.state = Node::STATE_INVALID;
					dedi.endTime = Game::Com_Milliseconds();
				}
			}

			for (auto &node : Node::Nodes)
			{
				if (node.address == address)
				{
					node.state = Node::STATE_INVALID;
					node.endTime = Game::Com_Milliseconds();
				}
			}
		});

		Dedicated::OnFrame([] ()
		{
			int heartbeatCount = 0;
			int count = 0;

			// Send requests
			for (auto &node : Node::Nodes)
			{
				if (count < NODE_FRAME_QUERY_LIMIT && (node.state == Node::STATE_UNKNOWN || (/*node.state != Node::STATE_INVALID && */node.state != Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.endTime) > (NODE_VALIDITY_EXPIRE))))
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
					Node::SendDediList(node.address);
				}

				if (node.state == Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.startTime) > (NODE_QUERY_TIMEOUT))
				{
					node.state = Node::STATE_INVALID;
					node.endTime = Game::Com_Milliseconds();
				}

				if (node.state == Node::STATE_VALID)
				{
					if (heartbeatCount < HEARTBEATS_FRAME_LIMIT && (!node.lastHeartbeat || (Game::Com_Milliseconds() - node.lastHeartbeat) > (HEARTBEAT_INTERVAL)))
					{
						heartbeatCount++;

						Logger::Print("Sending heartbeat to node %s...\n", node.address.GetString());
						node.lastHeartbeat = Game::Com_Milliseconds();
						Network::Send(node.address, "heartbeat\n");
					}
				}
			}

			count = 0;

			for (auto &dedi : Node::Dedis)
			{
				if (count < DEDI_FRAME_QUERY_LIMIT && (dedi.state == Node::STATE_UNKNOWN || (/*node.state != Node::STATE_INVALID && */dedi.state != Node::STATE_QUERYING && (Game::Com_Milliseconds() - dedi.endTime) > (DEDI_VALIDITY_EXPIRE))))
				{
					count++;

					dedi.startTime = Game::Com_Milliseconds();
					dedi.endTime = 0;
					dedi.challenge = Utils::VA("%d", dedi.startTime);
					dedi.state = Node::STATE_QUERYING;

					Logger::Print("Verifying dedi %s...\n", dedi.address.GetString());

					// Request new lists
					Network::Send(dedi.address, Utils::VA("getinfo %s\n", dedi.challenge.data()));
				}

				// No query response
				if (dedi.state == Node::STATE_QUERYING && (Game::Com_Milliseconds() - dedi.startTime) > (DEDI_QUERY_TIMEOUT))
				{
					dedi.state = Node::STATE_INVALID;
					dedi.endTime = Game::Com_Milliseconds();
				}

				// Lack of heartbeats
				if (dedi.state == Node::STATE_VALID && (Game::Com_Milliseconds() - dedi.startTime) > (HEARTBEAT_DEADLINE))
				{
					Logger::Print("Invalidating dedi %s\n", dedi.address.GetString());
					dedi.state = Node::STATE_INVALID;
				}
			}

			Node::DeleteInvalidNodes();
			Node::DeleteInvalidDedis();

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

		Command::Add("listdedis", [] (Command::Params params)
		{
			Logger::Print("Dedi: %d\n", Node::Dedis.size());

			for (auto dedi : Node::Dedis)
			{
				Logger::Print("%s\n", dedi.address.GetString());
			}
		});

		Command::Add("addnode", [](Command::Params params)
		{
			if (params.Length() < 2) return;

			Node::AddNode(Network::Address(params[1]));
		});
//#endif
	}

	Node::~Node()
	{
		Node::StoreNodes();
		Node::Nodes.clear();
	}
}
