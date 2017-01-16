namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Logger"; };
#endif

		static void MessagePrint(int channel, std::string message);
		static void Print(int channel, const char* message, ...);
		static void Print(const char* message, ...);
		static void ErrorPrint(int error, std::string message);
		static void Error(const char* message, ...);
		static void Error(int error, const char* message, ...);
		static void SoftError(const char* message, ...);
		static bool IsConsoleReady();

		static void PrintStub(int channel, const char* message, ...);

		static void PipeOutput(void(*callback)(std::string));

		static void Flush();

	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;
		static std::vector<Network::Address> LoggingAddresses[2];
		static void(*PipeCallback)(std::string);

		static void Frame();
		static void GameLogStub();
		static void PrintMessageStub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(std::string message);	

		static void NetworkLog(const char* data, bool gLog);

		static std::string Format(const char** message);
	};
}
