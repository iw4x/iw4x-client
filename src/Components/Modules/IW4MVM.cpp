#include "STDInclude.hpp"
#include <IW4MVM/client_main.h>

namespace Components
{
	IW4MVM::IW4MVM()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled() || Monitor::IsEnabled() || Loader::IsPerformingUnitTests()) return;

		DWORD oldProtect;
		std::uint8_t* module = reinterpret_cast<std::uint8_t*>(GetModuleHandle(nullptr));
		VirtualProtect(module + 0x1000, 0x2D6000, PAGE_EXECUTE_READWRITE, &oldProtect);

		client_main::Init();
		Scheduler::Once(client_main::PostInit);

		VirtualProtect(module + 0x1000, 0x2D6000, PAGE_EXECUTE_READ, &oldProtect);
	}

	IW4MVM::~IW4MVM()
	{

	}
}
