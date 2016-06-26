#include "STDInclude.hpp"

// Stuff causes warnings
#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma warning(pop)

namespace Components
{
	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		char filename[MAX_PATH];
		__time64_t time;
		tm* ltime;

		_time64(&time);
		ltime = _localtime64(&time);
		strftime(filename, sizeof(filename) - 1, "iw4x-" VERSION_STR "-%Y%m%d%H%M%S.dmp", ltime);

		HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile && hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION ex = { GetCurrentThreadId(),ExceptionInfo, FALSE };
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL);
			CloseHandle(hFile);
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			Logger::Error("Termination because of a stack overflow.\n");
			TerminateProcess(GetCurrentProcess(), EXCEPTION_STACK_OVERFLOW);
		}
		else
		{
			Logger::Error("Fatal error (0x%08x) at 0x%08x.", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
	{
		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
		return lpTopLevelExceptionFilter;
	}

	Exception::Exception()
	{
#ifdef DEBUG
		// Display DEBUG branding, so we know we're on a debug build
		Renderer::OnFrame([] ()
		{
			Game::Font* font = Game::R_RegisterFont("fonts/normalFont");
			float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Change the color when attaching a debugger
			if (IsDebuggerPresent())
			{
				color[0] = 0.6588f;
				color[1] = 1.0000f;
				color[2] = 0.0000f;
			}

			Game::R_AddCmdDrawText("DEBUG-BUILD", 0x7FFFFFFF, font, 15.0f, 10.0f + Game::R_TextHeight(font), 1.0f, 1.0f, 0.0f, color, Game::ITEM_TEXTSTYLE_SHADOWED);
		});
#else
		Utils::Hook::Set(0x6D70AC, Exception::SetUnhandledExceptionFilterStub);
		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
#endif

		Command::Add("mapTest", [] (Command::Params params)
		{
			std::string command;

			int max = (params.Length() >= 2 ? atoi(params[1]) : 16), current = 0;

			for (int i =0;;)
			{
				char* mapname = reinterpret_cast<char*>(0x7471D0) + 40 * i;
				if (!*mapname)
				{
					i = 0;
					continue;
				}

				if(!(i % 2)) command.append(Utils::VA("wait 250;disconnect;wait 750;", mapname)); // Test a disconnect
				else command.append(Utils::VA("wait 500;", mapname));                             // Test direct map switch
				command.append(Utils::VA("map %s;", mapname));

				++i, ++current;

				if (current >= max) break;
			}

			Command::Execute(command, false);
		});
	}
}
