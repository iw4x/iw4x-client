#pragma once

namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();

		static void MessagePrint(int channel, const std::string& message);
		static void Print(int channel, const char* message, ...);
		static void Print(const char* message, ...);
		static void ErrorPrint(Game::errorParm_t error, const std::string& message);
		static void Error(const char* message, ...);
		static void Error(Game::errorParm_t error, const char* message, ...);
		static void SoftError(const char* message, ...);
		static bool IsConsoleReady();

		static void PrintStub(int channel, const char* message, ...);

		static void PipeOutput(void(*callback)(const std::string&));

		static void Flush();

	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;
		static std::vector<Network::Address> LoggingAddresses[2];
		static void(*PipeCallback)(const std::string&);

		static void Frame();
		static void GameLogStub();
		static void PrintMessageStub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(const std::string& message);

		static void BuildOSPathStub();
		static void RedirectOSPath(const char* file, char* folder);

		static void NetworkLog(const char* data, bool gLog);

		static std::string Format(const char** message);
	};
}
