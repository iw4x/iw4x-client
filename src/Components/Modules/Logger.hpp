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

		static void PipeOutput(void(*callback)(std::string));

	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;
		static void(*PipeCallback)(std::string);

		static void Frame();
		static void PrintMessageStub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(std::string message);	
	};
}
