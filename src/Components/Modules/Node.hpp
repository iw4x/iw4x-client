#pragma once

#define NODE_HALFLIFE (3 * 60 * 1000) //3min
#define NODE_REQUEST_LIMIT 10

namespace Components
{
	class Node : public Component
	{
	public:
		class Data
		{
		public:
			uint64_t protocol;
		};

		class Entry
		{
		public:
			Network::Address address;
			Data data;

			std::optional<Utils::Time::Point> lastRequest;
			std::optional<Utils::Time::Point> lastResponse;
			Utils::Time::Point creationPoint;

			bool isValid();
			bool isDead();

			bool requiresRequest();
			void sendRequest();

			void reset();
		};

		Node();
		~Node();

		static void Add(Network::Address address);
		static void RunFrame();
		static void Synchronize();

		static void LoadNodeRemotePreset();

	private:
		static std::recursive_mutex Mutex;
		static std::vector<Entry> Nodes;

		static void HandleResponse(Network::Address address, std::string data);

		static void SendList(Network::Address address);

		static void LoadNodePreset();
		static void LoadNodes();
		static void StoreNodes(bool force);

		static unsigned short GetPort();
	};
}
