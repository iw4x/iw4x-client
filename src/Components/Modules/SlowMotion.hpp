#pragma once

namespace Components
{
	class SlowMotion : public Component
	{
	public:
		SlowMotion();

	private:
		static int Delay;

		static const Game::dvar_t* cg_drawDisconnect;

		static void Com_UpdateSlowMotion(int timePassed);
		static void Com_UpdateSlowMotion_Stub();

		static void ScrCmd_SetSlowMotion_Stub();

		static void CG_DrawDisconnect_Stub(int localClientNum);
	};
}
