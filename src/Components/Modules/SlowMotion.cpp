#include <STDInclude.hpp>
#include "SlowMotion.hpp"

namespace Components
{
	int SlowMotion::Delay = 0;

	const Game::dvar_t* SlowMotion::cg_drawDisconnect;

	void SlowMotion::Com_UpdateSlowMotion(int msec)
	{
		if (Delay <= 0)
		{
			Game::Com_UpdateSlowMotion(msec);
		}
		else
		{
			Delay -= msec;
		}
	}

	__declspec(naked) void SlowMotion::Com_UpdateSlowMotion_Stub()
	{
		__asm
		{
			pushad

			push [esp + 0x20 + 0x4]
			call Com_UpdateSlowMotion
			add esp, 0x4

			popad

			retn
		}
	}

	void SlowMotion::ScrCmd_SetSlowMotion_Stub()
	{
		auto duration = 1000;
		auto start = Game::Scr_GetFloat(0);
		auto end = 1.0f;

		if (Game::Scr_GetNumParam() >= 2)
		{
			end = Game::Scr_GetFloat(1);
		}

		if (Game::Scr_GetNumParam() >= 3)
		{
			duration = static_cast<int>(Game::Scr_GetFloat(2) * 1000.0f);
		}

		auto delay = 0;

		if (start > end)
		{
			if (duration < 150)
			{
				delay = duration;
			}
			else
			{
				delay = 150;
			}
		}

		duration = duration - delay;

		Game::Com_SetSlowMotion(start, end, duration);
		Delay = delay;

		// Set snapshot num to 1 behind (T6 does this, why shouldn't we?)
		for (auto i = 0; i < *Game::svs_clientCount; ++i)
		{
			Game::svs_clients[i].nextSnapshotTime = *Game::svs_time - 1;
		}
	}

	void SlowMotion::CG_DrawDisconnect_Stub(const int localClientNum)
	{
		if (cg_drawDisconnect->current.enabled)
		{
			Game::CG_DrawDisconnect(localClientNum);
		}
	}

	SlowMotion::SlowMotion()
	{
		cg_drawDisconnect = Game::Dvar_RegisterBool("cg_drawDisconnect", false, Game::DVAR_NONE, "Draw connection interrupted");

		Delay = 0;
		Utils::Hook(0x5F5FF2, ScrCmd_SetSlowMotion_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x60B38A, Com_UpdateSlowMotion_Stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x4A54ED, CG_DrawDisconnect_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A54FB, CG_DrawDisconnect_Stub, HOOK_CALL).install()->quick();
	}
}
