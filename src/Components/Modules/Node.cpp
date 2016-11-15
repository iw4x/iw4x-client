#include "STDInclude.hpp"

namespace Components
{
	std::mutex Node::NodeMutex;
	std::mutex Node::SessionMutex;
	Utils::Cryptography::ECC::Key Node::SignatureKey;
	std::vector<Node::NodeEntry> Node::Nodes;
	std::vector<Node::ClientSession> Node::Sessions;

	void Node::LoadNodeRemotePreset()
	{
		std::string nodes = Utils::WebIO("IW4x", "https://iw4xcachep26muba.onion.to/iw4/nodex.txt").SetTimeout(5000)->Get();
		if (nodes.empty()) return;

		auto nodeList = Utils::String::Explode(nodes, '\n');
		for (auto node : nodeList)
		{
			Utils::String::Replace(node, "\r", "");
			node = Utils::String::Trim(node);
			Node::AddNode(node);
		}
	}

	void Node::LoadNodePreset()
	{
		Proto::Node::List list;
		FileSystem::File defaultNodes("nodes_default.dat");
		if (!defaultNodes.Exists() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(defaultNodes.GetBuffer()))) return;

		for (int i = 0; i < list.address_size(); ++i)
		{
			Node::AddNode(list.address(i));
		}
	}

	void Node::LoadNodes()
	{
		Proto::Node::List list;
		std::string nodes = Utils::IO::ReadFile("players/nodes.dat");
		if (nodes.empty() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(nodes))) return;

		for (int i = 0; i < list.address_size(); ++i)
		{
			Node::AddNode(list.address(i));
		}
	}
	void Node::StoreNodes(bool force)
	{
		if (Dedicated::IsEnabled() && Dvar::Var("sv_lanOnly").Get<bool>()) return;

		static int lastStorage = 0;

		// Don't store nodes if the delta is too small and were not forcing it
		if (((Game::Sys_Milliseconds() - lastStorage) < NODE_STORE_INTERVAL && !force) || !Node::GetValidNodeCount()) return;
		lastStorage = Game::Sys_Milliseconds();

		Proto::Node::List list;

		// This is obsolete when storing to file.
		// However, defining another proto message due to this would be redundant.
		//list.set_is_dedi(Dedicated::IsDedicated());

		std::lock_guard<std::mutex> _(Node::NodeMutex);
		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID && node.registered)
			{
				node.address.Serialize(list.add_address());
			}
		}

		CreateDirectoryW(L"players", NULL);

		

		Utils::IO::WriteFile("players/nodes.dat", Utils::Compression::ZLib::Compress(list.SerializeAsString()));
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

	unsigned int Node::GetValidNodeCount()
	{
		unsigned int count = 0;
		std::lock_guard<std::mutex> _(Node::NodeMutex);

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_VALID)
			{
				++count;
			}
		}

		return count;
	}

	void Node::AddNode(Network::Address address)
	{
#ifdef DEBUG
		if (!address.IsValid() || address.IsSelf()) return;
#else
		if (!address.IsValid() || address.IsLocal() || address.IsSelf()) return;
#endif

		std::lock_guard<std::mutex> _(Node::NodeMutex);
		Node::NodeEntry* existingEntry = Node::FindNode(address);
		if (existingEntry)
		{
			existingEntry->lastHeard = Game::Sys_Milliseconds();
		}
		else
		{
			Node::NodeEntry entry;

			entry.lastHeard = Game::Sys_Milliseconds();
			entry.lastTime = 0;
			entry.lastListQuery = 0;
			entry.registered = false;
			entry.state = Node::STATE_UNKNOWN;
			entry.address = address;
			entry.challenge.clear();

			Node::Nodes.push_back(entry);

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
			Logger::Print("Adding node %s...\n", address.GetCString());
#endif
		}
	}

	void Node::SendNodeList(Network::Address address)
	{
		if (address.IsSelf()) return;

		Proto::Node::List list;
		list.set_is_dedi(Dedicated::IsEnabled());
		list.set_protocol(PROTOCOL);
		list.set_version(NODE_VERSION);

		std::lock_guard<std::mutex> _(Node::NodeMutex);

		for (auto& node : Node::Nodes)
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
		std::lock_guard<std::mutex> _(Node::SessionMutex);
		for (auto i = Node::Sessions.begin(); i != Node::Sessions.end();)
		{
			if (i->lastTime <= 0 || (Game::Sys_Milliseconds() - i->lastTime) > SESSION_TIMEOUT)
			{
				i = Node::Sessions.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void Node::DeleteInvalidNodes()
	{
		std::lock_guard<std::mutex> _(Node::NodeMutex);
		std::vector<Node::NodeEntry> cleanNodes;

		for (auto node : Node::Nodes)
		{
			if (node.state == Node::STATE_INVALID && (Game::Sys_Milliseconds() - node.lastHeard) > NODE_INVALID_DELETE)
			{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Removing invalid node %s\n", node.address.GetCString());
#endif
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
		std::lock_guard<std::mutex> _(Node::NodeMutex);
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

		entry->lastTime = Game::Sys_Milliseconds();

		if (Dedicated::IsEnabled())
		{
			entry->challenge = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());

			Proto::Node::Packet packet;
			packet.set_challenge(entry->challenge);

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
			Logger::Print("Sending registration request to %s\n", entry->address.GetCString());
#endif
			Network::SendCommand(entry->address, "nodeRegisterRequest", packet.SerializeAsString());
		}
		else
		{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
			Logger::Print("Sending session request to %s\n", entry->address.GetCString());
#endif
			Network::SendCommand(entry->address, "sessionRequest");
		}
	}

	void Node::FrameHandler()
	{
		if (Dedicated::IsEnabled() && Dvar::Var("sv_lanOnly").Get<bool>()) return;

		// Frame limit
		static int lastFrame = 0;
		if ((Game::Sys_Milliseconds() - lastFrame) < (1000 / NODE_FRAME_LOCK) || Game::Sys_Milliseconds() < 5000) return;
		lastFrame = Game::Sys_Milliseconds();

		int registerCount = 0;
		int listQueryCount = 0;

		{
			std::lock_guard<std::mutex> _(Node::NodeMutex);
			for (auto &node : Node::Nodes)
			{
				// TODO: Decide how to handle nodes that were already registered, but timed out re-registering.
				if (node.state == STATE_NEGOTIATING && (Game::Sys_Milliseconds() - node.lastTime) > (NODE_QUERY_TIMEOUT))
				{
					node.registered = false; // Definitely unregister here!
					node.state = Node::STATE_INVALID;
					node.lastHeard = Game::Sys_Milliseconds();
					node.lastTime = Game::Sys_Milliseconds();

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Node negotiation timed out. Invalidating %s\n", node.address.GetCString());
#endif
				}

				if (registerCount < NODE_FRAME_QUERY_LIMIT)
				{
					// Register when unregistered and in UNKNOWN state (I doubt it's possible to be unregistered and in VALID state)
					if (!node.registered && (node.state != Node::STATE_NEGOTIATING && node.state != Node::STATE_INVALID))
					{
						++registerCount;
						node.state = Node::STATE_NEGOTIATING;
						Node::PerformRegistration(node.address);
					}
					// Requery invalid nodes within the NODE_QUERY_INTERVAL
					// This is required, as a node might crash, which causes it to be invalid.
					// If it's restarted though, we wouldn't query it again.

					// But wouldn't it send a registration request to us?
					// Not sure if the code below is necessary...
					// Well, it might be possible that this node doesn't know use anymore. Anyways, just keep that code here...

					// Nvm, this is required for clients, as nodes don't send registration requests to clients.
					else if (node.state == STATE_INVALID && (Game::Sys_Milliseconds() - node.lastTime) > NODE_QUERY_INTERVAL)
					{
						++registerCount;
						Node::PerformRegistration(node.address);
					}
				}

				if (listQueryCount < NODE_FRAME_QUERY_LIMIT)
				{
					if (node.registered && node.state == Node::STATE_VALID && (!node.lastListQuery || (Game::Sys_Milliseconds() - node.lastListQuery) > NODE_QUERY_INTERVAL))
					{
						++listQueryCount;
						node.state = Node::STATE_NEGOTIATING;
						node.lastTime = Game::Sys_Milliseconds();
						node.lastListQuery = Game::Sys_Milliseconds();

						if (Dedicated::IsEnabled())
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
		}

		static int lastCheck = 0;
		if ((Game::Sys_Milliseconds() - lastCheck) < 1000) return;
		lastCheck = Game::Sys_Milliseconds();

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
		Node::SignatureKey = Utils::Cryptography::ECC::GenerateKey(512);

		// Load stored nodes
		Dvar::OnInit([] ()
		{
			Node::LoadNodePreset();
			Node::LoadNodes();
		});

		// Send deadline when shutting down
		if (Dedicated::IsEnabled())
		{
			QuickPatch::OnShutdown([] ()
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				std::string challenge = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());

				Proto::Node::Packet packet;
				packet.set_challenge(challenge);
				packet.set_signature(Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, challenge));

				std::lock_guard<std::mutex> _(Node::NodeMutex);
				for (auto node : Node::Nodes)
				{
					Network::SendCommand(node.address, "nodeDeregister", packet.SerializeAsString());
				}
			});

			// This is the handler that accepts registration requests from other nodes
			// If you want to get accepted as node, you have to send a request to this handler
			Network::Handle("nodeRegisterRequest", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				// Create a new entry, if we don't already know it
				if (!Node::FindNode(address))
				{
					Node::AddNode(address);
					if (!Node::FindNode(address)) return;
				}

				std::lock_guard<std::mutex> _(Node::NodeMutex);
				Node::NodeEntry* entry = Node::FindNode(address);

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Received registration request from %s\n", address.GetCString());
#endif

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.challenge().empty()) return;

				std::string signature = Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, packet.challenge());
				std::string challenge = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());

				// The challenge this client sent is exactly the challenge we stored for this client
				// That means this is us, so we're going to ignore us :P
				if (packet.challenge() == entry->challenge)
				{
					entry->lastHeard = Game::Sys_Milliseconds();
					entry->lastTime = Game::Sys_Milliseconds();
					entry->registered = false;
					entry->state = Node::STATE_INVALID;
					return;
				}

				packet.Clear();
				packet.set_challenge(challenge);
				packet.set_signature(signature);
				packet.set_publickey(Node::SignatureKey.GetPublicKey());

				entry->lastTime = Game::Sys_Milliseconds();
				entry->challenge = challenge;
				entry->state = Node::STATE_NEGOTIATING;

				Network::SendCommand(address, "nodeRegisterSynchronize", packet.SerializeAsString());
			});

			Network::Handle("nodeRegisterSynchronize", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				std::lock_guard<std::mutex> _(Node::NodeMutex);
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Received synchronization data for registration from %s!\n", address.GetCString());
#endif

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
				if (!Utils::Cryptography::ECC::VerifyMessage(entry->publicKey, entry->challenge, signature))
				{
					Logger::Print("Signature from %s for challenge '%s' is invalid!\n", address.GetCString(), entry->challenge.data());
					return;
				}

				// Mark as registered
				entry->lastTime = Game::Sys_Milliseconds();
				entry->state = Node::STATE_VALID;
				entry->registered = true;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Signature from %s for challenge '%s' is valid!\n", address.GetCString(), entry->challenge.data());
				Logger::Print("Node %s registered\n", address.GetCString());
#endif

				// Build response
				publicKey = Node::SignatureKey.GetPublicKey();
				signature = Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, challenge);

				packet.Clear();
				packet.set_signature(signature);
				packet.set_publickey(publicKey);

				Network::SendCommand(address, "nodeRegisterAcknowledge", packet.SerializeAsString());
			});

			Network::Handle("nodeRegisterAcknowledge", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				// Ignore requests from nodes we don't know
				std::lock_guard<std::mutex> _(Node::NodeMutex);
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || entry->state != Node::STATE_NEGOTIATING) return;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Received acknowledgment from %s\n", address.GetCString());
#endif

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.signature().empty()) return;
				if (packet.publickey().empty()) return;

				std::string publicKey = packet.publickey();
				std::string signature = packet.signature();

				entry->publicKey.Set(publicKey);

				if (Utils::Cryptography::ECC::VerifyMessage(entry->publicKey, entry->challenge, signature))
				{
					entry->lastTime = Game::Sys_Milliseconds();
					entry->state = Node::STATE_VALID;
					entry->registered = true;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Signature from %s for challenge '%s' is valid!\n", address.GetCString(), entry->challenge.data());
					Logger::Print("Node %s registered\n", address.GetCString());
#endif
				}
				else
				{
#ifdef DEBUG
					Logger::Print("Signature from %s for challenge '%s' is invalid!\n", address.GetCString(), entry->challenge.data());
#endif
				}
			});

			Network::Handle("nodeListRequest", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				// Check if this is a registered node
				bool allowed = false;

				{
					std::lock_guard<std::mutex> _(Node::NodeMutex);
					Node::NodeEntry* entry = Node::FindNode(address);
					if (entry && entry->registered)
					{
						entry->lastTime = Game::Sys_Milliseconds();
						allowed = true;
					}

					// Check if there is any open session
					if (!allowed)
					{
						std::lock_guard<std::mutex> __(Node::SessionMutex);
						Node::ClientSession* session = Node::FindSession(address);
						if (session)
						{
							session->lastTime = Game::Sys_Milliseconds();
							allowed = session->valid;
						}
					}
				}

				if (allowed)
				{
					Node::SendNodeList(address);
				}
				else
				{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					// Unallowed connection
					Logger::Print("Node list requested by %s, but no valid session was present!\n", address.GetCString());
#endif
					Network::SendCommand(address, "nodeListError");
				}
			});

			Network::Handle("nodeDeregister", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				std::lock_guard<std::mutex> _(Node::NodeMutex);
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry || !entry->registered) return;

				Proto::Node::Packet packet;
				if (!packet.ParseFromString(data)) return;
				if (packet.challenge().empty()) return;
				if (packet.signature().empty()) return;

				std::string challenge = packet.challenge();
				std::string signature = packet.signature();

				if (Utils::Cryptography::ECC::VerifyMessage(entry->publicKey, challenge, signature))
				{
					entry->lastHeard = Game::Sys_Milliseconds();
					entry->lastTime = Game::Sys_Milliseconds();
					entry->registered = false;
					entry->state = Node::STATE_INVALID;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Node %s unregistered\n", address.GetCString());
#endif
				}
				else
				{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Node %s tried to unregister using an invalid signature!\n", address.GetCString());
#endif
				}
			});

			Network::Handle("sessionRequest", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				// Search an active session, if we haven't found one, register a template
				std::lock_guard<std::mutex> _(Node::SessionMutex);
				if (!Node::FindSession(address))
				{
					Node::ClientSession templateSession;
					templateSession.address = address;
					Node::Sessions.push_back(templateSession);
				}

				// Search our target session (this should not fail!)
				Node::ClientSession* session = Node::FindSession(address);
				if (!session) return; // Registering template session failed, odd...

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Client %s is requesting a new session\n", address.GetCString());
#endif

				// Initialize session data
				session->challenge = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());
				session->lastTime = Game::Sys_Milliseconds();
				session->valid = false;

				Network::SendCommand(address, "sessionInitialize", session->challenge);
			});

			Network::Handle("sessionSynchronize", [] (Network::Address address, std::string data)
			{
				if (Dvar::Var("sv_lanOnly").Get<bool>()) return;

				// Return if we don't have a session for this address
				std::lock_guard<std::mutex> _(Node::SessionMutex);
				Node::ClientSession* session = Node::FindSession(address);
				if (!session || session->valid) return;

				if (session->challenge == data)
				{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Session for %s validated.\n", address.GetCString());
#endif
					session->valid = true;
					Network::SendCommand(address, "sessionAcknowledge");
				}
				else
				{
					session->lastTime = -1;
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Challenge mismatch. Validating session for %s failed.\n", address.GetCString());
#endif
				}
			});
		}
		else
		{
			Network::Handle("sessionInitialize", [] (Network::Address address, std::string data)
			{
				std::lock_guard<std::mutex> _(Node::NodeMutex);
				Node::NodeEntry* entry = Node::FindNode(address);
				if (!entry) return;

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Session initialization received from %s. Synchronizing...\n", address.GetCString());
#endif

				entry->lastTime = Game::Sys_Milliseconds();
				Network::SendCommand(address, "sessionSynchronize", data);
			});

			Network::Handle("sessionAcknowledge", [] (Network::Address address, std::string data)
			{
				{
					std::lock_guard<std::mutex> _(Node::NodeMutex);
					Node::NodeEntry* entry = Node::FindNode(address);
					if (!entry) return;

					entry->state = Node::STATE_VALID;
					entry->registered = true;
					entry->lastTime = Game::Sys_Milliseconds();
				}

#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Session acknowledged by %s, synchronizing node list...\n", address.GetCString());
#endif
				Network::SendCommand(address, "nodeListRequest");
				Node::SendNodeList(address);
			});
		}

		Network::Handle("nodeListResponse", [] (Network::Address address, std::string data)
		{
			Proto::Node::List list;

			if (data.empty() || !list.ParseFromString(data)) 
			{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
				Logger::Print("Received invalid node list from %s!\n", address.GetCString());
#endif
				return;
			}

			Node::NodeMutex.lock();
			Node::NodeEntry* entry = Node::FindNode(address);
			if (entry)
			{
				if (entry->registered)
				{
#if defined(DEBUG) && !defined(DISABLE_NODE_LOG)
					Logger::Print("Received valid node list with %i entries from %s\n", list.address_size(), address.GetCString());
#endif

					entry->isDedi = list.is_dedi();
					entry->protocol = list.protocol();
					entry->version = list.version();
					entry->state = Node::STATE_VALID;
					entry->lastTime = Game::Sys_Milliseconds();

#ifndef DEBUG
					// Block old versions
					if (entry->version < NODE_VERSION)
					{
						entry->state = Node::STATE_INVALID;
						Node::NodeMutex.unlock();
						return;
					}
#endif

					if (!Dedicated::IsEnabled() && entry->isDedi && ServerList::IsOnlineList() && entry->protocol == PROTOCOL)
					{
						ServerList::InsertRequest(entry->address, true);
					}

					Node::NodeMutex.unlock();

					for (int i = 0; i < list.address_size(); ++i)
					{
						Network::Address _addr(list.address(i));

						// Version 0 sends port in the wrong byte order!
						if (list.version() == 0)
						{
							_addr.SetPort(ntohs(_addr.GetPort()));
						}

// 						if (!Node::FindNode(_addr) && _addr.GetPort() >= 1024 && _addr.GetPort() - 20 < 1024)
// 						{
// 							std::string a1 = _addr.GetString();
// 							std::string a2 = address.GetString();
// 							Logger::Print("Received weird address %s from %s\n", a1.data(), a2.data());
// 						}

						Node::AddNode(_addr);
					}
				}
				else
				{
					Node::NodeMutex.unlock();
				}
			}
			else
			{
				Node::NodeMutex.unlock();
				//Node::AddNode(address);

				std::lock_guard<std::mutex> _(Node::SessionMutex);
				Node::ClientSession* session = Node::FindSession(address);
				if (session && session->valid)
				{
					session->lastTime = Game::Sys_Milliseconds();

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
			if (Dedicated::IsEnabled())
			{
				Node::NodeMutex.lock();
				Node::NodeEntry* entry = Node::FindNode(address);
				if (entry)
				{
					// Set to unregistered to perform registration later on
					entry->lastTime = Game::Sys_Milliseconds();
					entry->registered = false;
					entry->state = Node::STATE_UNKNOWN;
					Node::NodeMutex.unlock();
				}
				else
				{
					Node::NodeMutex.unlock();

					// Add as new entry to perform registration
					Node::AddNode(address);
				}
			}
		});

		Command::Add("listnodes", [] (Command::Params)
		{
			Logger::Print("Nodes: %d (%d)\n", Node::Nodes.size(), Node::GetValidNodeCount());

			std::lock_guard<std::mutex> _(Node::NodeMutex);
			for (auto node : Node::Nodes)
			{
				Logger::Print("%s\t(%s)\n", node.address.GetCString(), Node::GetStateName(node.state));
			}
		});

		Command::Add("addnode", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			Network::Address address(params[1]);
			Node::AddNode(address);

			std::lock_guard<std::mutex> _(Node::NodeMutex);
			Node::NodeEntry* entry = Node::FindNode(address);
			if (entry)
			{
				entry->state = Node::STATE_UNKNOWN;
				entry->registered = false;
			}
		});

		Command::Add("syncnodes", [] (Command::Params)
		{
			Logger::Print("Re-Synchronizing nodes...\n");
			
			std::lock_guard<std::mutex> _(Node::NodeMutex);
			for (auto& node : Node::Nodes)
			{
				node.state = Node::STATE_UNKNOWN;
				node.registered = false;
				node.lastTime = 0;
				node.lastListQuery = 0;
			}
		});

		// Install frame handlers
		QuickPatch::OnFrame(Node::FrameHandler);
	}

	Node::~Node()
	{
		Node::SignatureKey.Free();

		Node::StoreNodes(true);
		std::lock_guard<std::mutex> _(Node::NodeMutex);
		std::lock_guard<std::mutex> __(Node::SessionMutex);
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
			std::string message = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());
			std::string signature = Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, message);

			if (!Utils::Cryptography::ECC::VerifyMessage(Node::SignatureKey, message, signature))
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
			std::string message = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());
			std::string signature = Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, message);

			// Invalidate the message...
			++message[Utils::Cryptography::Rand::GenerateInt() % message.size()];

			if (Utils::Cryptography::ECC::VerifyMessage(Node::SignatureKey, message, signature))
			{
				printf("Error\n");
				printf("Signature for '%s' (%d) was valid? What the fuck? That is absolutely impossible...\n", message.data(), i);
				return false;
			}
		}

		printf("Success\n");
		printf("Testing ECDSA key import...");

		std::string pubKey = Node::SignatureKey.GetPublicKey();
		std::string message = fmt::sprintf("%X", Utils::Cryptography::Rand::GenerateInt());
		std::string signature = Utils::Cryptography::ECC::SignMessage(Node::SignatureKey, message);

		Utils::Cryptography::ECC::Key testKey;
		testKey.Set(pubKey);

		if (!Utils::Cryptography::ECC::VerifyMessage(Node::SignatureKey, message, signature))
		{
			printf("Error\n");
			printf("Verifying signature for message '%s' using imported keys failed!\n", message.data());
			return false;
		}

		printf("Success\n");
		return true;
	}
}
