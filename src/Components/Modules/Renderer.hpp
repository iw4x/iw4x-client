namespace Components
{
	class Renderer : public Component
	{
	public:
		typedef void(*Callback)();

		Renderer();
		~Renderer();
		const char* GetName() { return "Renderer"; };

		static int Width();
		static int Height();

		static void OnFrame(Callback callback);

	private:
		static void FrameHook();
		static void FrameHandler();

		static std::vector<Callback> FrameCallbacks;
		static Utils::Hook DrawFrameHook;
	};
}
