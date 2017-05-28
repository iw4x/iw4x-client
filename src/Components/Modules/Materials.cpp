#include "STDInclude.hpp"

namespace Components
{
	std::mutex Materials::TechsetMutex;
	int Materials::ImageNameLength;
	Utils::Hook Materials::ImageVersionCheckHook;

	__declspec(naked) void Materials::ImageVersionCheck()
	{
		__asm
		{
			cmp eax, 9
			je returnSafely

			jmp Materials::ImageVersionCheckHook.original

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
			push eax
			pushad

			push ecx
			call Materials::ResolveMaterial
			add esp, 4h

			mov[esp + 20h], eax
			popad
			pop eax

			push 5310F0h
			retn
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
			push eax
			pushad

			push edx // Material
			push eax // offset
			push ecx // String

			call Materials::WriteDeathMessageIcon
			add esp, 0Ch

			mov[esp + 20h], eax
			popad
			pop eax

			add esp, 8h
			retn
		}
	}

	int Materials::FormatImagePath(char* buffer, size_t size, int, int, const char* image)
	{
#if 0
		if (Utils::String::StartsWith(image, "preview_"))
		{
			std::string newImage = image;
			Utils::String::Replace(newImage, "preview_", "loadscreen_");

			if (FileSystem::FileReader(fmt::sprintf("images/%s.iwi", newImage.data())).exists())
			{
				image = Utils::String::VA("%s", newImage.data());
			}
		}
#endif

		return _snprintf_s(buffer, size, size, "images/%s.iwi", image);
	}

	int Materials::MaterialComparePrint(Game::Material* m1, Game::Material* m2)
	{
		//__debugbreak();
		return Utils::Hook::Call<int(Game::Material*, Game::Material*)>(0x5235B0)(m1, m2);
	}

#ifdef DEBUG
	void Materials::DumpImageCfg(int, const char*, const char* material)
	{
		Materials::DumpImageCfgPath(0, nullptr, Utils::String::VA("images/%s.iwi", material));
	}

	void Materials::DumpImageCfgPath(int, const char*, const char* material)
	{
		FILE* fp = nullptr;
		if (!fopen_s(&fp, "dump.cfg", "a") && fp)
		{
			fprintf(fp, "dumpraw %s\n", material);
			fclose(fp);
		}
	}

#endif

	void Materials::OverrideTechsets()
	{
		std::lock_guard<std::mutex> _(Materials::TechsetMutex);
		Utils::Hook::Call<void()>(0x522D00)(); // Material_OverrideTechniqueSets
	}

	void Materials::MaterialSort()
	{
		std::lock_guard<std::mutex> _(Materials::TechsetMutex);

		if (!*reinterpret_cast<bool*>(0x69F9AFD))
		{
			Utils::Hook::Call<void()>(0x523A20)(); // Material_Sort
		}
	}

	Materials::Materials()
	{
		Materials::ImageNameLength = 7;

		// Allow codo images
		Materials::ImageVersionCheckHook.initialize(0x53A456, Materials::ImageVersionCheck, HOOK_CALL)->install();

		// Fix material pointer exploit
		Utils::Hook(0x534E0C, Materials::DrawMaterialStub, HOOK_CALL).install()->quick();

		// Increment string pointer accordingly
		Utils::Hook(0x5358FA, Materials::PostDrawMaterialStub, HOOK_JUMP).install()->quick();

		// Adapt death message to IW5 material format
		Utils::Hook(0x5A30D9, Materials::DeathMessageStub, HOOK_JUMP).install()->quick();

		// Resolve preview images to loadscreens
		Utils::Hook(0x53AC19, Materials::FormatImagePath, HOOK_CALL).install()->quick();

		// Debug material comparison
		Utils::Hook::Set<void*>(0x523894, Materials::MaterialComparePrint);

		// Synchronize material sorting
		Utils::Hook(0x50AAFE, Materials::MaterialSort, HOOK_CALL).install()->quick();

		// Don't compare remapped techsets
		Utils::Hook::Set<BYTE>(0x523490, 0xEB);
		Utils::Hook::Set<BYTE>(0x5234A0, 0xEB);

		// Synchronize techset remapping
		Utils::Hook(0x50AAD8, Materials::OverrideTechsets, HOOK_CALL).install()->quick();
		Utils::Hook(0x518BB8, Materials::OverrideTechsets, HOOK_CALL).install()->quick();
		Utils::Hook(0x5BC791, Materials::OverrideTechsets, HOOK_CALL).install()->quick();

#ifdef DEBUG
		if (Flags::HasFlag("dump"))
		{
			Utils::Hook(0x51F5AC, Materials::DumpImageCfg, HOOK_CALL).install()->quick();
			Utils::Hook(0x51F4C4, Materials::DumpImageCfg, HOOK_CALL).install()->quick();
			Utils::Hook(0x53AC62, Materials::DumpImageCfgPath, HOOK_CALL).install()->quick();
		}
		else
		{
			// Ignore missing images
			Utils::Hook::Nop(0x51F5AC, 5);
			Utils::Hook::Nop(0x51F4C4, 5);
		}
#endif

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
		Materials::ImageVersionCheckHook.uninstall();
	}
}
