#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
	void(*Logger::PipeCallback)(std::string) = nullptr;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(*reinterpret_cast<HWND*>(0x64A3288)) != FALSE || (Dedicated::IsDedicated() && !Flags::HasFlag("console")));
	}

	void Logger::Print(const char* message, ...)
	{
		char buffer[0x1000] = { 0 };

		va_list ap;
		va_start(ap, message);
		vsprintf_s(buffer, message, ap);
		va_end(ap);

		if (Logger::IsConsoleReady())
		{
			if (!Game::Sys_IsMainThread())
			{
				Logger::EnqueueMessage(buffer);
			}
			else
			{
				Game::Com_PrintMessage(0, buffer, 0);
			}
		}
		else
		{
			// Only print to stdout, when doing unit tests
			if (Loader::PerformingUnitTests())
			{
				printf("%s", buffer);
			}

			OutputDebugStringA(buffer);
		}
	}

	void Logger::Error(const char* message, ...)
	{
		char buffer[0x1000] = { 0 };

		va_list ap;
		va_start(ap, message);
		vsprintf_s(buffer, message, ap);
		va_end(ap);

		Game::Com_Error(0, "%s", buffer);
	}

	void Logger::SoftError(const char* message, ...)
	{
		char buffer[0x1000] = { 0 };

		va_list ap;
		va_start(ap, message);
		vsprintf_s(buffer, message, ap);
		va_end(ap);

		Game::Com_Error(2, "%s", buffer);
	}

	void Logger::Frame()
	{
		Logger::MessageMutex.lock();

		for (unsigned int i = 0; i < Logger::MessageQueue.size(); ++i)
		{
			if (Logger::IsConsoleReady())
			{
				Game::Com_PrintMessage(0, Logger::MessageQueue[i].data(), 0);
			}
			else
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

	void __declspec(naked) Logger::PrintMessageStub()
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

		Utils::Hook(Game::Com_PrintMessage, Logger::PrintMessageStub, HOOK_JUMP).Install()->Quick();
	}

	Logger::~Logger()
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();
	}
}
