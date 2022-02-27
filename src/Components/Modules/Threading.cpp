#include <STDInclude.hpp>

namespace Components
{
	__declspec(naked) void Threading::FrameEpilogueStub()
	{
		__asm
		{
			pop edi
			pop esi
			pop ebx
			mov esp, ebp
			pop ebp
			retn
		}
	}

	__declspec(naked) void Threading::PacketEventStub()
	{
		__asm
		{
			mov eax, 49F0B0h
			call eax
			mov eax, 458160h
			jmp eax
		}
	}

	Threading::Threading()
	{
		// remove starting of server thread from Com_Init_Try_Block_Function
		Utils::Hook::Nop(0x60BEC0, 5);

		// make server thread function jump to per-frame stuff
		Utils::Hook(0x627049, 0x6271CE, HOOK_JUMP).install()->quick();

		// make SV_WaitServer insta-return
		Utils::Hook::Set<BYTE>(0x4256F0, 0xC3);

		// dvar setting function, unknown stuff related to server thread sync
		Utils::Hook::Set<BYTE>(0x647781, 0xEB);

		Utils::Hook(0x627695, 0x627040, HOOK_CALL).install()->quick();
		Utils::Hook(0x43D1C7, Threading::PacketEventStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x6272E3, Threading::FrameEpilogueStub, HOOK_JUMP).install()->quick();

		// Make VA thread safe
		Utils::Hook(0x4785B0, Utils::String::VA, HOOK_JUMP).install()->quick();
	}
}
