namespace Components
{
	class Console : public Component
	{
	public:
		Console();
		const char* GetName() { return "Console"; };

	private:
		static void ToggleConsole();
	};
}
