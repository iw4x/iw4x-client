#pragma once

namespace Components
{
	class Materials : public Component
	{
	public:
		Materials();
		~Materials();

		static int FormatImagePath(char* buffer, size_t size, int, int, const char* image);

	private:
		static int ImageNameLength;

		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();

		static Game::Material* ResolveMaterial(const char* stringPtr);
		static void DrawMaterialStub();
		static void PostDrawMaterialStub();

		static int WriteDeathMessageIcon(char* string, int offset, Game::Material* material);
		static void DeathMessageStub();

#ifdef DEBUG
		static void DumpImageCfg(int, const char*, const char* material);
		static void DumpImageCfgPath(int, const char*, const char* material);
#endif

		static int MaterialComparePrint(Game::Material* m1, Game::Material* m2);
	};
}
