#include <STDInclude.hpp>

namespace Components
{
	Utils::Hook Exception::SetFilterHook;
	int Exception::MiniDumpType;

	__declspec(noreturn) void Exception::ErrorLongJmp(jmp_buf _Buf, int _Value)
	{
		if (!*reinterpret_cast<DWORD*>(0x1AD7EB4))
		{
			TerminateProcess(GetCurrentProcess(), 1337);
		}

		longjmp(_Buf, _Value);
	}

	__declspec(noreturn) void Exception::LongJmp(jmp_buf _Buf, int _Value)
	{
		AssetHandler::ResetBypassState();
		longjmp(_Buf, _Value);
	}

	void Exception::SuspendProcess()
	{
		FreeConsole();

		if (IsWindow(Console::GetWindow()) != FALSE)
		{
			CloseWindow(Console::GetWindow());
			DestroyWindow(Console::GetWindow());
		}

		if (IsWindow(Window::GetWindow()) != FALSE)
		{
			CloseWindow(Window::GetWindow());
			DestroyWindow(Window::GetWindow());

			std::this_thread::sleep_for(2s);

			// This makes sure we either destroy the windows or wait till they are destroyed
			MSG msg;
			Utils::Time::Interval interval;
			while (IsWindow(Window::GetWindow()) != FALSE && !interval.elapsed(2s))
			{
				if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				}

				std::this_thread::sleep_for(10ms);
			}
		}

		// This only suspends the main game threads, which is enough for us
		Game::Sys_SuspendOtherThreads();
	}

	void Exception::CopyMessageToClipboard(const std::string& error)
	{
		const auto hWndNewOwner = GetDesktopWindow();
		const auto result = OpenClipboard(hWndNewOwner);

		if (result == FALSE)
			return;

		EmptyClipboard();
		auto* hMem = GlobalAlloc(GMEM_MOVEABLE, error.size() + 1);

		if (hMem == nullptr)
		{
			CloseClipboard();
			return;
		}

		auto lock = GlobalLock(hMem);
		if (lock != nullptr)
		{
			std::memcpy(lock, error.data(), error.size() + 1);
			GlobalUnlock(hMem);
			SetClipboardData(1, hMem);
		}

		CloseClipboard();
		GlobalFree(hMem);
	}

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		std::string errorStr;
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			errorStr = "Termination because of a stack overflow.\nCopy exception address to clipboard?";
		}
		else
		{
			errorStr = Utils::String::VA("Fatal error (0x%08X) at 0x%08X.\nCopy exception address to clipboard?", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
		}

		//Exception::SuspendProcess();

		// Message should be copied to the keyboard if no button is pressed
		if (MessageBoxA(nullptr, errorStr.data(), nullptr, MB_YESNO | MB_ICONERROR) == IDYES)
		{
			Exception::CopyMessageToClipboard(Utils::String::VA("0x%08X", ExceptionInfo->ExceptionRecord->ExceptionAddress));
		}

		if (Flags::HasFlag("bigminidumps"))
		{
			Exception::SetMiniDumpType(true, false);
		}

		// Current executable name
		char exeFileName[MAX_PATH];
		GetModuleFileNameA(nullptr, exeFileName, MAX_PATH);
		PathStripPathA(exeFileName);
		PathRemoveExtensionA(exeFileName);

		// Generate filename
		char filenameFriendlyTime[MAX_PATH];
		__time64_t time;
		tm ltime;
		_time64(&time);
		_localtime64_s(&ltime, &time);
		strftime(filenameFriendlyTime, sizeof(filenameFriendlyTime) - 1, "%Y%m%d%H%M%S", &ltime);

		// Combine with queuedMinidumpsFolder
		char filename[MAX_PATH] = { 0 };
		Utils::IO::CreateDir("minidumps");
		PathCombineA(filename, "minidumps\\", Utils::String::VA("%s-" VERSION "-%s.dmp", exeFileName, filenameFriendlyTime));

		DWORD fileShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
		HANDLE hFile = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, fileShare, nullptr, (fileShare & FILE_SHARE_WRITE) > 0 ? OPEN_ALWAYS : OPEN_EXISTING, NULL, nullptr);
		MINIDUMP_EXCEPTION_INFORMATION ex = { GetCurrentThreadId(), ExceptionInfo, FALSE };
		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, static_cast<MINIDUMP_TYPE>(Exception::MiniDumpType), &ex, nullptr, nullptr))
		{
			MessageBoxA(nullptr, Utils::String::VA("There was an error creating the minidump (%s)! Hit OK to close the program.", Utils::GetLastWindowsError().data()), "Minidump Error", MB_OK | MB_ICONERROR);
			OutputDebugStringA("Failed to create new minidump!");
			Utils::OutputDebugLastError();
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		//if (ExceptionInfo->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE)
		{
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER)
	{
		Exception::SetFilterHook.uninstall();
		LPTOP_LEVEL_EXCEPTION_FILTER retval = SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
		Exception::SetFilterHook.install();
		return retval;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER Exception::Hook()
	{
		return SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
	}

	void Exception::SetMiniDumpType(bool codeseg, bool dataseg)
	{
		Exception::MiniDumpType = MiniDumpIgnoreInaccessibleMemory;
		Exception::MiniDumpType |= MiniDumpWithHandleData;
		Exception::MiniDumpType |= MiniDumpScanMemory;
		Exception::MiniDumpType |= MiniDumpWithProcessThreadData;
		Exception::MiniDumpType |= MiniDumpWithFullMemoryInfo;
		Exception::MiniDumpType |= MiniDumpWithThreadInfo;
		//Exception::MiniDumpType |= MiniDumpWithModuleHeaders;

		if (codeseg)
		{
			Exception::MiniDumpType |= MiniDumpWithCodeSegs;
		}
		if (dataseg)
		{
			Exception::MiniDumpType |= MiniDumpWithDataSegs;
		}
	}

	Exception::Exception()
	{
		Exception::SetMiniDumpType(Flags::HasFlag("bigminidumps"), Flags::HasFlag("reallybigminidumps"));

#if !defined(DEBUG) || defined(FORCE_EXCEPTION_HANDLER)
		Exception::SetFilterHook.initialize(SetUnhandledExceptionFilter, Exception::SetUnhandledExceptionFilterStub, HOOK_JUMP);
		Exception::SetFilterHook.install();

		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
#endif

		//Utils::Hook(0x4B241F, Exception::ErrorLongJmp, HOOK_CALL).install()->quick();
		Utils::Hook(0x6B8898, Exception::LongJmp, HOOK_JUMP).install()->quick();

#ifdef _DEBUG
		Command::Add("mapTest", [](Command::Params* params)
		{
			Game::UI_UpdateArenas();

			std::string command;
			for (auto i = 0; i < (params->size() >= 2 ? atoi(params->get(1)) : *Game::arenaCount); ++i)
			{
				const auto* mapName = ArenaLength::NewArenas[i % *Game::arenaCount].mapName;

				if (!(i % 2)) command.append("wait 250;disconnect;wait 750;"); // Test a disconnect
				else command.append("wait 500;"); // Test direct map switch
				command.append(Utils::String::VA("map %s;", mapName));
			}

			Command::Execute(command, false);
		});
#endif
	}

	Exception::~Exception()
	{
		Exception::SetFilterHook.uninstall();
	}
}
