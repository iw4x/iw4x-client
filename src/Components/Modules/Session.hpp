#pragma once

#define SESSION_TIMEOUT (10 * 1000) //10s
#define SESSION_MAX_RETRIES 3
#define SESSION_REQUEST_LIMIT 10

#define DISABLE_SESSION

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
		void preDestroy() override;

		static void Send(const Network::Address& target, const std::string& command, const std::string& data = "");
		static void Handle(const std::string& packet, const Network::networkCallback& callback);

	private:
		static volatile bool Terminate;
		static std::thread Thread;
		static std::recursive_mutex Mutex;
		static std::unordered_map<Network::Address, Frame> Sessions;
		static std::unordered_map<Network::Address, std::queue<std::shared_ptr<Packet>>> PacketQueue;

		static Utils::Cryptography::ECC::Key SignatureKey;

		static std::unordered_map<std::string, Network::networkCallback> PacketHandlers;

		static std::queue<std::pair<Network::Address, std::string>> SignatureQueue;

		static void RunFrame();
		static void HandleSignatures();
	};
}
