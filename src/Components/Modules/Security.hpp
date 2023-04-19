#pragma once

namespace Components
{
	class Security : public Component
	{
	public:
		Security();

		static int Msg_ReadBitsCompressCheckSV(const unsigned char* from, unsigned char* to, int size);
		static int Msg_ReadBitsCompressCheckCL(const unsigned char* from, unsigned char* to, int size);

	private:
		static int SV_CanReplaceServerCommand_Hk(Game::client_t* client, const char* cmd);

		static long AtolAdjustPlayerLimit(const char* string);

		static void SelectStringTableEntryInDvar_Stub();

		static int G_GetClientScore_Hk();

		static void G_LogPrintf_Stub(const char* fmt);

		static void NET_DeferPacketToClient_Hk(Game::netadr_t* net_from, Game::msg_t* net_message);

		static void SV_ExecuteClientMessage_Stub(Game::client_t* client, Game::msg_t* msg);
	};
}
