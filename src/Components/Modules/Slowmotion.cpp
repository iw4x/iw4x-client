#include "STDInclude.hpp"

namespace Components
{
	int SlowMotion::Delay = 0;

	void SlowMotion::ApplySlowMotion(int timePassed)
	{
		if (SlowMotion::Delay <= 0)
		{
			Utils::Hook::Call<void(int)>(0x60B2D0)(timePassed);
		}
		else
		{
			SlowMotion::Delay -= timePassed;
		}
	}

	__declspec(naked) void SlowMotion::ApplySlowMotionStub()
	{
		__asm
		{
			pushad
			push [esp + 20h]

			call SlowMotion::ApplySlowMotion

			pop ecx
			popad

			retn
		}
	}

	void SlowMotion::SetSlowMotion()
	{
		int duration = 1000;
		float start = Game::Scr_GetFloat(0);
		float end = 1.0f;

		if (Game::Scr_GetNumParam() >= 2)
		{
			end = Game::Scr_GetFloat(1);
		}

		if (Game::Scr_GetNumParam() >= 3)
		{
			duration = static_cast<int>(Game::Scr_GetFloat(2) * 1000.0);
		}

		int delay = 0;

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
		SlowMotion::Delay = delay;

		// set snapshot num to 1 behind (T6 does this, why shouldn't we?)
		for (int i = 0; i < *Game::svs_numclients; ++i)
		{
			Game::svs_clients[i].snapNum = (*reinterpret_cast<DWORD*>(0x31D9384)) - 1;
		}
	}

	SlowMotion::SlowMotion()
	{
		if (Dedicated::IsEnabled())
		{
			SlowMotion::Delay = 0;
			Utils::Hook(0x5F5FF2, SlowMotion::SetSlowMotion, HOOK_JUMP).install()->quick();
			Utils::Hook(0x60B38A, SlowMotion::ApplySlowMotionStub, HOOK_CALL).install()->quick();
		}
	}
}
