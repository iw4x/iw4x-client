namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();
		const char* GetName() { return "Logger"; };

		static void Print(const char* message, ...);
		static void Error(const char* message, ...);
		static void SoftError(const char* message, ...);
		static bool IsConsoleReady();

	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;

		static void Frame();
		static void EnqueueMessage(std::string message);	
	};
}
