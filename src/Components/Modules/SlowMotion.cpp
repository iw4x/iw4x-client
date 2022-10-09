#include <STDInclude.hpp>

namespace Components
{
	int SlowMotion::Delay = 0;

	void SlowMotion::ApplySlowMotion(int timePassed)
	{
		if (Delay <= 0)
		{
			Utils::Hook::Call<void(int)>(0x60B2D0)(timePassed);
		}
		else
		{
			Delay -= timePassed;
		}
	}

	__declspec(naked) void SlowMotion::ApplySlowMotionStub()
	{
		__asm
		{
			pushad

			push [esp + 24h]
			call ApplySlowMotion
			add esp, 4h

			popad

			retn
		}
	}

	void SlowMotion::SetSlowMotion()
	{
		auto duration = 1000;
		auto start = Game::Scr_GetFloat(0);
		auto end = 1.0f;

		if (Game::Scr_GetNumParam() >= 2u)
		{
			end = Game::Scr_GetFloat(1);
		}

		if (Game::Scr_GetNumParam() >= 3u)
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

		// set snapshot num to 1 behind (T6 does this, why shouldn't we?)
		for (auto i = 0; i < *Game::svs_clientCount; ++i)
		{
			Game::svs_clients[i].nextSnapshotTime = *Game::svs_time - 1;
		}
	}

	SlowMotion::SlowMotion()
	{
		Delay = 0;
		Utils::Hook(0x5F5FF2, SetSlowMotion, HOOK_JUMP).install()->quick();
		Utils::Hook(0x60B38A, ApplySlowMotionStub, HOOK_CALL).install()->quick();


		Utils::Hook::Nop(0x4A54ED, 5);
		Utils::Hook::Nop(0x4A54FB, 5);
	}
}
