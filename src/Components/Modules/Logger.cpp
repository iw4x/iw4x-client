#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(*(HWND*)0x64A3288) != FALSE);
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

	void Logger::EnqueueMessage(std::string message)
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.push_back(message);
		Logger::MessageMutex.unlock();
	}

	Logger::Logger()
	{
		Renderer::OnFrame(Logger::Frame); // Client
		Dedicated::OnFrame(Logger::Frame); // Dedi
	}

	Logger::~Logger()
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();
	}
}
