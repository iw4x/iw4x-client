namespace Components
{
	class Materials : public Component
	{
	public:
		Materials();
		~Materials();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Materials"; };
#endif

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
	};
}
