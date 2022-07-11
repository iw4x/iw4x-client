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

		static void PrintInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void ErrorInternal(Game::errorParm_t error, std::string_view fmt, std::format_args&& args);
		static void PrintErrorInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void WarningInternal(int channel, std::string_view fmt, std::format_args&& args);
		static void DebugInternal(std::string_view fmt, std::format_args&& args, const std::source_location& loc);

		static void Print(std::string_view fmt)
		{
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(0));
		}

		static void Print(int channel, std::string_view fmt)
		{
			PrintInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Print(std::string_view fmt, Args&&... args)
		{
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(args...));
		}

		template <typename... Args>
		static void Print(int channel, std::string_view fmt, Args&&... args)
		{
			PrintInternal(channel, fmt, std::make_format_args(args...));
		}

		static void Error(Game::errorParm_t error, std::string_view fmt)
		{
			ErrorInternal(error, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Error(Game::errorParm_t error, std::string_view fmt, Args&&... args)
		{
			ErrorInternal(error, fmt, std::make_format_args(args...));
		}

		static void Warning(int channel, std::string_view fmt)
		{
			WarningInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Warning(int channel, std::string_view fmt, Args&&... args)
		{
			WarningInternal(channel, fmt, std::make_format_args(args...));
		}

		static void PrintError(int channel, std::string_view fmt)
		{
			PrintErrorInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void PrintError(int channel, std::string_view fmt, Args&&... args)
		{
			PrintErrorInternal(channel, fmt, std::make_format_args(args...));
		}

		template <typename... Args>
		class Debug
		{
		public:
			Debug([[maybe_unused]] std::string_view fmt, [[maybe_unused]] const Args&... args, [[maybe_unused]] const std::source_location& loc = std::source_location::current())
			{
#ifdef _DEBUG
				DebugInternal(fmt, std::make_format_args(args...), loc);
#endif
			}
		};

		template <typename... Args>
		Debug(std::string_view fmt, const Args&... args) -> Debug<Args...>;

	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;
		static std::vector<Network::Address> LoggingAddresses[2];
		static void(*PipeCallback)(const std::string&);

		static void MessagePrint(int channel, const std::string& msg);
		static void Frame();
		static void G_LogPrintfStub(const char* fmt, ...);
		static void PrintMessageStub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(const std::string& message);

		static void BuildOSPathStub();
		static void RedirectOSPath(const char* file, char* folder);

		static void NetworkLog(const char* data, bool gLog);

		static void AddServerCommands();
	};
}
