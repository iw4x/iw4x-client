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

	void Node::StoreNodes(bool force)
	{
		static int lastStorage = 0;

		// Don't store nodes if the delta is too small and were not forcing it
		if ((Game::Com_Milliseconds() - lastStorage) < NODE_STORE_INTERVAL && !force) return;
		lastStorage = Game::Com_Milliseconds();

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
#ifdef DEBUG
		if (address.IsSelf()) return;
#else
		if (address.IsLocal() || address.IsSelf()) return;
#endif

		Node::NodeEntry entry;

		entry.lastHeard = Game::Com_Milliseconds();
		entry.lastHeartbeat = 0;
		entry.lastTime = (valid ? Game::Com_Milliseconds() : 0);
		entry.state = (valid ? Node::STATE_VALID : Node::STATE_UNKNOWN);
		entry.address = address;

		// Search if we already know that node
		bool duplicate = false;
		for (auto &ourEntry : Node::Nodes)
		{
			if (ourEntry.address == entry.address)
			{
				ourEntry.lastHeard = Game::Com_Milliseconds();

// 				if (ourEntry.state == Node::STATE_INVALID)
// 				{
// 					Logger::Print("Node %s was invalidated, but we still received a reference from another node. Suspicous...\n", address.GetString());
// 				}

								// Validate it
				if (valid)
				{
					ourEntry.state = Node::STATE_VALID;
					ourEntry.lastTime = Game::Com_Milliseconds();
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

		entry.lastHeard = Game::Com_Milliseconds();
		entry.lastTime = 0;
		entry.state = Node::STATE_UNKNOWN;
		entry.address = address;

		// Search if we already know that node
		bool duplicate = false;
		for (auto &ourEntry : Node::Dedis)
		{
			if (ourEntry.address == entry.address)
			{
				ourEntry.lastHeard = Game::Com_Milliseconds();

// 				if (ourEntry.state == Node::STATE_INVALID)
// 				{
// 					Logger::Print("Dedi %s was invalidated, but we still received a reference from another node. Suspicous...\n", address.GetString());
// 				}

				if (dirty)
				{
					ourEntry.lastTime = Game::Com_Milliseconds();
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

			if (entries.size() >= NODE_PACKET_LIMIT)
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

			if (entries.size() >= DEDI_PACKET_LIMIT)
			{
				std::string packet = "nodeDediList\n";
				packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

				Network::SendRaw(target, packet);

				entries.clear();
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
				if (dedi.state == Node::STATE_QUERYING)
				{
					dedi.state = (info.Get("challenge") == dedi.challenge ? Node::STATE_VALID : Node::STATE_INVALID);
					dedi.lastTime = Game::Com_Milliseconds();

					if (dedi.state == Node::STATE_VALID)
					{
						Logger::Print("Validated dedi %s\n", address.GetString());
					}
				}
				break;
			}
		}
	}

	std::vector<Network::Address> Node::GetDediList()
	{
		std::vector<Network::Address> dedis;

		for (auto dedi : Node::Dedis)
		{
			if (dedi.state == Node::STATE_VALID)
			{
				dedis.push_back(dedi.address);
			}
		}

		return dedis;
	}

	void Node::DeleteInvalidNodes()
	{
		std::vector<Node::NodeEntry> cleanNodes;

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_INVALID && (Game::Com_Milliseconds() - node.lastHeard) > NODE_INVALID_DELETE)
			{
				Logger::Print("Removing invalid node %s\n", node.address.GetString());
			}
			else
			{
				cleanNodes.push_back(node);
			}
		}

		if (cleanNodes.size() != Node::Nodes.size())
		{
			Node::Nodes.clear();
			Utils::Merge(&Node::Nodes, cleanNodes);
		}
	}

	void Node::FrameHandler()
	{
		int heartbeatCount = 0;
		int count = 0;

		// Send requests
		for (auto &node : Node::Nodes)
		{
			if (count < NODE_FRAME_QUERY_LIMIT && (node.state == Node::STATE_UNKNOWN || (/*node.state != Node::STATE_INVALID && */node.state != Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.lastTime) >(NODE_VALIDITY_EXPIRE))))
			{
				count++;

				node.lastTime = Game::Com_Milliseconds();
				node.state = Node::STATE_QUERYING;

				Logger::Print("Syncing with node %s...\n", node.address.GetString());

				// Request new lists
				Network::Send(node.address, "nodeRequestLists");

				// Send our lists (only if dedi)
				if (Dedicated::IsDedicated())
				{
					Node::SendNodeList(node.address);
					Node::SendDediList(node.address);
				}
			}

			if (node.state == Node::STATE_QUERYING && (Game::Com_Milliseconds() - node.lastTime) > (NODE_QUERY_TIMEOUT))
			{
				node.state = Node::STATE_INVALID;
				node.lastTime = Game::Com_Milliseconds();
			}

			if (Dedicated::IsDedicated() && node.state == Node::STATE_VALID)
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
			if (count < DEDI_FRAME_QUERY_LIMIT && (dedi.state == Node::STATE_UNKNOWN || (/*node.state != Node::STATE_INVALID && */dedi.state != Node::STATE_QUERYING && (Game::Com_Milliseconds() - dedi.lastTime) >(DEDI_VALIDITY_EXPIRE))))
			{
				count++;

				dedi.lastTime = Game::Com_Milliseconds();
				dedi.challenge = Utils::VA("%d", Utils::Cryptography::Rand::GenerateInt());
				dedi.state = Node::STATE_QUERYING;

				Logger::Print("Verifying dedi %s...\n", dedi.address.GetString());

				// Request new lists
				Network::Send(dedi.address, Utils::VA("getinfo %s\n", dedi.challenge.data()));
			}

			// No query response
			if (dedi.state == Node::STATE_QUERYING && (Game::Com_Milliseconds() - dedi.lastTime) > (DEDI_QUERY_TIMEOUT))
			{
				dedi.state = Node::STATE_INVALID;
				dedi.lastTime = Game::Com_Milliseconds();
			}

			// Lack of heartbeats
			if (dedi.state == Node::STATE_VALID && (Game::Com_Milliseconds() - dedi.lastTime) > (HEARTBEAT_DEADLINE))
			{
				Logger::Print("Invalidating dedi %s\n", dedi.address.GetString());
				dedi.state = Node::STATE_INVALID;
			}
		}

		Node::DeleteInvalidNodes();
		Node::DeleteInvalidDedis();
		Node::StoreNodes(false);
	}

	void Node::DeleteInvalidDedis()
	{
		std::vector<Node::DediEntry> cleanDedis;

		for (auto dedi : Node::Dedis)
		{
			if (dedi.state == Node::STATE_INVALID && (Game::Com_Milliseconds() - dedi.lastHeard) > DEDI_INVALID_DELETE)
			{
				Logger::Print("Removing invalid dedi %s\n", dedi.address.GetString());
			}
			else
			{
				cleanDedis.push_back(dedi);
			}
		}

		if (cleanDedis.size() != Node::Dedis.size())
		{
			Node::Dedis.clear();
			Utils::Merge(&Node::Dedis, cleanDedis);
		}
	}

	const char* Node::GetStateName(EntryState state)
	{
		switch (state)
		{
		case Node::STATE_UNKNOWN:
			return "Unknown";

		case Node::STATE_QUERYING:
			return "Querying";

		case Node::STATE_INVALID:
			return "Invalid";

		case Node::STATE_VALID:
			return "Valid";
		}

		return "";
	}

	Node::Node()
	{
		Assert_Size(Node::AddressEntry, 6);

		// ZoneBuilder doesn't require node stuff
		if (ZoneBuilder::IsEnabled()) return;

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

		// Only dedis act as nodes!
		if (Dedicated::IsDedicated())
		{
			Network::Handle("nodeRequestLists", [] (Network::Address address, std::string data)
			{
				Logger::Print("Sending our lists to %s\n", address.GetString());

				Node::SendNodeList(address);
				Node::SendDediList(address);

				// Send our heartbeat as well :P
				// Otherwise, if there's only 1 node in the network (us), we might not get listed as dedi
				Network::Send(address, "heartbeat\n");
			});
		}

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
					dedi.lastTime = Game::Com_Milliseconds();
				}
			}

			for (auto &node : Node::Nodes)
			{
				if (node.address == address)
				{
					node.state = Node::STATE_INVALID;
					node.lastTime = Game::Com_Milliseconds();
				}
			}
		});

		Command::Add("listnodes", [] (Command::Params params)
		{
			Logger::Print("Nodes: %d\n", Node::Nodes.size());

			for (auto node : Node::Nodes)
			{
				Logger::Print("%s\t(%s)\n", node.address.GetString(), Node::GetStateName(node.state));
			}
		});

		Command::Add("listdedis", [] (Command::Params params)
		{
			Logger::Print("Dedi: %d\n", Node::Dedis.size());

			for (auto dedi : Node::Dedis)
			{
				Logger::Print("%s\t(%s)\n", dedi.address.GetString(), Node::GetStateName(dedi.state));
			}
		});

		Command::Add("addnode", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			Network::Address address(params[1]);
			Node::AddNode(address);

			// Invalidate it
			for (auto &node : Node::Nodes)
			{
				if (node.address == address)
				{
					node.state = Node::STATE_UNKNOWN;
					break;
				}
			}
		});

		Command::Add("syncnodes", [] (Command::Params params)
		{
			for (auto &node : Node::Nodes)
			{
				if (node.state != Node::STATE_INVALID)
				{
					node.state = Node::STATE_UNKNOWN;
				}
			}
		});

		Command::Add("syncdedis", [] (Command::Params params)
		{
			for (auto &dedi : Node::Dedis)
			{
				if (dedi.state != Node::STATE_INVALID)
				{
					dedi.state = Node::STATE_UNKNOWN;
				}
			}
		});

		// Install frame handlers
		Dedicated::OnFrame(Node::FrameHandler);
		Renderer::OnFrame(Node::FrameHandler);
	}

	Node::~Node()
	{
		Node::StoreNodes(true);
		Node::Nodes.clear();
		Node::Dedis.clear();
	}
}
