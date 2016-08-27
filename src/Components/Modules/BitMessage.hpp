#pragma once

#ifndef DISABLE_BITMESSAGE

#define BITMESSAGE_KEYS_FILENAME std::string("players/bmkey.dat")
#define BITMESSAGE_OBJECT_STORAGE_FILENAME std::string("players/bmstore.dat")

static const std::map<std::string, unsigned short> bitmessageKnownNodes = {
	// https://github.com/Bitmessage/PyBitmessage/blob/4622d952e47a7dbb3a90aa79f4d20163aa14b041/src/defaultKnownNodes.py#L15-L23

	// Stream 1
	//{ "2604:2000:1380:9f:82e:148b:2746:d0c7", 8080 },
	{ "5.45.99.75", 8444 },
	{ "75.167.159.54", 8444 },
	{ "95.165.168.168", 8444 },
	{ "85.180.139.241", 8444 },
	{ "158.222.211.81", 8080 },
	{ "178.62.12.187", 8448 },
	{ "24.188.198.204", 8111 },
	{ "109.147.204.113", 1195 },
	{ "178.11.46.221", 8444 },

	// Stream 2 has none yet

	// Stream 3 has none yet
};

namespace Components
{
	class BitMessage : public Component
	{
	public:
		BitMessage();

#ifdef DEBUG
		const char* GetName() { return "BitMessage"; };
#endif

		static BitMessage* Singleton;
		BitMRC* BMClient;

	private:
		bool InitAddr();
	};
}

#endif