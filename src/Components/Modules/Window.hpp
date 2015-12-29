namespace Components
{
	class Window : public Component
	{
	public:
		Window();
		const char* GetName() { return "Window"; };

		static Dvar::Var NoBorder;
		static void Window::StyleHookStub();
	};
}
