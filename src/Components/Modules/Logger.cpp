#include "..\..\STDInclude.hpp"

namespace Components
{
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
			Game::Com_Printf(0, "%s", buffer);
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

	Logger::Logger()
	{

	}
}
