#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
	std::vector<Network::Address> Logger::LoggingAddresses[2];
	void(*Logger::PipeCallback)(const std::string&) = nullptr;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(Console::GetWindow()) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
	}

	void Logger::PrintStub(int channel, const char* message, ...)
	{
		return Logger::MessagePrint(channel, Logger::Format(&message));
	}

	void Logger::Print(const char* message, ...)
	{
		return Logger::MessagePrint(0, Logger::Format(&message));
	}

	void Logger::Print(int channel, const char* message, ...)
	{
		return Logger::MessagePrint(channel, Logger::Format(&message));
	}

	void Logger::MessagePrint(int channel, const std::string& message)
	{
		if (Flags::HasFlag("stdout") || Loader::IsPerformingUnitTests())
		{
			printf("%s", message.data());
			fflush(stdout);
			return;
		}

		if (!Logger::IsConsoleReady())
		{
			OutputDebugStringA(message.data());
		}

		if (!Game::Sys_IsMainThread())
		{
			Logger::EnqueueMessage(message);
		}
		else
		{
			Game::Com_PrintMessage(channel, message.data(), 0);
		}
	}

	void Logger::ErrorPrint(Game::errorParm_t error, const std::string& message)
	{
#ifdef DEBUG
		if (IsDebuggerPresent()) __debugbreak();
#endif

		return Game::Com_Error(error, "%s", message.data());
	}

	void Logger::Error(Game::errorParm_t error, const char* message, ...)
	{
		return Logger::ErrorPrint(error, Logger::Format(&message));
	}

	void Logger::Error(const char* message, ...)
	{
		return Logger::ErrorPrint(Game::ERR_FATAL, Logger::Format(&message));
	}

	void Logger::SoftError(const char* message, ...)
	{
		return Logger::ErrorPrint(Game::ERR_SERVERDISCONNECT, Logger::Format(&message));
	}

	std::string Logger::Format(const char** message)
	{
		const size_t bufferSize = 0x10000;
		Utils::Memory::Allocator allocator;
		char* buffer = allocator.allocateArray<char>(bufferSize);

		va_list ap = reinterpret_cast<char*>(const_cast<char**>(&message[1]));
		//va_start(ap, *message);
		_vsnprintf_s(buffer, bufferSize, bufferSize, *message, ap);
		va_end(ap);

		return buffer;
	}

	void Logger::Flush()
	{
// 		if (!Game::Sys_IsMainThread())
// 		{
// 			while (!Logger::MessageQueue.empty())
// 			{
// 				std::this_thread::sleep_for(10ms);
// 			}
// 		}
// 		else
		{
			Logger::Frame();
		}
	}

	void Logger::Frame()
	{
		std::lock_guard<std::mutex> _(Logger::MessageMutex);

		for (unsigned int i = 0; i < Logger::MessageQueue.size(); ++i)
		{
			Game::Com_PrintMessage(0, Logger::MessageQueue[i].data(), 0);

			if (!Logger::IsConsoleReady())
			{
				OutputDebugStringA(Logger::MessageQueue[i].data());
			}
		}

		Logger::MessageQueue.clear();
	}

	void Logger::PipeOutput(void(*callback)(const std::string&))
	{
		Logger::PipeCallback = callback;
	}

	void Logger::PrintMessagePipe(const char* data)
	{
		if (Logger::PipeCallback)
		{
			Logger::PipeCallback(data);
		}
	}

	void Logger::NetworkLog(const char* data, bool gLog)
	{
		if (!data) return;

		std::string buffer(data);
		for (auto& addr : Logger::LoggingAddresses[gLog & 1])
		{
			Network::SendCommand(addr, "print", buffer);
		}
	}

	__declspec(naked) void Logger::GameLogStub()
	{
		__asm
		{
			pushad

			push 1
			push [esp + 28h]
			call Logger::NetworkLog
			add esp, 8h

			popad

			push 4576C0h
			retn
		}
	}

	__declspec(naked) void Logger::PrintMessageStub()
	{
		__asm
		{
			mov eax, Logger::PipeCallback
			test eax, eax
			jz returnPrint

			pushad
			push [esp + 28h]
			call Logger::PrintMessagePipe
			add esp, 4h
			popad
			retn

		returnPrint:
			pushad
			push 0
			push [esp + 2Ch]
			call Logger::NetworkLog
			add esp, 8h
			popad

			push esi
			mov esi, [esp + 0Ch]

			push 4AA835h
			retn
		}
	}

	void Logger::EnqueueMessage(const std::string& message)
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.push_back(message);
		Logger::MessageMutex.unlock();
	}

	void Logger::RedirectOSPath(const char* file, char* folder)
	{
		if (Dvar::Var("g_log").get<std::string>() == file)
		{
			if (folder != "userraw"s)
			{
				if (Dvar::Var("iw4x_onelog").get<bool>())
				{
					strcpy_s(folder, 256, "userraw");
				}
			}
		}
	}

	__declspec(naked) void Logger::BuildOSPathStub()
	{
		__asm
		{
			pushad

			push [esp + 28h]
			push [esp + 30h]

			call Logger::RedirectOSPath

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

	Logger::Logger()
	{
		Dvar::Register<bool>("iw4x_onelog", false, Game::dvar_flag::DVAR_FLAG_LATCHED | Game::dvar_flag::DVAR_FLAG_SAVED, "Only write the game log to the 'userraw' OS folder");
		Utils::Hook(0x642139, Logger::BuildOSPathStub, HOOK_JUMP).install()->quick();

		Logger::PipeOutput(nullptr);

		Scheduler::OnFrame(Logger::Frame);

		Utils::Hook(0x4B0218, Logger::GameLogStub, HOOK_CALL).install()->quick();
		Utils::Hook(Game::Com_PrintMessage, Logger::PrintMessageStub, HOOK_JUMP).install()->quick();

		if (Loader::IsPerformingUnitTests())
		{
			Utils::Hook(Game::Com_Printf, Logger::PrintStub, HOOK_JUMP).install()->quick();
		}

		Dvar::OnInit([]()
		{
			Command::AddSV("log_add", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				Network::Address addr(params->get(1));

				if (std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr) == Logger::LoggingAddresses[0].end())
				{
					Logger::LoggingAddresses[0].push_back(addr);
				}
			});

			Command::AddSV("log_del", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				int num = atoi(params->get(1));
				if (Utils::String::VA("%i", num) == std::string(params->get(1)) && static_cast<unsigned int>(num) < Logger::LoggingAddresses[0].size())
				{
					auto addr = Logger::LoggingAddresses[0].begin() + num;
					Logger::Print("Address %s removed\n", addr->getCString());
					Logger::LoggingAddresses[0].erase(addr);
				}
				else
				{
					Network::Address addr(params->get(1));

					auto i = std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr);
					if (i != Logger::LoggingAddresses[0].end())
					{
						Logger::LoggingAddresses[0].erase(i);
						Logger::Print("Address %s removed\n", addr.getCString());
					}
					else
					{
						Logger::Print("Address %s not found!\n", addr.getCString());
					}
				}
			});

			Command::AddSV("log_list", [](Command::Params*)
			{
				Logger::Print("# ID: Address\n");
				Logger::Print("-------------\n");

				for (unsigned int i = 0; i < Logger::LoggingAddresses[0].size(); ++i)
				{
					Logger::Print("#%03d: %5s\n", i, Logger::LoggingAddresses[0][i].getCString());
				}
			});

			Command::AddSV("g_log_add", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				Network::Address addr(params->get(1));

				if (std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr) == Logger::LoggingAddresses[1].end())
				{
					Logger::LoggingAddresses[1].push_back(addr);
				}
			});

			Command::AddSV("g_log_del", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				int num = atoi(params->get(1));
				if (Utils::String::VA("%i", num) == std::string(params->get(1)) && static_cast<unsigned int>(num) < Logger::LoggingAddresses[1].size())
				{
					auto addr = Logger::LoggingAddresses[1].begin() + num;
					Logger::Print("Address %s removed\n", addr->getCString());
					Logger::LoggingAddresses[1].erase(addr);
				}
				else
				{
					Network::Address addr(params->get(1));

					auto i = std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr);
					if (i != Logger::LoggingAddresses[1].end())
					{
						Logger::LoggingAddresses[1].erase(i);
						Logger::Print("Address %s removed\n", addr.getCString());
					}
					else
					{
						Logger::Print("Address %s not found!\n", addr.getCString());
					}
				}
			});

			Command::AddSV("g_log_list", [](Command::Params*)
			{
				Logger::Print("# ID: Address\n");
				Logger::Print("-------------\n");

				for (unsigned int i = 0; i < Logger::LoggingAddresses[1].size(); ++i)
				{
					Logger::Print("#%03d: %5s\n", i, Logger::LoggingAddresses[1][i].getCString());
				}
			});
		});
	}

	Logger::~Logger()
	{
		Logger::LoggingAddresses[0].clear();
		Logger::LoggingAddresses[1].clear();

		Logger::MessageMutex.lock();
		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();

		// Flush the console log
		if (int fh = *reinterpret_cast<int*>(0x1AD8F28))
		{
			Game::FS_FCloseFile(fh);
		}
	}
}
