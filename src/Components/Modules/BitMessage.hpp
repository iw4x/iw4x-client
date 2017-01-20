#pragma once

#ifndef DISABLE_BITMESSAGE

#define BITMESSAGE_KEYS_FILENAME "players/bmk.dat"s
#define BITMESSAGE_OBJECT_STORAGE_FILENAME "players/storage.dat"s

namespace Components
{
	class BitMessage : public Component
	{
	public:
		BitMessage();
		~BitMessage();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "BitMessage"; };
#endif

		static void SetDefaultTTL(time_t ttl);
		static bool RequestPublicKey(std::string targetAddress);
		static bool WaitForPublicKey(std::string targetAddress);
		static bool Subscribe(std::string targetAddress);
		static bool SendMsg(std::string targetAddress, std::string message, time_t ttl = 0);
		static bool SendBroadcast(std::string message, time_t ttl = 0);
		static void Save();

		static BitMRC* BMClient;

	private:
		static std::thread ShutDownThread;

		static PubAddr* FindPublicKey(PubAddr addr);
		static bool InitAddr();

		static void ShutDown();
	};
}

#endif
