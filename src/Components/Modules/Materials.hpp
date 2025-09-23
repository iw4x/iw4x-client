#pragma once

namespace Components
{
	class Materials
	{
	public:
		Materials();
		~Materials();

		static int FormatImagePath(char* buffer, size_t size, int, int, const char* image);

		static Game::Material* Create(const std::string& name, Game::GfxImage* image);
		static void Delete(Game::Material* material, bool deleteImage = false);

		static Game::GfxImage* CreateImage(const std::string& name, unsigned int width, unsigned int height, unsigned int depth, unsigned int flags, _D3DFORMAT format);
		static void DeleteImage(Game::GfxImage* image);

		static bool IsValid(Game::Material* material);

	private:
		static std::vector<Game::GfxImage*> ImageTable;
		static std::vector<Game::Material*> MaterialTable;

		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();

		static int WriteDeathMessageIcon(char* string, int offset, Game::Material* material);
		static void DeathMessageStub();

#ifdef DEBUG
		static void DumpImageCfg(int, const char*, const char* material);
		static void DumpImageCfgPath(int, const char*, const char* material);
#endif

		static int MaterialComparePrint(Game::Material* m1, Game::Material* m2);

		static void DeleteAll();
	};
}
