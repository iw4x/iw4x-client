#include <STDInclude.hpp>

namespace Components
{
	std::recursive_mutex Node::Mutex;
	std::vector<Node::Entry> Node::Nodes;

	bool Node::wasIngame = false;

	bool Node::Entry::isValid()
	{
		return (this->lastResponse.has_value() && !this->lastResponse->elapsed(NODE_HALFLIFE * 2));
	}

	bool Node::Entry::isDead()
	{
		if (!this->lastResponse.has_value())
		{
			if (this->lastRequest.has_value() && this->lastRequest->elapsed(NODE_HALFLIFE))
			{
				return true;
			}
		}
		else if (this->lastResponse->elapsed(NODE_HALFLIFE * 2) && this->lastRequest.has_value() && this->lastRequest->after(*this->lastResponse))
		{
			return true;
		}

		return false;
	}

	bool Node::Entry::requiresRequest()
	{
		return (!this->isDead() && (!this->lastRequest.has_value() || this->lastRequest->elapsed(NODE_HALFLIFE)));
	}

	void Node::Entry::sendRequest()
	{
		if (!this->lastRequest.has_value()) this->lastRequest.emplace(Utils::Time::Point());
		this->lastRequest->update();

		Session::Send(this->address, "nodeListRequest");
		Node::SendList(this->address);
		Logger::Debug("Sent request to {}", this->address.getCString());
	}

	void Node::Entry::reset()
	{
		// this->lastResponse.reset(); // This would invalidate the node, but maybe we don't want that?
		this->lastRequest.reset();
	}

	nlohmann::json Node::Entry::to_json() const
	{
		return this->address.getString();
	}

	void Node::LoadNodeRemotePreset()
	{
		std::string nodes = Utils::Cache::GetFile("/iw4/nodes.txt");
		if (nodes.empty()) return;

		auto nodeList = Utils::String::Split(nodes, '\n');
		for (auto& node : nodeList)
		{
			Utils::String::Replace(node, "\r", "");
			node = Utils::String::Trim(node);
			Node::Add(node);
		}
	}

	void Node::LoadNodePreset()
	{
		Proto::Node::List list;

		if (Monitor::IsEnabled())
		{
			std::string nodes = Utils::IO::ReadFile("players/nodes_default.dat");
			if (nodes.empty() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(nodes))) return;
		}
		else
		{
			FileSystem::File defaultNodes("nodes_default.dat");
			if (!defaultNodes.exists() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(defaultNodes.getBuffer()))) return;
		}

		for (int i = 0; i < list.nodes_size(); ++i)
		{
			const std::string& addr = list.nodes(i);

			if (addr.size() == sizeof(sockaddr))
			{
				Node::Add(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
			}
		}
	}

	void Node::LoadNodes()
	{
		Proto::Node::List list;
		std::string nodes = Utils::IO::ReadFile("players/nodes.dat");
		if (nodes.empty() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(nodes))) return;

		for (int i = 0; i < list.nodes_size(); ++i)
		{
			const std::string& addr = list.nodes(i);

			if (addr.size() == sizeof(sockaddr))
			{
				Node::Add(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
			}
		}
	}

	void Node::StoreNodes(bool force)
	{
		if (Dedicated::IsEnabled() && Dedicated::SVLanOnly.get<bool>()) return;

		static Utils::Time::Interval interval;
		if (!force && !interval.elapsed(1min)) return;
		interval.update();

		Proto::Node::List list;

		Node::Mutex.lock();
		for (auto& node : Node::Nodes)
		{
			if (node.isValid())
			{
				std::string* str = list.add_nodes();

				sockaddr addr = node.address.getSockAddr();
				str->append(reinterpret_cast<char*>(&addr), sizeof(addr));
			}
		}
		Node::Mutex.unlock();

		Utils::IO::WriteFile("players/nodes.dat", Utils::Compression::ZLib::Compress(list.SerializeAsString()));
	}

	void Node::Add(Network::Address address)
	{
#ifndef DEBUG
		if (address.isLocal() || address.isSelf()) return;
#endif

		if (!address.isValid()) return;

		std::lock_guard _(Node::Mutex);
		for (auto& session : Node::Nodes)
		{
			if (session.address == address) return;
		}

		Node::Entry node;
		node.address = address;

		Node::Nodes.push_back(node);
	}

	std::vector<Node::Entry> Node::GetNodes()
	{
		std::lock_guard _(Node::Mutex);

		return Node::Nodes;
	}

	void Node::RunFrame()
	{
		if (Dedicated::IsEnabled() && Dedicated::SVLanOnly.get<bool>()) return;

		if (!Dedicated::IsEnabled())
		{
			if (ServerList::useMasterServer) return; // don't run node frame if master server is active

			if (*Game::clcState > 0)
			{
				wasIngame = true;
				return; // don't run while ingame because it can still cause lag spikes on lower end PCs
			}
		}

		if (wasIngame) // our last frame we were ingame and now we aren't so touch all nodes
		{
			for (auto i = Node::Nodes.begin(); i != Node::Nodes.end();++i)
			{
				// clearing the last request and response times makes the 
				// dispatcher think its a new node and will force a refresh
				i->lastRequest.reset();
				i->lastResponse.reset();
			}
			wasIngame = false;
		}

		static Utils::Time::Interval frameLimit;
		int interval = static_cast<int>(1000.0f / Dvar::Var("net_serverFrames").get<int>());
		if (!frameLimit.elapsed(std::chrono::milliseconds(interval))) return;
		frameLimit.update();

		std::lock_guard _(Node::Mutex);
		Dvar::Var queryLimit("net_serverQueryLimit");

		int sentRequests = 0;
		for (auto i = Node::Nodes.begin(); i != Node::Nodes.end();)
		{
			if (i->isDead())
			{
				i = Node::Nodes.erase(i);
				continue;
			}
			if (sentRequests < queryLimit.get<int>() && i->requiresRequest())
			{
				++sentRequests;
				i->sendRequest();
			}

			++i;
		}
	}

	void Node::Synchronize()
	{
		std::lock_guard _(Node::Mutex);
		for (auto& node : Node::Nodes)
		{
			//if (node.isValid()) // Comment out to simulate 'syncnodes' behaviour
			{
				node.reset();
			}
		}
	}

	void Node::HandleResponse(Network::Address address, const std::string& data)
	{
		Proto::Node::List list;
		if (!list.ParseFromString(data)) return;

		Logger::Debug("Received response from {}", address.getCString());

		std::lock_guard _(Node::Mutex);

		for (int i = 0; i < list.nodes_size(); ++i)
		{
			const std::string& addr = list.nodes(i);

			if (addr.size() == sizeof(sockaddr))
			{
				Node::Add(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
			}
		}

		if (list.isnode() && (!list.port() || list.port() == address.getPort()))
		{
			if (!Dedicated::IsEnabled() && ServerList::IsOnlineList() && !ServerList::useMasterServer && list.protocol() == PROTOCOL)
			{
				Logger::Debug("Inserting {} into the serverlist", address.getCString());
				ServerList::InsertRequest(address);
			}
			else
			{
				Logger::Debug("Dropping serverlist insertion for {}", address.getCString());
			}

			for (auto& node : Node::Nodes)
			{
				if (address == node.address)
				{
					if (!node.lastResponse.has_value()) node.lastResponse.emplace(Utils::Time::Point());
					node.lastResponse->update();

					node.data.protocol = list.protocol();
					return;
				}
			}

			Node::Entry entry;
			entry.address = address;
			entry.data.protocol = list.protocol();
			entry.lastResponse.emplace(Utils::Time::Point());

			Node::Nodes.push_back(entry);
		}
	}

	void Node::SendList(const Network::Address& address)
	{
		std::lock_guard _(Node::Mutex);

		// need to keep the message size below 1404 bytes else recipient will just drop it
		std::vector<std::string> nodeListReponseMessages;

		for (std::size_t curNode = 0; curNode < Node::Nodes.size();)
		{
			Proto::Node::List list;
			list.set_isnode(Dedicated::IsEnabled());
			list.set_protocol(PROTOCOL);
			list.set_port(Node::GetPort());

			for (std::size_t i = 0; i < NODE_MAX_NODES_TO_SEND;)
			{
				if (curNode >= Node::Nodes.size())
					break;

				auto node = Node::Nodes.at(curNode++);

				if (node.isValid())
				{
					std::string* str = list.add_nodes();

					sockaddr addr = node.address.getSockAddr();
					str->append(reinterpret_cast<char*>(&addr), sizeof(addr));

					i++;
				}
			}

			nodeListReponseMessages.push_back(list.SerializeAsString());
		}

		auto i = 0;
		for (const auto& nodeListData : nodeListReponseMessages)
		{
			Scheduler::Once([=]
			{
#ifdef DEBUG_NODE
				Logger::Debug("Sending {} nodeListResponse length to {}\n", nodeListData.length(), address.getCString());
#endif
				Session::Send(address, "nodeListResponse", nodeListData);
			}, Scheduler::Pipeline::MAIN, NODE_SEND_RATE * i++);
		}
	}

	unsigned short Node::GetPort()
	{
		if (Dvar::Var("net_natFix").get<bool>()) return 0;
		return Network::GetPort();
	}

	Node::Node()
	{
		if (ZoneBuilder::IsEnabled()) return;
		Dvar::Register<bool>("net_natFix", false, 0, "Fix node registration for certain firewalls/routers");

		Scheduler::Loop([]
		{
			Node::StoreNodes(false);
		}, Scheduler::Pipeline::ASYNC);

		Scheduler::Loop(Node::RunFrame, Scheduler::Pipeline::MAIN);

		Session::Handle("nodeListResponse", Node::HandleResponse);
		Session::Handle("nodeListRequest", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			Node::SendList(address);
		});

		// Load stored nodes
		auto loadNodes = []
		{
			Node::LoadNodePreset();
			Node::LoadNodes();
		};

		if (Monitor::IsEnabled()) Network::OnStart(loadNodes);
		else Scheduler::OnGameInitialized(loadNodes, Scheduler::Pipeline::MAIN);

		Network::OnStart([]
		{
			std::thread([]
			{
				Node::LoadNodeRemotePreset();
			}).detach();
		});

		Command::Add("listnodes", [](Command::Params*)
		{
			Logger::Print("Nodes: {}\n", Node::Nodes.size());

			std::lock_guard _(Node::Mutex);
			for (auto& node : Node::Nodes)
			{
				Logger::Print("{}\t({})\n", node.address.getCString(), node.isValid() ? "Valid" : "Invalid");
			}
		});

		Command::Add("addnode", [](Command::Params* params)
		{
			if (params->size() < 2) return;
			Node::Add({ params->get(1) });
		});
	}

	Node::~Node()
	{
		std::lock_guard _(Node::Mutex);
		Node::StoreNodes(true);
		Node::Nodes.clear();
	}
}
