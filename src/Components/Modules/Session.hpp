#pragma once

#define SESSION_TIMEOUT (10 * 1000) //10s
#define SESSION_MAX_RETRIES 3
#define SESSION_REQUEST_LIMIT 3

namespace Components
{
	class Session : public Component
	{
	public:
		class Packet
		{
		public:
			std::string command;
			std::string data;

			unsigned int tries;
			std::optional<Utils::Time::Point> lastTry;
		};

		class Frame
		{
		public:
			std::string challenge;
			Utils::Time::Point creationPoint;
		};

		Session();
		~Session();

		bool unitTest() override;

		static void Send(Network::Address target, std::string command, std::string data = "");
		static void Handle(std::string packet, Utils::Slot<Network::Callback> callback);

		static void RunFrame();

	private:
		static std::recursive_mutex Mutex;
		static std::unordered_map<Network::Address, Frame> Sessions;
		static std::unordered_map<Network::Address, std::queue<std::shared_ptr<Packet>>> PacketQueue;

		static Utils::Cryptography::ECC::Key SignatureKey;

		static std::map<std::string, Utils::Slot<Network::Callback>> PacketHandlers;
	};
}
