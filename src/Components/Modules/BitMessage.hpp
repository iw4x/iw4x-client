#pragma once

#ifndef DISABLE_BITMESSAGE

#define BITMESSAGE_KEYS_FILENAME std::string("players/bmkeys.dat")
#define BITMESSAGE_OBJECT_STORAGE_FILENAME std::string("players/bmstore.dat")

namespace Components
{
	class BitMessage : public Component
	{
	public:
		BitMessage();
		~BitMessage();

#ifdef DEBUG
		const char* GetName() { return "BitMessage"; };
#endif

		void SetDefaultTTL(time_t ttl);
		bool RequestPublicKey(std::string targetAddress);
		bool RequestAndWaitForPublicKey(std::string targetAddress);
		bool Subscribe(std::string targetAddress);
		bool SendMsg(std::string targetAddress, std::string message, time_t ttl = 0);
		bool SendBroadcast(std::string message, time_t ttl = 0);
		void Save();

		static BitMessage* Singleton;
		BitMRC* BMClient;

	private:
		bool InitAddr();
	};
}

#endif