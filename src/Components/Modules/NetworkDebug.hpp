#pragma once

namespace Components
{
	class NetworkDebug : public Component
	{
	public:
		NetworkDebug();

	private:
		static void CL_ParseServerMessage_Hk(Game::msg_t* msg);

		static void CL_ParseBadPacket_f();
	};
}
