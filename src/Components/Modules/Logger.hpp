namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();
		const char* GetName() { return "Logger"; };


		static void MessagePrint(int channel, std::string message);
		static void Print(int channel, const char* message, ...);
		static void Print(const char* message, ...);
		static void ErrorPrint(int error, std::string message);
		static void Error(const char* message, ...);
		static void Error(int error, const char* message, ...);
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

		static std::string Format(const char** message);
	};
}
