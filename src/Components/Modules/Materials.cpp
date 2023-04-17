#include <STDInclude.hpp>
#include "Materials.hpp"

namespace Components
{
	Utils::Hook Materials::ImageVersionCheckHook;

	std::vector<Game::GfxImage*> Materials::ImageTable;
	std::vector<Game::Material*> Materials::MaterialTable;

	Game::Material* Materials::Create(const std::string& name, Game::GfxImage* image)
	{
		Game::Material* material = Utils::Memory::GetAllocator()->allocate<Game::Material>();
		Game::MaterialTextureDef* texture = Utils::Memory::GetAllocator()->allocate<Game::MaterialTextureDef>();

		material->textureCount = 1;
		material->textureTable = texture;

		material->info.name = Utils::Memory::GetAllocator()->duplicateString(name);
		material->info.sortKey = 0x22;
		material->info.textureAtlasColumnCount = 1;
		material->info.textureAtlasRowCount = 1;

		for (int i = 0; i < 48; ++i)
		{
			if (i != 4) material->stateBitsEntry[i] = -1;
		}

		material->stateFlags = 3;
		material->cameraRegion = 4;
		material->techniqueSet = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_TECHNIQUE_SET, "2d").techniqueSet;

		material->textureTable->nameHash = Game::R_HashString("colorMap");
		material->textureTable->nameStart = 'c';
		material->textureTable->nameEnd = 'p';
		material->textureTable->samplerState = -30;
		material->textureTable->u.image = image;

		Game::Material* cursor = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "ui_cursor").material;
		if (cursor)
		{
			material->stateBitsTable = cursor->stateBitsTable;
			material->stateBitsCount = cursor->stateBitsCount;
		}

		Materials::MaterialTable.push_back(material);

		return material;
	}

	void Materials::Delete(Game::Material* material, bool deleteImage)
	{
		if (!material) return;

		if (deleteImage)
		{
			for (char i = 0; i < material->textureCount; ++i)
			{
				Materials::DeleteImage(material->textureTable[i].u.image);
			}
		}

		Utils::Memory::GetAllocator()->free(material->textureTable);
		Utils::Memory::GetAllocator()->free(material->info.name);
		Utils::Memory::GetAllocator()->free(material);

		auto mat = std::find(Materials::MaterialTable.begin(), Materials::MaterialTable.end(), material);
		if (mat != Materials::MaterialTable.end())
		{
			Materials::MaterialTable.erase(mat);
		}
	}

	Game::GfxImage* Materials::CreateImage(const std::string& name, unsigned int width, unsigned int height, unsigned int depth, unsigned int flags, _D3DFORMAT format)
	{
		Game::GfxImage* image = Utils::Memory::GetAllocator()->allocate<Game::GfxImage>();
		image->name = Utils::Memory::GetAllocator()->duplicateString(name);

		Game::Image_Setup(image, width, height, depth, flags, format);

		Materials::ImageTable.push_back(image);

		return image;
	}

	void Materials::DeleteImage(Game::GfxImage* image)
	{
		if (!image) return;

		Game::Image_Release(image);

		Utils::Memory::GetAllocator()->free(image->name);
		Utils::Memory::GetAllocator()->free(image);

		auto img = std::find(Materials::ImageTable.begin(), Materials::ImageTable.end(), image);
		if (img != Materials::ImageTable.end())
		{
			Materials::ImageTable.erase(img);
		}
	}

	void Materials::DeleteAll()
	{
		std::vector<Game::Material*> materials;
		Utils::Merge(&materials, Materials::MaterialTable);
		Materials::MaterialTable.clear();

		for (auto& material : materials)
		{
			Materials::Delete(material);
		}

		std::vector<Game::GfxImage*> images;
		Utils::Merge(&images, Materials::ImageTable);
		Materials::ImageTable.clear();

		for (auto& image : images)
		{
			Materials::DeleteImage(image);
		}
	}

	bool Materials::IsValid(Game::Material* material)
	{
		if (!material || !material->textureCount || !material->textureTable) return false;

		for (char i = 0; i < material->textureCount; ++i)
		{
			if (!material->textureTable[i].u.image || !material->textureTable[i].u.image->texture.map)
			{
				return false;
			}
		}

		return true;
	}

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

	int Materials::WriteDeathMessageIcon(char* string, int offset, Game::Material* material)
	{
		if (!material)
		{
			material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;
		}

		int length = strlen(material->info.name);
		string[offset++] = static_cast<char>(length);

		strncpy_s(string + offset, 1024 - offset, material->info.name, length);

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

			mov [esp + 20h], eax
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

	Materials::Materials()
	{
		// Allow codo images
		Materials::ImageVersionCheckHook.initialize(0x53A456, Materials::ImageVersionCheck, HOOK_CALL)->install();

		// Adapt death message to IW5 material format
		Utils::Hook(0x5A30D9, Materials::DeathMessageStub, HOOK_JUMP).install()->quick();

		// Resolve preview images to loadscreens
		Utils::Hook(0x53AC19, Materials::FormatImagePath, HOOK_CALL).install()->quick();

		// Debug material comparison
		Utils::Hook::Set<void*>(0x523894, Materials::MaterialComparePrint);

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

		Renderer::OnDeviceRecoveryBegin([]()
		{
			for (auto& image : Materials::ImageTable)
			{
				Game::Image_Release(image);
				image->texture.map = nullptr;
			}
		});

		/*Renderer::OnDeviceRecoveryEnd([]()
		{
			for (auto& image : Materials::ImageTable)
			{
				Utils::Hook::Call<void(void*)>(0x51F7B0)(image);
			}
		});*/
	}

	Materials::~Materials()
	{
		Materials::DeleteAll();

		Materials::ImageVersionCheckHook.uninstall();
	}
}
