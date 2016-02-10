#include "STDInclude.hpp"

namespace Components
{
	Utils::Cryptography::ECDSA::Key Node::SignatureKey;
	std::vector<Node::NodeEntry> Node::Nodes;
	std::vector<Node::ClientSession> Node::Sessions;

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

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered)
			{
				Node::AddressEntry thisAddress;
				thisAddress.fromNetAddress(node.address);

				entries.push_back(thisAddress);
			}
		}

		std::string nodeStream(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

		CreateDirectoryW(L"players", NULL);
		Utils::WriteFile("players/nodes.dat", nodeStream);
	}

	Node::NodeEntry* Node::FindNode(Network::Address address)
	{
		for (auto i = Node::Nodes.begin(); i != Node::Nodes.end(); ++i)
		{
			if (i->address == address)
			{
				// I don't know if that's safe, but we'll see that later...
				return &*i;
			}
		}

		return nullptr;
	}
	Node::ClientSession* Node::FindSession(Network::Address address)
	{
		for (auto i = Node::Sessions.begin(); i != Node::Sessions.end(); ++i)
		{
			if (i->address == address)
			{
				// I don't know if that's safe, but we'll see that later...
				return &*i;
			}
		}

		return nullptr;
	}

	void Node::AddNode(Network::Address address)
	{
#ifdef DEBUG
		if (address.IsSelf()) return;
#else
		if (address.IsLocal() || address.IsSelf()) return;
#endif

		Node::NodeEntry* existingEntry = Node::FindNode(address);
		if (existingEntry)
		{
			existingEntry->lastHeard = Game::Com_Milliseconds();
		}
		else
		{
			Node::NodeEntry entry;

			entry.lastHeard = Game::Com_Milliseconds();
			entry.lastTime = 0;
			entry.lastListQuery = 0;
			entry.registered = false;
			entry.state = Node::STATE_UNKNOWN;
			entry.address = address;

			Node::Nodes.push_back(entry);
		}
	}

	void Node::SendNodeList(Network::Address address)
	{
		if (address.IsSelf()) return;

		std::vector<Node::AddressEntry> entries;

		for (auto entry : Node::Nodes)
		{
			if (entry.state == Node::STATE_VALID && entry.registered)
			{
				Node::AddressEntry thisAddress;
				thisAddress.fromNetAddress(entry.address);

				entries.push_back(thisAddress);
			}

			if (entries.size() >= NODE_PACKET_LIMIT)
			{
				std::string packet = "nodeListResponse\n";
				packet.append((Dedicated::IsDedicated() ? "\x01" : "\0"), 1);
				packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

				Network::SendRaw(address, packet);

				entries.clear();
			}
		}

		std::string packet = "nodeListResponse\n";
		packet.append((Dedicated::IsDedicated() ? "\x01" : "\0"), 1);
		packet.append(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(Node::AddressEntry));

		Network::SendRaw(address, packet);
	}

	void Node::DeleteInvalidSessions()
	{
		std::vector<Node::ClientSession> cleanSessions;

		for (auto session : Node::Sessions)
		{
			if (session.lastTime > 0 && (Game::Com_Milliseconds() - session.lastTime) <= SESSION_TIMEOUT)
			{
				cleanSessions.push_back(session);
			}
		}

		if (cleanSessions.size() != Node::Sessions.size())
		{
			//Node::Sessions.clear();
			//Utils::Merge(&Node::Sessions, cleanSessions);
			Node::Sessions = cleanSessions;
		}
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
			//Node::Nodes.clear();
			//Utils::Merge(&Node::Nodes, cleanNodes);
			Node::Nodes = cleanNodes;
		}
	}

	std::vector<Network::Address> Node::GetDediList()
	{
		std::vector<Network::Address> dedis;

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered && node.isDedi)
			{
				dedis.push_back(node.address);
			}
		}

		return dedis;
	}

	void Node::FrameHandler()
	{
		int registerCount = 0;
		int listQueryCount = 0;

		for (auto &node : Node::Nodes)
		{
			// TODO: Decide how to handle nodes that were already registered, but timed out re-registering.
			if (node.state == STATE_NEGOTIATING && (Game::Com_Milliseconds() - node.lastTime) > (NODE_QUERY_TIMEOUT))
			{
				node.registered = false; // Definitely unregister here!
				node.state = Node::STATE_INVALID;
				node.lastHeard = Game::Com_Milliseconds();
				node.lastTime = Game::Com_Milliseconds();

				Logger::Print("Node negotiation timed out. Invalidating %s\n", node.address.GetString());
			}

			if (registerCount < NODE_FRAME_QUERY_LIMIT)
			{
				// Register when unregistered and in UNKNOWN state (I doubt it's possible to be unregistered and in VALID state)
				if (!node.registered && (node.state != Node::STATE_NEGOTIATING && node.state != Node::STATE_INVALID))
				{
					registerCount++;
					node.state = Node::STATE_NEGOTIATING;
					node.lastTime = Game::Com_Milliseconds();

					if (Dedicated::IsDedicated())
					{
						node.challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

						std::string data;
						Proto::NodePacket packet;
						packet.set_challenge(node.challenge);
						packet.SerializePartialToString(&data);

						Logger::Print("Sending registration request to %s\n", node.address.GetString());
						Network::SendRaw(node.address, "nodeRegisterRequest\n" + data);
					}
					else
					{
						Logger::Print("Sending session request to %s\n", node.address.GetString());
						Network::Send(node.address, "sessionRequest\n");
					}
				}
			}

			if (listQueryCount < NODE_FRAME_QUERY_LIMIT)
			{
				if (node.registered && node.state == Node::STATE_VALID && (!node.lastListQuery || (Game::Com_Milliseconds() - node.lastListQuery) > NODE_QUERY_INTERVAL))
				{
					listQueryCount++;
					node.state = Node::STATE_NEGOTIATING;
					node.lastTime = Game::Com_Milliseconds();

					if (Dedicated::IsDedicated())
					{
						Network::Send(node.address, "nodeListRequest\n");
					}
					else
					{
						Network::Send(node.address, "sessionRequest\n");
					}
				}
			}
		}

		Node::DeleteInvalidSessions();
		Node::DeleteInvalidNodes();
		Node::StoreNodes(false);
	}

	const char* Node::GetStateName(EntryState state)
	{
		switch (state)
		{
		case Node::STATE_UNKNOWN:
			return "Unknown";

		case Node::STATE_NEGOTIATING:
			return "Negotiating";

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

		// Generate our ECDSA key
		Node::SignatureKey = Utils::Cryptography::ECDSA::GenerateKey(512);

		// Load stored nodes
		Dvar::OnInit([] ()
		{
			Node::Nodes.clear();
			Node::LoadNodes();
		});

		// Send deadline when shutting down
		if (Dedicated::IsDedicated())
		{
			QuickPatch::OnShutdown([] ()
			{
				std::string data, challenge;
				challenge = Utils::VA("X", Utils::Cryptography::Rand::GenerateInt());

				Proto::NodePacket packet;
				packet.set_challenge(challenge);
				packet.set_signature(Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, challenge));
				packet.SerializePartialToString(&data);

				for (auto node : Node::Nodes)
				{
					Network::SendRaw(node.address, "nodeDeregister\n" + data);
				}
			});

			// This is the handler that accepts registration requests from other nodes
			// If you want to get accepted as node, you have to send a request to this handler
			Network::Handle("nodeRegisterRequest", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);

				// Create a new entry, if we don't already know it
				if (!entry)
				{
					Node::AddNode(address);
					entry = Node::FindNode(address);
					if (!entry) return;
				}

				Logger::Print("Received registration request from %s\n", address.GetString());

				Proto::NodePacket packet;
				if (!packet.ParseFromString(data)) return;
				if (!packet.has_challenge()) return;

				std::string response;
				std::string signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, packet.challenge());
				std::string challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

				packet.Clear();
				packet.set_challenge(challenge);
				packet.set_signature(signature);
				packet.set_publickey(Node::SignatureKey.GetPublicKey());
				packet.SerializeToString(&response);

				entry->lastTime = Game::Com_Milliseconds();
				entry->challenge = challenge;
				entry->state = Node::STATE_NEGOTIATING;

				Network::SendRaw(address, "nodeRegisterSynchronize\n" + response);
			});

			Network::Handle("nodeRegisterSynchronize", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

				Logger::Print("Received synchronization data for registration from %s!\n", address.GetString());

				Proto::NodePacket packet;
				if (!packet.ParseFromString(data)) return;
				if (!packet.has_challenge()) return;
				if (!packet.has_publickey()) return;
				if (!packet.has_signature()) return;

				std::string challenge = packet.challenge();
				std::string publicKey = packet.publickey();
				std::string signature = packet.signature();

				// Verify signature
				entry->publicKey.Set(publicKey);
				if (!Utils::Cryptography::ECDSA::VerifyMessage(entry->publicKey, entry->challenge, signature))
				{
					Logger::Print("Signature from %s for challenge '%s' is invalid!\n", address.GetString(), entry->challenge.data());
					return;
				}

				Logger::Print("Signature from %s for challenge '%s' is valid!\n", address.GetString(), entry->challenge.data());

				// Mark as registered
				entry->lastTime = Game::Com_Milliseconds();
				entry->state = Node::STATE_VALID;
				entry->registered = true;

				Logger::Print("Node %s registered\n", address.GetString());

				// Build response
				data.clear();
				publicKey = Node::SignatureKey.GetPublicKey();
				signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, challenge);

				packet.Clear();
				packet.set_signature(signature);
				packet.set_publickey(publicKey);
				packet.SerializePartialToString(&data);

				Network::SendRaw(address, "nodeRegisterAcknowledge\n" + data);
			});

			Network::Handle("nodeRegisterAcknowledge", [] (Network::Address address, std::string data)
			{
				// Ignore requests from nodes we don't know
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

				Logger::Print("Received acknowledgment from %s\n", address.GetString());

				Proto::NodePacket packet;
				if (!packet.ParseFromString(data)) return;
				if (!packet.has_signature()) return;
				if (!packet.has_publickey()) return;

				std::string publicKey = packet.publickey();
				std::string signature = packet.signature();

				entry->publicKey.Set(publicKey);

				if (Utils::Cryptography::ECDSA::VerifyMessage(entry->publicKey, entry->challenge, signature))
				{
					entry->lastTime = Game::Com_Milliseconds();
					entry->state = Node::STATE_VALID;
					entry->registered = true;

					Logger::Print("Signature from %s for challenge '%s' is valid!\n", address.GetString(), entry->challenge.data());
					Logger::Print("Node %s registered\n", address.GetString());
				}
				else
				{
					Logger::Print("Signature from %s for challenge '%s' is invalid!\n", address.GetString(), entry->challenge.data());
				}
			});

			Network::Handle("nodeListRequest", [] (Network::Address address, std::string data)
			{
				// Requesting a list is either possible, by being registered as node
				// Or having a valid client session
				// Client sessions do expire after some time or when having received a list
				bool allowed = false;
				Node::NodeEntry* entry = Node::FindNode(address);
				if (entry && entry->registered)
				{
					entry->lastTime = Game::Com_Milliseconds();
					allowed = true;
				}

				// Check if there is any open session
				if (!allowed) 
				{
					Node::ClientSession* session = Node::FindSession(address);
					if (session)
					{
						session->lastTime = Game::Com_Milliseconds();
						allowed = session->valid;
					}
				}

				if (allowed)
				{
					Node::SendNodeList(address);
				}
				else
				{
					// Unallowed connection
					Logger::Print("Node list requested by %s, but no valid session was present!\n", address.GetString());
					Network::Send(address, "nodeListError\n");
				}
			});

			Network::Handle("nodeDeregister", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || !entry->registered) return;

				Proto::NodePacket packet;
				if (!packet.ParseFromString(data)) return;
				if (!packet.has_challenge()) return;
				if (!packet.has_signature()) return;

				std::string challenge = packet.challenge();
				std::string signature = packet.signature();

				if (Utils::Cryptography::ECDSA::VerifyMessage(entry->publicKey, challenge, signature))
				{
					entry->lastHeard = Game::Com_Milliseconds();
					entry->lastTime = Game::Com_Milliseconds();
					entry->registered = false;
					entry->state = Node::STATE_INVALID;

					Logger::Print("Node %s unregistered\n", address.GetString());
				}
				else
				{
					Logger::Print("Node %s tried to unregister using an invalid signature!\n", address.GetString());
				}
			});

			Network::Handle("sessionRequest", [] (Network::Address address, std::string data)
			{
				// Return if we already have a session for this address
				if (Node::FindSession(address)) return;

				Logger::Print("Client %s is requesting a new session\n", address.GetString());

				Node::ClientSession session;
				session.address = address;
				session.challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
				session.lastTime = Game::Com_Milliseconds();
				session.valid = false;

				Node::Sessions.push_back(session);

				Network::Send(address, "sessionInitialize\n" + session.challenge);
			});

			Network::Handle("sessionSynchronize", [] (Network::Address address, std::string data)
			{
				// Return if we don't have a session for this address
				Node::ClientSession* session = Node::FindSession(address);
				if (!session || session->valid) return;

				if (session->challenge == data)
				{
					Logger::Print("Session for %s validated.\n", address.GetString());
					session->valid = true;
					Network::Send(address, "sessionAcknowledge\n");
				}
				else
				{
					session->lastTime = -1;
					Logger::Print("Challenge mismatch. Validating session for %s failed.\n", address.GetString());
				}
			});
		}
		else
		{
			Network::Handle("sessionInitialize", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry) return;

				Logger::Print("Session initialization received. Synchronizing...\n", address.GetString());

				entry->lastTime = Game::Com_Milliseconds();
				Network::Send(address, "sessionSynchronize\n" + data);
			});

			Network::Handle("sessionAcknowledge", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry) return;

				entry->state = Node::STATE_VALID;
				entry->registered = true;
				entry->lastTime = Game::Com_Milliseconds();

				Logger::Print("Session acknowledged, synchronizing node list...\n", address.GetString());
				Network::Send(address, "nodeListRequest\n");
				Node::SendNodeList(address);
			});
		}

		Network::Handle("nodeListResponse", [] (Network::Address address, std::string data)
		{
			if (data.size() % sizeof(Node::AddressEntry) != 1)
			{
				Logger::Print("Received invalid node list from %s!\n", address.GetString());
				return;
			}

			unsigned int size = ((data.size() - 1) / sizeof(Node::AddressEntry));
			Node::AddressEntry* addresses = reinterpret_cast<Node::AddressEntry*>(const_cast<char*>(data.data() + 1));
			bool isDedicated = (*data.data() != 0);

			Node::NodeEntry* entry = Node::FindNode(address);
			if (entry)
			{
				if (entry->registered)
				{
					Logger::Print("Received valid node list with %d entries from %s\n", size, address.GetString());

					entry->isDedi = isDedicated;
					entry->state = Node::STATE_VALID;
					entry->lastTime = Game::Com_Milliseconds();
					entry->lastListQuery = Game::Com_Milliseconds();

					for (unsigned int i = 0; i < size; ++i)
					{
						Node::AddNode(addresses[i].toNetAddress());
					}
				}
			}
			else
			{
				//Node::AddNode(address);

				Node::ClientSession* session = Node::FindSession(address);
				if (session && session->valid)
				{
					session->lastTime = Game::Com_Milliseconds();

					for (unsigned int i = 0; i < size; ++i)
					{
						Node::AddNode(addresses[i].toNetAddress());
					}
				}
			}
		});

		// If we receive that response, our request was not permitted
		// So we either have to register as node, or register a remote session
		Network::Handle("nodeListError", [] (Network::Address address, std::string data)
		{
			if (Dedicated::IsDedicated())
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (entry)
				{
					// Set to unregistered to perform registration later on
					entry->lastTime = Game::Com_Milliseconds();
					entry->registered = false;
					entry->state = Node::STATE_UNKNOWN;
				}
				else
				{
					// Add as new entry to perform registration
					Node::AddNode(address);
				}
			}
			else
			{
				// TODO: Implement client handshake stuff
				// Nvm, clients can simply ignore that i guess
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

		Command::Add("addnode", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			Network::Address address(params[1]);
			Node::AddNode(address);

			Node::NodeEntry* entry = Node::FindNode(address);
			if (entry)
			{
				entry->state = Node::STATE_UNKNOWN;
				entry->registered = false;
			}
		});

		// Install frame handlers
		Dedicated::OnFrame(Node::FrameHandler);
		Renderer::OnFrame(Node::FrameHandler);
	}

	Node::~Node()
	{
		Node::SignatureKey.Free();

		Node::StoreNodes(true);
		Node::Nodes.clear();
		Node::Sessions.clear();
	}

	bool Node::UnitTest()
	{
		printf("Testing ECDSA key...");

		if (!Node::SignatureKey.IsValid())
		{
			printf("Error\n");
			printf("ECDSA key seems invalid!\n");
			return false;
		}
		
		printf("Success\n");
		printf("Testing 10 valid signatures...");

		for (int i = 0; i < 10; ++i)
		{
			std::string message = Utils::VA("%d", Utils::Cryptography::Rand::GenerateInt());
			std::string signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, message);

			if (!Utils::Cryptography::ECDSA::VerifyMessage(Node::SignatureKey, message, signature))
			{
				printf("Error\n");
				printf("Signature for '%s' (%d) was invalid!\n", message.data(), i);
				return false;
			}
		}

		printf("Success\n");
		printf("Testing 10 invalid signatures...");

		for (int i = 0; i < 10; ++i)
		{
			std::string message = Utils::VA("%d", Utils::Cryptography::Rand::GenerateInt());
			std::string signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, message);

			// Invalidate the message...
			message[Utils::Cryptography::Rand::GenerateInt() % message.size()]++;

			if (Utils::Cryptography::ECDSA::VerifyMessage(Node::SignatureKey, message, signature))
			{
				printf("Error\n");
				printf("Signature for '%s' (%d) was valid? What the fuck? That is absolutely impossible...\n", message.data(), i);
				return false;
			}
		}

		printf("Success\n");
		printf("Testing ECDSA key import...");

		std::string pubKey = Node::SignatureKey.GetPublicKey();
		std::string message = Utils::VA("%d", Utils::Cryptography::Rand::GenerateInt());
		std::string signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, message);

		Utils::Cryptography::ECDSA::Key testKey;
		testKey.Set(pubKey);

		if (!Utils::Cryptography::ECDSA::VerifyMessage(Node::SignatureKey, message, signature))
		{
			printf("Error\n");
			printf("Verifying signature for message '%s' using imported keys failed!\n", message.data());
			return false;
		}

		printf("Success\n");
		return true;
	}
}
