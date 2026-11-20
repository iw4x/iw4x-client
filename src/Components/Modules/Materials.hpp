#pragma once

extern "C"
{
  void Materials_ImageVersionCheck();
  void Materials_DeathMessageStub();

  int Materials_WriteDeathMessageIcon(char* string, int offset, Game::Material* material);
}

namespace Components
{
  class Materials : public Component
  {
  public:
    Materials();
    ~Materials();

    static int FormatImagePath(char* buffer, size_t size, const char* format, const char* directory, const char* image);

    static Game::Material* Create(const std::string& name, Game::GfxImage* image);
    static void Delete(Game::Material* material, bool deleteImage = false);

    static Game::GfxImage* CreateImage(const std::string& name, unsigned int width, unsigned int height, unsigned int depth, unsigned int flags, _D3DFORMAT format);
    static void DeleteImage(Game::GfxImage* image);

    static bool IsValid(Game::Material* material);

  private:
    friend int (::Materials_WriteDeathMessageIcon)(char* string, int offset, Game::Material* material);

    static std::vector<Game::GfxImage*> ImageTable;
    static std::vector<Game::Material*> MaterialTable;

    static Utils::Hook ImageVersionCheckHook;

    static int WriteDeathMessageIcon(char* string, int offset, Game::Material* material);

#ifdef DEBUG
    static void DumpImageCfg(int, const char*, const char* material);
    static void DumpImageCfgPath(int, const char*, const char* material);
#endif

    static void DeleteAll();
  };
}
