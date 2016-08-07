#include "STDInclude.hpp"

namespace Components
{
	Utils::Hook Materials::ImageVersionCheckHook;

	__declspec(naked) void Materials::ImageVersionCheck()
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

	Game::Material* Materials::VerifyMaterial(Game::Material* material)
	{
		if (!IsBadReadPtr(material, 4) && !IsBadReadPtr(material->name, 1))
		{
			return material;
		}

		return Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;
	}

	__declspec(naked) void Materials::DrawMaterialStub()
	{
		__asm
		{
			push eax
			call Materials::VerifyMaterial
			add esp, 4h

			mov edx, 5310F0h
			jmp edx
		}
	}

	Materials::Materials()
	{
		// Allow codo images
		Materials::ImageVersionCheckHook.Initialize(0x53A456, Materials::ImageVersionCheck, HOOK_CALL)->Install();

		// Fix material pointer exploit
		Utils::Hook(0x534E0C, Materials::DrawMaterialStub, HOOK_CALL).Install()->Quick();
	}

	Materials::~Materials()
	{
		Materials::ImageVersionCheckHook.Uninstall();
	}
}
