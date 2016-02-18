#include "STDInclude.hpp"

using namespace std::literals;

namespace Components
{
	Utils::Cryptography::ECDSA::Key Node::SignatureKey;
	std::vector<Node::NodeEntry> Node::Nodes;
	std::vector<Node::ClientSession> Node::Sessions;

	void Node::LoadNodePreset()
	{
		FileSystem::File defaultNodes("default_nodes.dat");
		if (!defaultNodes.Exists()) return;

		auto buffer = defaultNodes.GetBuffer();
		Utils::Replace(buffer, "\r", "");

		auto nodes = Utils::Explode(buffer, '\n');
		for (auto node : nodes)
		{
			if (!node.empty())
			{
				Node::AddNode(node);
			}
		}
	}

	void Node::LoadNodes()
	{
		Proto::Node::List list;
		std::string nodes = Utils::ReadFile("players/nodes.dat");
		if (nodes.empty() || !list.ParseFromString(nodes)) return;

		for (int i = 0; i < list.address_size(); ++i)
		{
			Node::AddNode(list.address(i));
		}
	}
	void Node::StoreNodes(bool force)
	{
		static int lastStorage = 0;

		// Don't store nodes if the delta is too small and were not forcing it
		if ((Game::Com_Milliseconds() - lastStorage) < NODE_STORE_INTERVAL && !force) return;
		lastStorage = Game::Com_Milliseconds();

		Proto::Node::List list;

		// This is obsolete when storing to file.
		// However, defining another proto message due to this would be redundant.
		//list.set_is_dedi(Dedicated::IsDedicated());

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered)
			{
				node.address.Serialize(list.add_address());
			}
		}

		CreateDirectoryW(L"players", NULL);
		Utils::WriteFile("players/nodes.dat", list.SerializeAsString());
	}

	Node::NodeEntry* Node::FindNode(Network::Address address)
	{
		for (auto i = Node::Nodes.begin(); i != Node::Nodes.end(); ++i)
		{
			if (i->address == address)
			{
				return &(*i);
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
				return &(*i);
			}
		}

		return nullptr;
	}

	void Node::AddNode(Network::Address address)
	{
#ifdef DEBUG
		if (!address.IsValid() || address.IsSelf()) return;
#else
		if (!address.IsValid() || address.IsLocal() || address.IsSelf()) return;
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

			Logger::Print("Adding node %s...\n", address.GetString());
		}
	}

	void Node::SendNodeList(Network::Address address)
	{
		if (address.IsSelf()) return;

		Proto::Node::List list;
		list.set_is_dedi(Dedicated::IsDedicated());

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered)
			{
				node.address.Serialize(list.add_address());
			}

			if (list.address_size() >= NODE_PACKET_LIMIT)
			{
				Network::SendCommand(address, "nodeListResponse", list.SerializeAsString());
				list.clear_address();
			}
		}

		// Even if we send an empty list, we have to tell the client about our dedi-status
		// If the amount of servers we have modulo the NODE_PACKET_LIMIT equals 0, we will send this request without any servers, so it's obsolete, but meh...
		Network::SendCommand(address, "nodeListResponse", list.SerializeAsString());
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

	void Node::SyncNodeList()
	{
		for (auto& node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered)
			{
				node.state = Node::STATE_UNKNOWN;
				node.registered = false;
			}
		}
	}

	void Node::PerformRegistration(Network::Address address)
	{
		Node::NodeEntry* entry = Node::FindNode(address);
		if (!entry) return;

		entry->lastTime = Game::Com_Milliseconds();

		if (Dedicated::IsDedicated())
		{
			entry->challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

			Proto::Node::Packet packet;
			packet.set_challenge(entry->challenge);

			Logger::Print("Sending registration request to %s\n", entry->address.GetString());
			Network::SendCommand(entry->address, "nodeRegisterRequest", packet.SerializeAsString());
		}
		else
		{
			Logger::Print("Sending session request to %s\n", entry->address.GetString());
			Network::SendCommand(entry->address, "sessionRequest");
		}
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
					Node::PerformRegistration(node.address);
				}
				// Requery invalid nodes within the NODE_QUERY_INTERVAL
				// This is required, as a node might crash, which causes it to be invalid
				// If it's restarted though, we wouldn't query it again
				else if (node.state == STATE_INVALID && (Game::Com_Milliseconds() - node.lastTime) >(NODE_QUERY_INTERVAL)) 
				{
					registerCount++;
					Node::PerformRegistration(node.address);
				}
			}

			if (listQueryCount < NODE_FRAME_QUERY_LIMIT)
			{
				if (node.registered && node.state == Node::STATE_VALID && (!node.lastListQuery || (Game::Com_Milliseconds() - node.lastListQuery) > NODE_QUERY_INTERVAL))
				{
					listQueryCount++;
					node.state = Node::STATE_NEGOTIATING;
					node.lastTime = Game::Com_Milliseconds();
					node.lastListQuery = Game::Com_Milliseconds();

					if (Dedicated::IsDedicated())
					{
						Network::SendCommand(node.address, "nodeListRequest");
					}
					else
					{
						Network::SendCommand(node.address, "sessionRequest");
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
		Node::Nodes.clear();

		// ZoneBuilder doesn't require node stuff
		if (ZoneBuilder::IsEnabled()) return;

		// Generate our ECDSA key
		Node::SignatureKey = Utils::Cryptography::ECDSA::GenerateKey(512);

		// Load stored nodes
		Dvar::OnInit([] ()
		{
			Node::LoadNodePreset();
			Node::LoadNodes();
		});

		// Send deadline when shutting down
		if (Dedicated::IsDedicated())
		{
			QuickPatch::OnShutdown([] ()
			{
				std::string challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

				Proto::Node::Packet packet;
				packet.set_challenge(challenge);
				packet.set_signature(Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, challenge));

				for (auto node : Node::Nodes)
				{
					Network::SendCommand(node.address, "nodeDeregister", packet.SerializeAsString());
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

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.challenge().empty()) return;

				std::string signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, packet.challenge());
				std::string challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());

				packet.Clear();
				packet.set_challenge(challenge);
				packet.set_signature(signature);
				packet.set_publickey(Node::SignatureKey.GetPublicKey());

				entry->lastTime = Game::Com_Milliseconds();
				entry->challenge = challenge;
				entry->state = Node::STATE_NEGOTIATING;

				Network::SendCommand(address, "nodeRegisterSynchronize", packet.SerializeAsString());
			});

			Network::Handle("nodeRegisterSynchronize", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

				Logger::Print("Received synchronization data for registration from %s!\n", address.GetString());

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.challenge().empty()) return;
				if (packet.publickey().empty()) return;
				if (packet.signature().empty()) return;

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
				publicKey = Node::SignatureKey.GetPublicKey();
				signature = Utils::Cryptography::ECDSA::SignMessage(Node::SignatureKey, challenge);

				packet.Clear();
				packet.set_signature(signature);
				packet.set_publickey(publicKey);

				Network::SendCommand(address, "nodeRegisterAcknowledge", packet.SerializeAsString());
			});

			Network::Handle("nodeRegisterAcknowledge", [] (Network::Address address, std::string data)
			{
				// Ignore requests from nodes we don't know
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

				Logger::Print("Received acknowledgment from %s\n", address.GetString());

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.signature().empty()) return;
				if (packet.publickey().empty()) return;

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
				// Check if this is a registered node
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
					Network::SendCommand(address, "nodeListError");
				}
			});

			Network::Handle("nodeDeregister", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || !entry->registered) return;

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.challenge().empty()) return;
				if (packet.signature().empty()) return;

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
				// Search an active session, if we haven't found one, register a template
				if (!Node::FindSession(address))
				{
					Node::ClientSession templateSession;
					templateSession.address = address;
					Node::Sessions.push_back(templateSession);
				}

				// Search our target session (this should not fail!)
				Node::ClientSession* session = Node::FindSession(address);
				if (!session) return; // Registering template session failed, odd...

				Logger::Print("Client %s is requesting a new session\n", address.GetString());

				// Initialize session data
				session->challenge = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
				session->lastTime = Game::Com_Milliseconds();
				session->valid = false;

				Network::SendCommand(address, "sessionInitialize", session->challenge);
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
					Network::SendCommand(address, "sessionAcknowledge");
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
				Network::SendCommand(address, "sessionSynchronize", data);
			});

			Network::Handle("sessionAcknowledge", [] (Network::Address address, std::string data)
			{
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry) return;

				entry->state = Node::STATE_VALID;
				entry->registered = true;
				entry->lastTime = Game::Com_Milliseconds();

				Logger::Print("Session acknowledged, synchronizing node list...\n", address.GetString());
				Network::SendCommand(address, "nodeListRequest");
				Node::SendNodeList(address);
			});
		}

		Network::Handle("nodeListResponse", [] (Network::Address address, std::string data)
		{
			Proto::Node::List list;

			if (data.empty() || !list.ParseFromString(data)) 
			{
				Logger::Print("Received invalid node list from %s!\n", address.GetString());
				return;
			}

			Node::NodeEntry* entry = Node::FindNode(address);
			if (entry)
			{
				if (entry->registered)
				{
					Logger::Print("Received valid node list with %i entries from %s\n", list.address_size(), address.GetString());

					entry->isDedi = list.is_dedi();
					entry->state = Node::STATE_VALID;
					entry->lastTime = Game::Com_Milliseconds();

					if (!Dedicated::IsDedicated() && entry->isDedi && ServerList::IsOnlineList())
					{
						ServerList::InsertRequest(entry->address, true);
					}

					for (int i = 0; i < list.address_size(); ++i)
					{
						Node::AddNode(list.address(i));
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

					for (int i = 0; i < list.address_size(); ++i)
					{
						Node::AddNode(list.address(i));
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
			std::string message = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
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
			std::string message = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
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
		std::string message = Utils::VA("%X", Utils::Cryptography::Rand::GenerateInt());
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

		uint32_t randIntCount = 4'000'000;
		printf("Generating %d random integers...", randIntCount);

		auto startTime = std::chrono::high_resolution_clock::now();

		for (uint32_t i = 0; i < randIntCount; ++i)
		{
			Utils::Cryptography::Rand::GenerateInt();
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
		Logger::Print("took %llims\n", duration);

		return true;
	}
}
