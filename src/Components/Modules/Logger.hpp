#pragma once

namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();
		
		static bool IsConsoleReady();

		static void Print_Stub(int channel, const char* message, ...);

		static void PipeOutput(const std::function<void(const std::string&)>& callback);

		static void PrintInternal(Game::conChannel_t channel, std::string_view fmt, std::format_args&& args);
		static void ErrorInternal(Game::errorParm_t error, std::string_view fmt, std::format_args&& args);
		static void PrintErrorInternal(Game::conChannel_t channel, std::string_view fmt, std::format_args&& args);
		static void WarningInternal(Game::conChannel_t channel, std::string_view fmt, std::format_args&& args);
		static void DebugInternal(std::string_view fmt, std::format_args&& args, const std::source_location& loc);

		static void Print(std::string_view fmt)
		{
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(0));
		}

		static void Print(Game::conChannel_t channel, std::string_view fmt)
		{
			PrintInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Print(std::string_view fmt, Args&&... args)
		{
			(Utils::String::SanitizeFormatArgs(args), ...);
			PrintInternal(Game::CON_CHANNEL_DONT_FILTER, fmt, std::make_format_args(args...));
		}

		template <typename... Args>
		static void Print(Game::conChannel_t channel, std::string_view fmt, Args&&... args)
		{
			(Utils::String::SanitizeFormatArgs(args), ...);
			PrintInternal(channel, fmt, std::make_format_args(args...));
		}

		static void Error(Game::errorParm_t error, std::string_view fmt)
		{
			ErrorInternal(error, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Error(Game::errorParm_t error, std::string_view fmt, Args&&... args)
		{
			(Utils::String::SanitizeFormatArgs(args), ...);
			ErrorInternal(error, fmt, std::make_format_args(args...));
		}

		static void Warning(Game::conChannel_t channel, std::string_view fmt)
		{
			WarningInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void Warning(Game::conChannel_t channel, std::string_view fmt, Args&&... args)
		{
			(Utils::String::SanitizeFormatArgs(args), ...);
			WarningInternal(channel, fmt, std::make_format_args(args...));
		}

		static void PrintError(Game::conChannel_t channel, std::string_view fmt)
		{
			PrintErrorInternal(channel, fmt, std::make_format_args(0));
		}

		template <typename... Args>
		static void PrintError(Game::conChannel_t channel, std::string_view fmt, Args&&... args)
		{
			(Utils::String::SanitizeFormatArgs(args), ...);
			PrintErrorInternal(channel, fmt, std::make_format_args(args...));
		}

		template <typename... Args>
		class Debug
		{
		public:
			Debug([[maybe_unused]] std::string_view fmt, [[maybe_unused]] const Args&... args, [[maybe_unused]] const std::source_location& loc = std::source_location::current())
			{
#ifdef _DEBUG
				(Utils::String::SanitizeFormatArgs(args), ...);
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

		static std::function<void(const std::string&)> PipeCallback;

		static void MessagePrint(int channel, const std::string& msg);

		static void Frame();

		static void G_LogPrintf_Hk(const char* fmt, ...);
		static void PrintMessage_Stub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(const std::string& message);

		static void BuildOSPath_Stub();
		static void RedirectOSPath(const char* file, char* folder);

		static void NetworkLog(const char* data, bool gLog);

		static void AddServerCommands();
	};
}
