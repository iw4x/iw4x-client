#include "STDInclude.hpp"

namespace Components
{
	int Materials::ImageNameLength;
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

	Game::Material* Materials::ResolveMaterial(const char* stringPtr)
	{
		const char* imagePtr = stringPtr + 4;
		unsigned int length = static_cast<unsigned int>(stringPtr[3] & 0xFF);

		if (strlen(imagePtr) >= length)
		{
			Materials::ImageNameLength = 4 + length;
			std::string image(imagePtr, length);
			
			return Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, image.data()).material;
		}

		Materials::ImageNameLength = 4;
		return Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;
	}

	__declspec(naked) void Materials::PostDrawMaterialStub()
	{
		__asm
		{
			mov eax, Materials::ImageNameLength
			add [esp + 30h], eax

			mov eax, 5358FFh
			jmp eax
		}
	}

	__declspec(naked) void Materials::DrawMaterialStub()
	{
		__asm
		{
			push ecx
			call Materials::ResolveMaterial
			add esp, 4h

			mov edx, 5310F0h
			jmp edx
		}
	}

	int Materials::WriteDeathMessageIcon(char* string, int offset, Game::Material* material)
	{
		if (!material) 
		{
			material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;
		}

		int length = strlen(material->name);
		string[offset++] = static_cast<char>(length);

		strncpy_s(string + offset, 1024 - offset, material->name, length);

		return offset + length;
	}

	__declspec(naked) void Materials::DeathMessageStub()
	{
		__asm
		{
			push edx // Material
			push eax // offset
			push ecx // String

			call Materials::WriteDeathMessageIcon

			add esp, 14h
			retn
		}
	}

	Materials::Materials()
	{
		Materials::ImageNameLength = 7;

		// Allow codo images
		Materials::ImageVersionCheckHook.Initialize(0x53A456, Materials::ImageVersionCheck, HOOK_CALL)->Install();

		// Fix material pointer exploit
		Utils::Hook(0x534E0C, Materials::DrawMaterialStub, HOOK_CALL).Install()->Quick();

		// Increment string pointer accordingly
		Utils::Hook(0x5358FA, Materials::PostDrawMaterialStub, HOOK_JUMP).Install()->Quick();

		// Adapt death message to IW5 material format
		Utils::Hook(0x5A30D9, Materials::DeathMessageStub, HOOK_JUMP).Install()->Quick();

// 		Renderer::OnFrame([] ()
// 		{
// 			Game::Font* font = Game::R_RegisterFont("fonts/normalFont");
// 			float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
// 
// 			Game::R_AddCmdDrawText("test^==preview_mp_rustzob", 0x7FFFFFFF, font, 500.0f, 150.0f, 1.0f, 1.0f, 0.0f, color, Game::ITEM_TEXTSTYLE_SHADOWED);
// 		});
	}

	Materials::~Materials()
	{
		Materials::ImageVersionCheckHook.Uninstall();
	}
}
