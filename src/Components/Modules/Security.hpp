#pragma once

namespace Components
{
	class Security : public Component
	{
	public:
		Security();

		static int MsgReadBitsCompressCheckSV(const unsigned char* from, unsigned char* to, int size);
		static int MsgReadBitsCompressCheckCL(const unsigned char* from, unsigned char* to, int size);

	private:
		static int SVCanReplaceServerCommand(Game::client_t* client, const char* cmd);

		static long AtolAdjustPlayerLimit(const char* string);

		static void SelectStringTableEntryInDvarStub();

		static int G_GetClientScore();

		static void G_LogPrintfStub(const char* fmt);

		static void NET_DeferPacketToClientStub(Game::netadr_t* net_from, Game::msg_t* net_message);
	};
}
