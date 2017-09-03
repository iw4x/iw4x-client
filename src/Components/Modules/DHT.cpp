#include "STDInclude.hpp"

int dht_random_bytes(void *buf, size_t size)
{
	Utils::Cryptography::Rand::GetRandomBytes(buf, size);
	return INT(size);
}

void dht_hash(void *hash_return, int hash_size, const void *v1, int len1, const void *v2, int len2, const void *v3, int len3)
{
	std::string data;
	data.append(LPSTR(v1), len1);
	data.append(LPSTR(v2), len2);
	data.append(LPSTR(v3), len3);

	Components::DHT::Hash(data, hash_return, size_t(hash_size));
}

int dht_blacklisted(const struct sockaddr* /*sa*/, int /*salen*/)
{
	return 0;
}

extern "C" int dht_gettimeofday(struct timeval *tp, struct timezone* /*tzp*/)
{
	static const unsigned __int64 epoch = 116444736000000000ULL;

	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);

	FILETIME fileTime;
	SystemTimeToFileTime(&systemTime, &fileTime);

	ULARGE_INTEGER ularge;
	ularge.LowPart = fileTime.dwLowDateTime;
	ularge.HighPart = fileTime.dwHighDateTime;

	tp->tv_sec = LONG((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = LONG(systemTime.wMilliseconds * 1000);

	return 0;
}

namespace Components
{
	std::mutex DHT::Mutex;
	std::vector<Network::Address> DHT::Nodes;
	std::map<std::basic_string<uint8_t>, Utils::Slot<void(std::vector<Network::Address>)>> DHT::Handlers;

	void DHT::Insert(std::string data, Utils::Slot<void(std::vector<Network::Address>)> callback)
	{
		unsigned char hash[20];
		DHT::Hash(data, hash, sizeof(hash));
		DHT::InsertHash(hash, callback);
	}

	void DHT::InsertHash(char* hash, Utils::Slot<void(std::vector<Network::Address>)> callback)
	{
		DHT::InsertHash(reinterpret_cast<unsigned char*>(hash), callback);
	}

	void DHT::InsertHash(unsigned char* hash, Utils::Slot<void(std::vector<Network::Address>)> callback)
	{
		std::basic_string<uint8_t> hashStr(hash, 20);
		DHT::Handlers[hashStr] = callback;
	}

	void DHT::BootstrapDone()
	{
		// Tell the game we got our external ip
		Utils::Hook::Set<BYTE>(0x649D6F0, 1);
	}

	void DHT::Hash(std::string data, void* out, size_t size)
	{
		ZeroMemory(out, size);

		for (size_t i = 0; i < size; ++i)
		{
			std::string hashData = data;
			hashData.append(LPSTR(out), size);

			std::string hash = Utils::Cryptography::SHA512::Compute(hashData);
			std::memmove(LPSTR(out) + i, hash.data(), std::min(size - i, hash.size()));
		}
	}

	void DHT::Callback(void* /*closure*/, int event, const unsigned char* info_hash, const void* data, size_t data_len)
	{
		if (event == DHT_EVENT_VALUES)
		{
			std::basic_string<uint8_t> hashStr(info_hash, 20);
			auto handler = DHT::Handlers.find(hashStr);
			if (handler != DHT::Handlers.end())
			{
				std::vector<Network::Address> addresses;

				const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
				while ((data_len - (LPSTR(bytes) - LPSTR(data))) >= 6)
				{
					unsigned char ip[4];
					ip[0] = *bytes++;
					ip[1] = *bytes++;
					ip[2] = *bytes++;
					ip[3] = *bytes++;

					unsigned short port = ntohs(*reinterpret_cast<const unsigned short*>(bytes));
					bytes += 2;

					Network::Address address(Utils::String::VA("%u.%u.%u.%u:%hu", ip[0], ip[1], ip[2], ip[3], port));
					addresses.push_back(address);
				}

				handler->second(addresses);
			}
		}
		else if (event == DHT_EVENT_SEARCH_DONE)
		{
			Logger::Print("Search done!\n");
		}
	}

	void DHT::OnData(char* buf, int len, sockaddr* from, int fromlen)
	{
		time_t tosleep = 0;
		dht_periodic(buf, len, from, fromlen, &tosleep, DHT::Callback, NULL);
	}

	void DHT::StoreNodes(bool force)
	{
		static Utils::Time::Interval interval;
		if (!force && !interval.elapsed(1min)) return;
		interval.update();

		Utils::Memory::Allocator allocator;
		int nodes = dht_nodes(AF_INET, nullptr, nullptr, nullptr, nullptr) + 10;

		sockaddr_in* addresses = allocator.allocateArray<sockaddr_in>(nodes);
		DHT::Id* ids = allocator.allocateArray<DHT::Id>(nodes);

		int num6 = 0;
		dht_get_nodes(addresses, reinterpret_cast<unsigned char*>(ids), &nodes, nullptr, nullptr, &num6);

		// We could be offline, don't overwrite anything
		if (nodes <= 0) return;

		Proto::Network::Nodes protoNodes;

		for (int i = 0; i < nodes; ++i)
		{
			Proto::Network::Node* node = protoNodes.add_nodes();

			node->mutable_id()->clear();
			node->mutable_id()->append(LPSTR(ids + i), sizeof(*ids));

			node->mutable_address()->clear();
			node->mutable_address()->append(LPSTR(addresses + i), sizeof(*addresses));
		}

		Utils::IO::WriteFile(Utils::String::VA("players/dht/%hu.nodes", Network::GetPort()), protoNodes.SerializeAsString());
	}

	void DHT::LoadNodes()
	{
		std::string file = Utils::String::VA("players/dht/%hu.nodes", Network::GetPort());
		if (!Utils::IO::FileExists(file)) return;

		auto data = Utils::IO::ReadFile(file);

		Proto::Network::Nodes nodes;
		if (!nodes.ParseFromString(data)) return;

		for (auto& node : nodes.nodes())
		{
			const std::string& id = node.id();
			const std::string& address = node.address();

			if (id.size() == 20)
			{
				dht_insert_node(reinterpret_cast<const unsigned char*>(id.data()), reinterpret_cast<sockaddr*>(LPSTR(address.data())), INT(address.size()));
			}
		}
	}

	void DHT::Add(Network::Address addr)
	{
		std::lock_guard<std::mutex> _(DHT::Mutex);
		
		if (std::find(DHT::Nodes.begin(), DHT::Nodes.end(), addr) == DHT::Nodes.end())
		{
			DHT::Nodes.push_back(addr);
		}

		if (ServerList::IsOnlineList())
		{
			ServerList::InsertRequest(addr);
		}
	}

	void DHT::Search()
	{
		// Serverlist specific
		{
			std::lock_guard<std::mutex> _(DHT::Mutex);
			for (auto& address : DHT::Nodes)
			{
				if (ServerList::IsOnlineList())
				{
					ServerList::InsertRequest(address);
				}
			}
		}

		for (auto& hash : DHT::Handlers)
		{
			dht_search(hash.first.data(), (Dedicated::IsEnabled() ? (Dvar::Var("sv_lanOnly").get<bool>() ? 0 : Network::GetPort()) : 0), AF_INET, DHT::Callback, nullptr);
		}
	}

	void DHT::RunFrame()
	{
		time_t tosleep = 0;
		dht_periodic(NULL, 0, NULL, 0, &tosleep, DHT::Callback, NULL);
		DHT::StoreNodes(false);

		static std::optional<Utils::Time::Interval> interval;
		if (!interval.has_value() || interval->elapsed(2min))
		{
			if (!interval.has_value()) interval.emplace();
			interval->update();

			DHT::Search();
		}
	}

	void DHT::Bootstrap(Network::Address node)
	{
		sockaddr addr = node.getSockAddr();
		dht_ping_node(&addr, sizeof(addr));
	}

	DHT::DHT()
	{
// #ifdef DEBUG
// 		AllocConsole();
// 		AttachConsole(GetCurrentProcessId());
// 
// 		freopen("conin$", "r", stdin);
// 		freopen("conout$", "w", stdout);
// 		freopen("conout$", "w", stderr);
// 		dht_debug = stdout;
// #endif

		Network::OnStart([]()
		{
			std::string idData;

			std::string file = Utils::String::VA("players/dht/%hu.id", Network::GetPort());
			if (Utils::IO::FileExists(file))
			{
				idData = Utils::IO::ReadFile(file);
			}

			if (idData.size() != 20)
			{
				idData = Utils::Cryptography::Rand::GetRandomBytes(20);
				Utils::IO::WriteFile(file, idData);
			}

			dht_init(INT(*Game::ip_socket), -1, reinterpret_cast<unsigned char*>(idData.data()), reinterpret_cast<unsigned char*>("JC\0\0"));

			DHT::LoadNodes();
			Scheduler::OnFrame(DHT::RunFrame);

			DHT::Bootstrap("router.bittorrent.com:6881");
			DHT::Bootstrap("dht.transmissionbt.com:6881");
 			DHT::Bootstrap("dht.aelitis.com:6881");
		});

		auto callback = [](std::vector<Network::Address> addresses)
		{
			std::lock_guard<std::mutex> _(DHT::Mutex);

			DHT::BootstrapDone();

			for (auto& address : addresses)
			{
				if (std::find(DHT::Nodes.begin(), DHT::Nodes.end(), address) == DHT::Nodes.end())
				{
					DHT::Nodes.push_back(address);
				}
			}

			for (auto& address : addresses)
			{
				if (ServerList::IsOnlineList())
				{
					ServerList::InsertRequest(address);
				}

				Logger::Print("Received %s\n", address.getCString());
			}
		};

		DHT::Insert("xPROTO_IW4x", callback);

		Command::Add("addnode", [](Command::Params* params)
		{
			if (params->length() >= 2)
			{
				DHT::Add(params->get(1));
			}
		});
	}

	void DHT::preDestroy()
	{
		DHT::Handlers.clear();

		DHT::StoreNodes(true);
		dht_uninit();
	}
}
