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
		strftime(filename, sizeof(filename) - 1, "iw4m-" VERSION_STR "-%Y%m%d%H%M%S.dmp", ltime);

		HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
		Utils::Hook::Set(0x6D70AC, Exception::SetUnhandledExceptionFilterStub);
		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
	}
}
