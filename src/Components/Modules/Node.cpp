#include <STDInclude.hpp>
#include <Utils/Compression.hpp>
#include <Utils/InfoString.hpp>

#include <proto/node.pb.h>

#include "Node.hpp"
#include "ServerList.hpp"
#include "Session.hpp"

namespace Components
{
	std::recursive_mutex Node::Mutex;
	std::vector<Node::Entry> Node::Nodes;

	bool Node::WasIngame = false;

	const Game::dvar_t* Node::net_natFix;

	bool Node::Entry::isValid() const
	{
		return (this->lastResponse.has_value() && !this->lastResponse->elapsed(NODE_HALFLIFE * 2));
	}

	bool Node::Entry::isDead() const
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

	bool Node::Entry::requiresRequest() const
	{
		return (!this->isDead() && (!this->lastRequest.has_value() || this->lastRequest->elapsed(NODE_HALFLIFE)));
	}

	void Node::Entry::sendRequest()
	{
		if (!this->lastRequest.has_value()) this->lastRequest.emplace(Utils::Time::Point());
		this->lastRequest->update();

		Session::Send(this->address, "nodeListRequest");
		SendList(this->address);
#ifdef NODE_SYSTEM_DEBUG
		Logger::Debug("Sent request to {}", this->address.getString());
#endif
	}

	void Node::Entry::reset()
	{
		this->lastRequest.reset();
	}

	void Node::LoadNodePreset()
	{
		Proto::Node::List list;

		FileSystem::File defaultNodes("nodes_default.dat");
		if (!defaultNodes.exists() || !list.ParseFromString(Utils::Compression::ZLib::Decompress(defaultNodes.getBuffer()))) return;

		for (auto i = 0; i < list.nodes_size(); ++i)
		{
			const auto& addr = list.nodes(i);
			if (addr.size() == sizeof(sockaddr))
			{
				Add(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
			}
		}
	}

	void Node::LoadNodes()
	{
		std::string data;
		if (!Utils::IO::ReadFile("players/nodes.json", &data) || data.empty())
		{
			return;
		}

		nlohmann::json nodes;
		try
		{
			nodes = nlohmann::json::parse(data);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
			return;
		}

		if (!nodes.contains("nodes"))
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "nodes.json contains invalid data\n");
			return;
		}

		const auto& list = nodes["nodes"];
		if (!list.is_array())
		{
			return;
		}

		const nlohmann::json::array_t arr = list;
		Logger::Print("Parsing {} nodes from nodes.json\n", arr.size());

		for (const auto& entry : arr)
		{
			if (entry.is_string())
			{
				Network::Address address(entry.get<std::string>());
				Add(address);
			}
		}
	}

	void Node::StoreNodes(bool force)
	{
		if (Dedicated::IsEnabled() && Dedicated::SVLanOnly.get<bool>()) return;

		std::vector<std::string> nodes;

		static Utils::Time::Interval interval;
		if (!force && !interval.elapsed(1min)) return;
		interval.update();

		Mutex.lock();

		for (auto& node : Nodes)
		{
			if (node.isValid() || force)
			{
				const auto address = node.address.getString();
				nodes.emplace_back(address);
			}
		}

		Mutex.unlock();

		nlohmann::json out;
		out["nodes"] = nodes;

		Utils::IO::WriteFile("players/nodes.json", out.dump());
	}

	void Node::Add(const Network::Address& address)
	{
#ifndef DEBUG
		if (address.isLocal() || address.isSelf()) return;
#endif

		if (!address.isValid()) return;

		std::lock_guard _(Mutex);
		for (auto& session : Nodes)
		{
			if (session.address == address) return;
		}

		Entry node;
		node.address = address;

		Nodes.push_back(node);
	}

	std::vector<Node::Entry> Node::GetNodes()
	{
		std::lock_guard _(Mutex);

		return Nodes;
	}

	void Node::RunFrame()
	{
		if (Dedicated::IsEnabled() && Dedicated::SVLanOnly.get<bool>()) return;

		if (!Dedicated::IsEnabled())
		{
			if (ServerList::UseMasterServer) return; // don't run node frame if master server is active

			if (Game::CL_GetLocalClientConnectionState(0) != Game::CA_DISCONNECTED)
			{
				WasIngame = true;
				return; // don't run while in-game because it can still cause lag spikes on lower end PCs
			}
		}

		if (WasIngame) // our last frame we were in-game and now we aren't so touch all nodes
		{
			for (auto& entry : Nodes)
			{
				// clearing the last request and response times makes the
				// dispatcher think its a new node and will force a refresh
				entry.lastRequest.reset();
				entry.lastResponse.reset();
			}

			WasIngame = false;
		}

		static Utils::Time::Interval frameLimit;
		const auto interval = 1000 / ServerList::NETServerFrames.get<int>();
		if (!frameLimit.elapsed(std::chrono::milliseconds(interval))) return;
		frameLimit.update();

		std::lock_guard _(Mutex);

		int sentRequests = 0;
		for (auto i = Nodes.begin(); i != Nodes.end();)
		{
			if (i->isDead())
			{
				i = Nodes.erase(i);
				continue;
			}

			if (sentRequests < ServerList::NETServerQueryLimit.get<int>() && i->requiresRequest())
			{
				++sentRequests;
				i->sendRequest();
			}

			++i;
		}
	}

	void Node::Synchronize()
	{
		std::lock_guard _(Mutex);
		for (auto& node : Nodes)
		{
			{
				node.reset();
			}
		}
	}

	void Node::HandleResponse(const Network::Address& address, const std::string& data)
	{
		Proto::Node::List list;
		if (!list.ParseFromString(data)) return;

#ifdef NODE_SYSTEM_DEBUG
		Logger::Debug("Received response from {}", address.getString());
#endif

		std::lock_guard _(Mutex);

		for (int i = 0; i < list.nodes_size(); ++i)
		{
			const std::string& addr = list.nodes(i);

			if (addr.size() == sizeof(sockaddr))
			{
				Add(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
			}
		}

		if (list.isnode() && (!list.port() || list.port() == address.getPort()))
		{
			if (!Dedicated::IsEnabled() && ServerList::IsOnlineList() && !ServerList::UseMasterServer && list.protocol() == PROTOCOL)
			{
#ifdef NODE_SYSTEM_DEBUG
				Logger::Debug("Inserting {} into the serverlist", address.getString());
#endif
				ServerList::InsertRequest(address);
			}
			else
			{
#ifdef NODE_SYSTEM_DEBUG
				Logger::Debug("Dropping serverlist insertion for {}", address.getString());
#endif
			}

			for (auto& node : Nodes)
			{
				if (address == node.address)
				{
					if (!node.lastResponse.has_value()) node.lastResponse.emplace(Utils::Time::Point());
					node.lastResponse->update();

					node.data.protocol = list.protocol();
					return;
				}
			}

			Entry entry;
			entry.address = address;
			entry.data.protocol = list.protocol();
			entry.lastResponse.emplace(Utils::Time::Point());

			Nodes.push_back(entry);
		}
	}

	void Node::SendList(const Network::Address& address)
	{
		std::lock_guard _(Mutex);

		// need to keep the message size below 1404 bytes else recipient will just drop it
		std::vector<std::string> nodeListReponseMessages;

		for (std::size_t curNode = 0; curNode < Nodes.size();)
		{
			Proto::Node::List list;
			list.set_isnode(Dedicated::IsEnabled());
			list.set_protocol(PROTOCOL);
			list.set_port(GetPort());

			for (std::size_t i = 0; i < NODE_MAX_NODES_TO_SEND;)
			{
				if (curNode >= Nodes.size())
				{
					break;
				}

				auto& node = Nodes.at(curNode++);

				if (node.isValid())
				{
					auto* str = list.add_nodes();

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
#ifdef NODE_SYSTEM_DEBUG
					Logger::Debug("Sending {} nodeListResponse length to {}\n", nodeListData.length(), address.getCString());
#endif
					Session::Send(address, "nodeListResponse", nodeListData);
				}, Scheduler::Pipeline::MAIN, NODE_SEND_RATE * i++);
		}
	}

	std::uint16_t Node::GetPort()
	{
		if (net_natFix->current.enabled) return 0;
		return Network::GetPort();
	}

	void Node::Migrate()
	{
		Proto::Node::List list;
		std::string nodes;

		if (!Utils::IO::ReadFile("players/nodes.dat", &nodes) || nodes.empty())
		{
			return;
		}

		if (!list.ParseFromString(Utils::Compression::ZLib::Decompress(nodes)))
		{
			return;
		}

		std::vector<std::string> data;
		for (auto i = 0; i < list.nodes_size(); ++i)
		{
			const std::string& addr = list.nodes(i);

			if (addr.size() == sizeof(sockaddr))
			{
				Network::Address address(reinterpret_cast<sockaddr*>(const_cast<char*>(addr.data())));
				data.emplace_back(address.getString());
			}
		}

		nlohmann::json out;
		out["nodes"] = data;

		if (!Utils::IO::FileExists("players/nodes.json"))
		{
			Utils::IO::WriteFile("players/nodes.json", out.dump());
		}

		Utils::IO::RemoveFile("players/nodes.dat");
	}

	Node::Node()
	{
		if (ZoneBuilder::IsEnabled())
		{
			return;
		}

		net_natFix = Game::Dvar_RegisterBool("net_natFix", false, 0, "Fix node registration for certain firewalls/routers");

		Scheduler::Loop([]
			{
				StoreNodes(false);
			}, Scheduler::Pipeline::ASYNC, 5min);

		Scheduler::Loop(RunFrame, Scheduler::Pipeline::MAIN);

		Scheduler::OnGameInitialized([]
			{

				Session::Handle("nodeListResponse", HandleResponse);
				Session::Handle("nodeListRequest", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
					{
						SendList(address);
					});

				Migrate();
				LoadNodePreset();
				LoadNodes();
			}, Scheduler::Pipeline::MAIN);

		Command::Add("listNodes", [](const Command::Params*)
			{
				Logger::Print("Nodes: {}\n", Nodes.size());

				std::lock_guard _(Mutex);
				for (const auto& node : Nodes)
				{
					Logger::Print("{}\t({})\n", node.address.getString(), node.isValid() ? "Valid" : "Invalid");
				}
			});

		Command::Add("addNode", [](const Command::Params* params)
			{
				if (params->size() < 2) return;
				auto address = Network::Address{ params->get(1) };
				if (address.isValid())
				{
					Add(address);
				}
			});
	}

	void Node::preDestroy()
	{
		std::lock_guard _(Mutex);
		StoreNodes(true);
		Nodes.clear();
	}
}
