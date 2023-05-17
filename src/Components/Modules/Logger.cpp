#include <STDInclude.hpp>

#include "Console.hpp"
#include "Events.hpp"

namespace Components
{
	using namespace Utils::String;

	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;

	std::recursive_mutex Logger::LoggingMutex;
	std::vector<Network::Address> Logger::LoggingAddresses[2];

	Dvar::Var Logger::IW4x_oneLog;

	void(*Logger::PipeCallback)(const std::string&) = nullptr;;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(Console::GetWindow()) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
	}

	void Logger::Print_Stub(const int channel, const char* message, ...)
	{
		char buf[4096]{};

		va_list va;
		va_start(va, message);
		vsnprintf_s(buf, _TRUNCATE, message, va);
		va_end(va);

		MessagePrint(channel, std::string{ buf });
	}

	void Logger::MessagePrint(const int channel, const std::string& msg)
	{
		static const auto shouldPrint = []() -> bool
		{
			return Flags::HasFlag("stdout") || Loader::IsPerformingUnitTests();
		}();

		if (shouldPrint)
		{
			std::printf("%s", msg.data());
			std::fflush(stdout);
			return;
		}

#ifdef _DEBUG
		if (!IsConsoleReady())
		{
			OutputDebugStringA(msg.data());
		}
#endif

		if (!Game::Sys_IsMainThread())
		{
			EnqueueMessage(msg);
		}
		else
		{
			Game::Com_PrintMessage(channel, msg.data(), 0);
		}
	}

	void Logger::DebugInternal(const std::string_view& fmt, std::format_args&& args, [[maybe_unused]] const std::source_location& loc)
	{
#ifdef LOGGER_TRACE
		const auto msg = std::vformat(fmt, args);
		const auto out = std::format("Debug:\n    {}\nFile:    {}\nLine:    {}\n", msg, loc.file_name(), loc.line());
#else
		const auto msg = std::vformat(fmt, args);
		const auto out = std::format("^2{}\n", msg);
#endif

		MessagePrint(Game::CON_CHANNEL_DONT_FILTER, out);
	}

	void Logger::PrintInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
	{
		const auto msg = std::vformat(fmt, args);

		MessagePrint(channel, msg);
	}

	void Logger::ErrorInternal(const Game::errorParm_t error, const std::string_view& fmt, std::format_args&& args)
	{
		const auto msg = std::vformat(fmt, args);

#ifdef _DEBUG
		if (IsDebuggerPresent()) __debugbreak();
#endif

		Game::Com_Error(error, "%s", msg.data());
	}

	void Logger::PrintErrorInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
	{
		const auto msg = "^1Error: " + std::vformat(fmt, args);

		++(*Game::com_errorPrintsCount);
		MessagePrint(channel, msg);

		if (Game::cls->uiStarted && (*Game::com_fixedConsolePosition == 0))
		{
			Game::CL_ConsoleFixPosition();
		}
	}

	void Logger::WarningInternal(Game::conChannel_t channel, const std::string_view& fmt, std::format_args&& args)
	{
		const auto msg = "^3" + std::vformat(fmt, args);

		MessagePrint(channel, msg);
	}

	void Logger::Frame()
	{
		std::unique_lock _(MessageMutex);

		for (auto i = MessageQueue.begin(); i != MessageQueue.end();)
		{
			Game::Com_PrintMessage(Game::CON_CHANNEL_DONT_FILTER, i->data(), 0);

#ifdef _DEBUG
			if (!IsConsoleReady())
			{
				OutputDebugStringA(i->data());
			}
#endif

			i = MessageQueue.erase(i);
		}
	}

	void Logger::PipeOutput(void(*callback)(const std::string&))
	{
		PipeCallback = callback;
	}

	void Logger::PrintMessagePipe(const char* data)
	{
		if (PipeCallback)
		{
			PipeCallback(data);
		}
	}

	void Logger::NetworkLog(const char* data, bool gLog)
	{
		if (!data)
		{
			return;
		}

		std::unique_lock lock(LoggingMutex);
		for (const auto& addr : LoggingAddresses[gLog & 1])
		{
			Network::SendCommand(addr, "print", data);
		}
	}

	void Logger::G_LogPrintf_Hk(const char* fmt, ...)
	{
		char string[1024]{};
		char string2[1024]{};

		va_list ap;
		va_start(ap, fmt);
		vsnprintf_s(string2, _TRUNCATE, fmt, ap);
		va_end(ap);

		const auto time = Game::level->time / 1000;
		const auto len = sprintf_s(string, "%3i:%i%i %s", time / 60, time % 60 / 10, time % 60 % 10, string2);

		if (Game::level->logFile)
		{
			Game::FS_Write(string, len, Game::level->logFile);
		}

		// Allow the network log to run even if logFile was not opened
		NetworkLog(string, true);
	}

	__declspec(naked) void Logger::PrintMessage_Stub()
	{
		__asm
		{
			mov eax, PipeCallback
			test eax, eax
			jz returnPrint

			pushad

			push [esp + 28h]
			call PrintMessagePipe
			add esp, 4h

			popad
			ret

		returnPrint:
			pushad

			push 0 // gLog
			push [esp + 2Ch] // data
			call NetworkLog
			add esp, 8h

			popad

			push esi
			mov esi, [esp + 0Ch]

			push 4AA835h
			ret
		}
	}

	void Logger::EnqueueMessage(const std::string& message)
	{
		std::unique_lock _(MessageMutex);
		MessageQueue.push_back(message);
	}

	void Logger::RedirectOSPath(const char* file, char* folder)
	{
		const auto* g_log = (*Game::g_log) ? (*Game::g_log)->current.string : "";

		if (std::strcmp(g_log, file) == 0)
		{
			if (std::strcmp(folder, "userraw") != 0)
			{
				if (IW4x_oneLog.get<bool>())
				{
					strncpy_s(folder, 256, "userraw", _TRUNCATE);
				}
			}
		}
	}

	__declspec(naked) void Logger::BuildOSPath_Stub()
	{
		__asm
		{
			pushad

			push [esp + 20h + 8h]
			push [esp + 20h + 10h]
			call RedirectOSPath
			add esp, 8h

			popad

			mov eax, [esp + 8h]
			push ebp
			push esi
			mov esi, [esp + 0Ch]

			push 64213Fh
			retn
		}
	}

	void Logger::LSP_LogString_Stub([[maybe_unused]] int localControllerIndex, const char* string)
	{
		NetworkLog(string, false);
	}

	void Logger::LSP_LogStringAboutUser_Stub([[maybe_unused]] int localControllerIndex, std::uint64_t xuid, const char* string)
	{
		NetworkLog(VA("%" PRIx64 ";%s", xuid, string), false);
	}

	void Logger::AddServerCommands()
	{
		Command::AddSV("log_add", [](const Command::Params* params)
		{
			if (params->size() < 2) return;

			std::unique_lock lock(LoggingMutex);

			Network::Address addr(params->get(1));
			if (std::ranges::find(LoggingAddresses[0], addr) == LoggingAddresses[0].end())
			{
				LoggingAddresses[0].push_back(addr);
			}
		});

		Command::AddSV("log_del", [](const Command::Params* params)
		{
			if (params->size() < 2) return;

			std::unique_lock lock(LoggingMutex);

			const auto num = std::atoi(params->get(1));
			if (!std::strcmp(VA("%i", num), params->get(1)) && static_cast<unsigned int>(num) < LoggingAddresses[0].size())
			{
				auto addr = Logger::LoggingAddresses[0].begin() + num;
				Print("Address {} removed\n", addr->getString());
				LoggingAddresses[0].erase(addr);
			}
			else
			{
				Network::Address addr(params->get(1));

				if (const auto i = std::ranges::find(LoggingAddresses[0], addr); i != LoggingAddresses[0].end())
				{
					LoggingAddresses[0].erase(i);
					Print("Address {} removed\n", addr.getString());
				}
				else
				{
					Print("Address {} not found!\n", addr.getString());
				}
			}
		});

		Command::AddSV("log_list", []([[maybe_unused]] const Command::Params* params)
		{
			Print("# ID: Address\n");
			Print("-------------\n");

			std::unique_lock lock(LoggingMutex);

			for (unsigned int i = 0; i < LoggingAddresses[0].size(); ++i)
			{
				Print("#{:03d}: {}\n", i, LoggingAddresses[0][i].getString());
			}
		});

		Command::AddSV("g_log_add", [](const Command::Params* params)
		{
			if (params->size() < 2) return;

			std::unique_lock lock(LoggingMutex);

			const Network::Address addr(params->get(1));
			if (std::ranges::find(LoggingAddresses[1], addr) == LoggingAddresses[1].end())
			{
				LoggingAddresses[1].push_back(addr);
			}
		});

		Command::AddSV("g_log_del", [](const Command::Params* params)
		{
			if (params->size() < 2) return;

			std::unique_lock lock(LoggingMutex);

			const auto num = std::atoi(params->get(1));
			if (!std::strcmp(VA("%i", num), params->get(1)) && static_cast<unsigned int>(num) < LoggingAddresses[1].size())
			{
				const auto addr = LoggingAddresses[1].begin() + num;
				Print("Address {} removed\n", addr->getString());
				LoggingAddresses[1].erase(addr);
			}
			else
			{
				const Network::Address addr(params->get(1));
				const auto i = std::ranges::find(LoggingAddresses[1].begin(), LoggingAddresses[1].end(), addr);
				if (i != LoggingAddresses[1].end())
				{
					LoggingAddresses[1].erase(i);
					Print("Address {} removed\n", addr.getString());
				}
				else
				{
					Print("Address {} not found!\n", addr.getString());
				}
			}
		});

		Command::AddSV("g_log_list", []([[maybe_unused]] const Command::Params* params)
		{
			Print("# ID: Address\n");
			Print("-------------\n");

			std::unique_lock lock(LoggingMutex);
			for (std::size_t i = 0; i < LoggingAddresses[1].size(); ++i)
			{
				Print("#{:03d}: {}\n", i, LoggingAddresses[1][i].getString());
			}
		});
	}

	Logger::Logger()
	{
		IW4x_oneLog = Dvar::Register<bool>("iw4x_onelog", false, Game::DVAR_LATCH, "Only write the game log to the 'userraw' OS folder");
		Utils::Hook(0x642139, BuildOSPath_Stub, HOOK_JUMP).install()->quick();

		Scheduler::Loop(Frame, Scheduler::Pipeline::SERVER);

		Utils::Hook(Game::G_LogPrintf, G_LogPrintf_Hk, HOOK_JUMP).install()->quick();
		Utils::Hook(Game::Com_PrintMessage, PrintMessage_Stub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x5F67AE, LSP_LogString_Stub, HOOK_CALL).install()->quick(); // Scr_LogString
		Utils::Hook(0x5F67EE, LSP_LogStringAboutUser_Stub, HOOK_CALL).install()->quick(); // ScrCmd_LogString_Stub

		if (Loader::IsPerformingUnitTests())
		{
			Utils::Hook(Game::Com_Printf, Print_Stub, HOOK_JUMP).install()->quick();
		}

		Events::OnSVInit(AddServerCommands);
	}

	Logger::~Logger()
	{
		std::unique_lock lock_logging(LoggingMutex);
		LoggingAddresses[0].clear();
		LoggingAddresses[1].clear();

		std::unique_lock lock_message(MessageMutex);
		MessageQueue.clear();

		// Flush the console log
		if (*Game::logfile)
		{
			Game::FS_FCloseFile(*Game::logfile);
		}
	}
}
