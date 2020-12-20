#include "STDInclude.hpp"

#ifdef COMPILE_IW4MVM
#include <IW4MVM/client_main.h>
#endif

namespace Components
{
	IW4MVM::IW4MVM()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled() || Monitor::IsEnabled() || Loader::IsPerformingUnitTests()) return;

		DWORD oldProtect;
		std::uint8_t* _module = reinterpret_cast<std::uint8_t*>(GetModuleHandle(nullptr));
		VirtualProtect(_module + 0x1000, 0x2D6000, PAGE_EXECUTE_READWRITE, &oldProtect);

#ifdef COMPILE_IW4MVM
		client_main::Init();
		Scheduler::Once(client_main::PostInit);
#endif
		Scheduler::OnFrame([]()
		{
			if (!Game::CL_IsCgameInitialized())
			{
				Dvar::Var("com_timescale").setRaw(1.0f);
			}
		});

		VirtualProtect(_module + 0x1000, 0x2D6000, PAGE_EXECUTE_READ, &oldProtect);
	}

	IW4MVM::~IW4MVM()
	{

	}
}
