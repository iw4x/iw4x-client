namespace Components
{
	class Renderer : public Component
	{
	public:
		typedef void(Callback)();

		Renderer();
		~Renderer();
		const char* GetName() { return "Renderer"; };

		static int Width();
		static int Height();

		static void Once(Callback* callback);
		static void OnFrame(Callback* callback);

	private:
		static void FrameHook();
		static void FrameHandler();

		static wink::signal<wink::slot<Callback>> FrameSignal;
		static wink::signal<wink::slot<Callback>> FrameOnceSignal;
		static Utils::Hook DrawFrameHook;
	};
}
