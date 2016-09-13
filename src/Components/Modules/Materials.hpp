namespace Components
{
	class Materials : public Component
	{
	public:
		Materials();
		~Materials();

#ifdef DEBUG
		const char* GetName() { return "Materials"; };
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
	};
}
