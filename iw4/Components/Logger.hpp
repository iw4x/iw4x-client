namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		const char* GetName() { return "Logger"; };

		static void Print(const char* message, ...);
		static void Error(const char* message, ...);
		static void SoftError(const char* message, ...);
		static bool IsConsoleReady();
	};
}
