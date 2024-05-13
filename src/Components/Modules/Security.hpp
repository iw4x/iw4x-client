#pragma once

namespace Components
{
	class Security : public Component
	{
	public:
		Security();

	private:
		static long AtolAdjustPlayerLimit(const char* string);

		static void SelectStringTableEntryInDvar_Stub();

		static int G_GetClientScore_Hk();

		static void G_LogPrintf_Stub(const char* fmt);

		static void NET_DeferPacketToClient_Hk(Game::netadr_t* net_from, Game::msg_t* net_message);

		static void SV_ExecuteClientMessage_Stub(Game::client_s* client, Game::msg_t* msg);
	};
}
