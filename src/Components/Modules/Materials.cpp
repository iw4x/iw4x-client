#include "..\..\STDInclude.hpp"

namespace Components
{
	Utils::Hook Materials::ImageVersionCheckHook;

	void __declspec(naked) Materials::ImageVersionCheck()
	{
		__asm
		{
			cmp eax, 9
			je returnSafely

			jmp Materials::ImageVersionCheckHook.Original

		returnSafely:
			mov al, 1
			add esp, 18h
			retn
		}
	}

	Materials::Materials()
	{
		// Allow codo images
		Materials::ImageVersionCheckHook.Initialize(0x53A456, Materials::ImageVersionCheck, HOOK_CALL)->Install();
	}

	Materials::~Materials()
	{
		Materials::ImageVersionCheckHook.Uninstall();
	}
}
