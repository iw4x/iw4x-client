#pragma once

#define DHT_PORT 9000
#define DHT_RANGE 100

namespace Components
{
	class DHT : public Component
	{
	public:
		typedef unsigned char Id[20];

		DHT();
		~DHT();

		static void OnData(std::string data, Network::Address address);

		static void Insert(std::string data, Utils::Slot<void(std::vector<Network::Address>)> callback);

		static void InsertHash(char* hash, Utils::Slot<void(std::vector<Network::Address>)> callback);
		static void InsertHash(unsigned char* hash, Utils::Slot<void(std::vector<Network::Address>)> callback);

		static void RunFrame();

		static void Search();

		static void Add(Network::Address addr);

		static void Hash(std::string data, void* out, size_t size);

	private:
		static SOCKET Socket;
		static std::mutex Mutex;
		static std::vector<Network::Address> Nodes;
		static std::map<std::basic_string<uint8_t>, Utils::Slot<void(std::vector<Network::Address>)>> Handlers;

		static void Callback(void *closure, int event, const unsigned char *info_hash, const void *data, size_t data_len);

		static void StoreNodes(bool force);
		static void LoadNodes();

		static void InitSocket();
		static void SocketFrame();

		static void Bootstrap(Network::Address node);
	};
}
