#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
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
			Game::Com_PrintMessage(0, message.data(), 0);
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
