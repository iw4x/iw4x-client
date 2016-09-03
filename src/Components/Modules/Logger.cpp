#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
	std::vector<Network::Address> Logger::LoggingAddresses[2];
	void(*Logger::PipeCallback)(std::string) = nullptr;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(*reinterpret_cast<HWND*>(0x64A3288)) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
	}

	void Logger::Print(const char* message, ...)
	{
		return Logger::MessagePrint(0, Logger::Format(&message));
	}

	void Logger::Print(int channel, const char* message, ...)
	{
		return Logger::MessagePrint(channel, Logger::Format(&message));
	}

	void Logger::MessagePrint(int channel, std::string message)
	{
		if (Flags::HasFlag("stdout") || Loader::PerformingUnitTests())
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

	void Logger::ErrorPrint(int error, std::string message)
	{
		return Game::Com_Error(error, "%s", message.data());
	}

	void Logger::Error(int error, const char* message, ...)
	{
		return Logger::ErrorPrint(error, Logger::Format(&message));
	}

	void Logger::Error(const char* message, ...)
	{
		return Logger::ErrorPrint(0, Logger::Format(&message));
	}

	void Logger::SoftError(const char* message, ...)
	{
		return Logger::ErrorPrint(2, Logger::Format(&message));
	}

	std::string Logger::Format(const char** message)
	{
		char buffer[0x1000] = { 0 };

		va_list ap = reinterpret_cast<char*>(const_cast<char**>(&message[1]));
		//va_start(ap, *message);
		_vsnprintf_s(buffer, sizeof(buffer), *message, ap);
		va_end(ap);

		return buffer;
	}

	void Logger::Frame()
	{
		Logger::MessageMutex.lock();

		for (unsigned int i = 0; i < Logger::MessageQueue.size(); ++i)
		{
			Game::Com_PrintMessage(0, Logger::MessageQueue[i].data(), 0);

			if (!Logger::IsConsoleReady())
			{
				OutputDebugStringA(Logger::MessageQueue[i].data());
			}
		}

		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();
	}

	void Logger::PipeOutput(void(*callback)(std::string))
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
			push 1
			push [esp + 8h]
			call Logger::NetworkLog
			add esp, 8h

			mov eax, 4576C0h
			jmp eax
		}
	}

	__declspec(naked) void Logger::PrintMessageStub()
	{
		__asm
		{
			mov eax, Logger::PipeCallback
			test eax, eax
			jz returnPrint

			push [esp + 8h]
			call Logger::PrintMessagePipe
			add esp, 4h
			retn

		returnPrint:
			push 0
			push [esp + 0Ch]
			call Logger::NetworkLog
			add esp, 8h

			push esi
			mov esi, [esp + 0Ch]

			mov eax, 4AA835h
			jmp eax
		}
	}

	void Logger::EnqueueMessage(std::string message)
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.push_back(message);
		Logger::MessageMutex.unlock();
	}

	Logger::Logger()
	{
		Logger::PipeOutput(nullptr);

		QuickPatch::OnFrame(Logger::Frame);

		Utils::Hook(0x4B0218, Logger::GameLogStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(Game::Com_PrintMessage, Logger::PrintMessageStub, HOOK_JUMP).Install()->Quick();

		Dvar::OnInit([] ()
		{
			Command::AddSV("log_add", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				Network::Address addr(params[1]);

				if (std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr) == Logger::LoggingAddresses[0].end())
				{
					Logger::LoggingAddresses[0].push_back(addr);
				}
			});

			Command::AddSV("log_del", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				Network::Address addr(params[1]);

				auto i = std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr);
				if (i != Logger::LoggingAddresses[0].end())
				{
					Logger::LoggingAddresses[0].erase(i);
				}
			});

			Command::AddSV("log_list", [] (Command::Params)
			{
				Logger::Print("# ID: Address\n");
				Logger::Print("-------------\n");

				for (unsigned int i = 0; i < Logger::LoggingAddresses[0].size(); ++i)
				{
					Logger::Print("#%03d: %5s\n", i, Logger::LoggingAddresses[0][i].GetCString());
				}
			});

			Command::AddSV("glog_add", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				Network::Address addr(params[1]);

				if (std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr) == Logger::LoggingAddresses[1].end())
				{
					Logger::LoggingAddresses[1].push_back(addr);
				}
			});

			Command::AddSV("glog_del", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				Network::Address addr(params[1]);

				auto i = std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr);
				if (i != Logger::LoggingAddresses[1].end())
				{
					Logger::LoggingAddresses[1].erase(i);
				}
			});

			Command::AddSV("glog_list", [] (Command::Params)
			{
				Logger::Print("# ID: Address\n");
				Logger::Print("-------------\n");

				for (unsigned int i = 0; i < Logger::LoggingAddresses[1].size(); ++i)
				{
					Logger::Print("#%03d: %5s\n", i, Logger::LoggingAddresses[1][i].GetCString());
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
	}
}
