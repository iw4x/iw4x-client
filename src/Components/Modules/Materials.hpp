namespace Components
{
	class Materials : public Component
	{
	public:
		Materials();
		~Materials();
		const char* GetName() { return "Materials"; };

	private:
		static Utils::Hook ImageVersionCheckHook;
		static void ImageVersionCheck();
	};
}
