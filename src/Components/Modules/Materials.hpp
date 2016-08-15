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
		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();

		static Game::Material* VerifyMaterial(Game::Material* material);
		static void DrawMaterialStub();
	};
}
