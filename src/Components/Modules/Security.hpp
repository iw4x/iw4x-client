#pragma once

namespace Components
{
	class Security : public Component
	{
	public:
		Security();

	private:
		static int MsgReadBitsCompressCheckSV(const char* from, char* to, int size);
		static int MsgReadBitsCompressCheckCL(const char* from, char* to, int size);

		static int SVCanReplaceServerCommand(Game::client_t* client, const char* cmd);

		static long AtolAdjustPlayerLimit(const char* string);

		static void SelectStringTableEntryInDvarStub();

		static int G_GetClientScore();

		static void G_LogPrintfStub(const char* fmt);

		static void NET_DeferPacketToClientStub(Game::netadr_t* net_from, Game::msg_t* net_message);
	};
}
