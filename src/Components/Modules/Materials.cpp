#include "Materials.hpp"
#include "Renderer.hpp"

namespace Components
{
  Utils::Hook Materials::ImageVersionCheckHook;

  std::vector<Game::GfxImage*> Materials::ImageTable;
  std::vector<Game::Material*> Materials::MaterialTable;

  Game::Material* Materials::Create(const std::string& name, Game::GfxImage* image)
  {
    AssertSize(Game::Material, 0x78);
    AssertSize(Game::MaterialTextureDef, 0x10);

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
      for (int i = 0; i < material->textureCount; ++i)
      {
        Materials::DeleteImage(material->textureTable[i].u.image);
      }
    }

    Utils::Memory::GetAllocator()->free(material->textureTable);
    Utils::Memory::GetAllocator()->free(material->info.name);
    Utils::Memory::GetAllocator()->free(material);

    auto mat = std::ranges::find(Materials::MaterialTable, material);
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

    auto img = std::ranges::find(Materials::ImageTable, image);
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

    for (int i = 0; i < material->textureCount; ++i)
    {
      if (!material->textureTable[i].u.image || !material->textureTable[i].u.image->texture.map)
      {
        return false;
      }
    }

    return true;
  }

  // Write the icon into the death message as the length of the material's
  // name followed by the name, the way IW5 does.
  //
  // Note that on x64 the game writes the 8-byte material pointer here, which
  // we replace the same way as the 4-byte one on x86.
  //
  int Materials::WriteDeathMessageIcon(char* string, int offset, Game::Material* material)
  {
    if (!material)
    {
      material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;
    }

    const int length (static_cast<int>(std::strlen(material->info.name)));
    string[offset++] = static_cast<char>(length);

    strncpy_s(string + offset, 1024 - offset, material->info.name, length);

    return offset + length;
  }

  int Materials::FormatImagePath(char* buffer, size_t size, [[maybe_unused]] const char* format, [[maybe_unused]] const char* directory, const char* image)
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
    //
    // Note that on x64 the game checks the version of the image file header
    // inline and fails without printing anything. So we jump to the stub in
    // place of the check.
    //
    Materials::ImageVersionCheckHook.initialize(0x14006A1F8, Materials_ImageVersionCheck, HOOK_JUMP)->install();

    // x64 also fails with "Image file corrupt." if the size of the image file
    // isn't the one its header gives for the full resolution, which x86
    // doesn't check at all. Some of our images (angry_joy, for one) give the
    // size without the header, so we skip that check and read what the
    // header says, as x86 does. Arguably those images are the ones at fault,
    // but fixing them all seems a lot more work than one byte.
    //
    Utils::Hook::Set<BYTE>(0x14006A2B3, 0xEB); // jz -> jmp

    // Adapt death message to IW5 material format
    //
    // On x86 we jump from the function that writes the icon, but x64 inlines
    // it into the one that prints the death message. So we replace the
    // instructions that write the material there instead.
    //
    Utils::Hook(0x1400EBF20, Materials_DeathMessageStub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x1400EBF25, 6);

    // Resolve preview images to loadscreens
    Utils::Hook(0x14006A1A6, Materials::FormatImagePath, HOOK_CALL).install()->quick();

    // @@ On x86 we also replace the comparison function of the material sort
    //    with one of our own that calls it, just to have a place to break at
    //    while debugging. We haven't found the sort on x64, since the game
    //    compiles out the code we would find it by. Probably not a big loss.
    //

#ifdef DEBUG
    // Note that x64 compiles out the error for a missing image file, which
    // we also dump on x86. The caller then reports that it couldn't load the
    // image, so we dump that report instead, with the same path.
    //
    if (Flags::HasFlag("dump"))
    {
      Utils::Hook(0x140037BC9, Materials::DumpImageCfg, HOOK_CALL).install()->quick();
      Utils::Hook(0x140037AC3, Materials::DumpImageCfg, HOOK_CALL).install()->quick();
    }
    else
    {
      // Ignore missing images
      Utils::Hook::Nop(0x140037BC9, 5);
      Utils::Hook::Nop(0x140037AC3, 5);
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

int Materials_WriteDeathMessageIcon(char* string, int offset, Game::Material* material)
{
  return Components::Materials::WriteDeathMessageIcon(string, offset, material);
}
