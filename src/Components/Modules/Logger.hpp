#pragma once

namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();
		
		static bool IsConsoleReady();

		static void PrintStub(int channel, const char* message, ...);

		static void PipeOutput(void(*callback)(const std::string&));

		static void Flush();

		static void MessagePrint(int channel, const std::string& msg);
		static void PrintInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void ErrorInternal(Game::errorParm_t error, std::string_view fmt, std::format_args&& args);
		static void PrintErrorInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void WarningInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void DebugInternal(bool verbose, const std::source_location& srcLoc, std::string_view fmt, std::format_args&& args);

		__forceinline static void Print(std::string_view fmt)
		{
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(0));
		}

		__forceinline static void Print(int channel, std::string_view fmt)
		{
			PrintInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void Print(std::string_view fmt, Args&&... args)
		{
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(args...));
		}

		template <typename... Args>
		__forceinline static void Print(int channel, std::string_view fmt, Args&&... args)
		{
			PrintInternal(channel, fmt, std::make_format_args(args...));
		}

		__forceinline static void Error(Game::errorParm_t error, std::string_view fmt)
		{
			ErrorInternal(error, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void Error(Game::errorParm_t error, std::string_view fmt, Args&&... args)
		{
			ErrorInternal(error, fmt, std::make_format_args(args...));
		}

		__forceinline static void Warning(int channel, std::string_view fmt)
		{
			WarningInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void Warning(int channel, std::string_view fmt, Args&&... args)
		{
			WarningInternal(channel, fmt, std::make_format_args(args...));
		}

		__forceinline static void PrintError(int channel, std::string_view fmt)
		{
			PrintErrorInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void PrintError(int channel, std::string_view fmt, Args&&... args)
		{
			PrintErrorInternal(channel, fmt, std::make_format_args(args...));
		}

#ifdef _DEBUG
		__forceinline static void DebugInfo([[maybe_unused]] std::string_view fmt)
		{
			DebugInternal(true, std::source_location::current(), fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void DebugInfo([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			DebugInternal(true, std::source_location::current(), fmt, std::make_format_args(args...));
		}

		__forceinline static void Debug([[maybe_unused]] std::string_view fmt)
		{
			DebugInternal(false, std::source_location::current(), fmt, std::make_format_args(0));
		}

		template <typename... Args>
		__forceinline static void Debug([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			DebugInternal(false, std::source_location::current(), fmt, std::make_format_args(args...));
		}
#else
		__forceinline static void DebugInfo([[maybe_unused]] std::string_view fmt)
		{
		}

		template <typename... Args>
		__forceinline static void DebugInfo([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
		}

		__forceinline static void Debug([[maybe_unused]] std::string_view fmt)
		{
		}

		template <typename... Args>
		__forceinline static void Debug([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
		}
#endif

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

		static void AddServerCommands();
	};
}
