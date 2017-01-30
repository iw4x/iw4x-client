#pragma once

namespace Components
{
	class IPCHandler : public Component
	{
	public:
		typedef Utils::Slot<void(std::string)> Callback;

		IPCHandler();
		~IPCHandler();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "IPCHandler"; };
#endif

		static void SendClient(std::string message, std::string data);
		static void OnClient(std::string message, Callback callback);

	private:
		static std::unique_ptr<Utils::IPC::BidirectionalChannel> ClientChannel;
		static std::unordered_map<std::string, Callback> ClientCallbacks;

		static void InitChannel();
		static void HandleClient();
	};
}
