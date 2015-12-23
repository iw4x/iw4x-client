namespace Components
{
	class Materials : public Component
	{
	public:
		Materials();
		~Materials();
		const char* GetName() { return "Materials"; };

		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();
	};
}
