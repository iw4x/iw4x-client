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
		class VerifyContainer
		{
		public:
			bool isValid;
			Game::Material* material;
		};

		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();

		static Game::Material* VerifyMaterial(Game::Material* material);
		static void DrawMaterialStub();
	};
}
