#include <STDInclude.hpp>
#include "Console.hpp"

#include <version.hpp>

namespace Components
{
	Utils::Hook Exception::SetFilterHook;
	int Exception::MiniDumpType;

	__declspec(noreturn) void Exception::LongJmp_Internal_Stub(jmp_buf env, int status)
	{
		AssetHandler::ResetBypassState();
		Game::longjmp_internal(env, status);
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

		auto* lock = GlobalLock(hMem);
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

		// Message should be copied to the keyboard if no button is pressed
		if (MessageBoxA(nullptr, errorStr.data(), nullptr, MB_YESNO | MB_ICONERROR) == IDYES)
		{
			CopyMessageToClipboard(Utils::String::VA("0x%08X", ExceptionInfo->ExceptionRecord->ExceptionAddress));
		}

		if (Flags::HasFlag("bigminidumps"))
		{
			SetMiniDumpType(true, false);
		}

		// Current executable name
		char exeFileName[MAX_PATH];
		GetModuleFileNameA(nullptr, exeFileName, MAX_PATH);
		PathStripPathA(exeFileName);
		PathRemoveExtensionA(exeFileName);

		// Generate filename
		char filenameFriendlyTime[MAX_PATH]{};
		__time64_t time;
		tm ltime;
		_time64(&time);
		_localtime64_s(&ltime, &time);
		strftime(filenameFriendlyTime, sizeof(filenameFriendlyTime) - 1, "%Y%m%d%H%M%S", &ltime);

		// Combine with queued MinidumpsFolder
		char filename[MAX_PATH]{};
		CreateDirectoryA("minidumps", nullptr);
		PathCombineA(filename, "minidumps\\", Utils::String::VA("%s-" VERSION "-%s.dmp", exeFileName, filenameFriendlyTime));

		constexpr auto fileShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
		HANDLE hFile = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, fileShare, nullptr, (fileShare & FILE_SHARE_WRITE) > 0 ? OPEN_ALWAYS : OPEN_EXISTING, NULL, nullptr);
		MINIDUMP_EXCEPTION_INFORMATION ex = { GetCurrentThreadId(), ExceptionInfo, FALSE };
		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, static_cast<MINIDUMP_TYPE>(MiniDumpType), &ex, nullptr, nullptr))
		{
			MessageBoxA(nullptr, Utils::String::Format("There was an error creating the minidump ({})! Hit OK to close the program.", Utils::GetLastWindowsError()), "ERROR", MB_OK | MB_ICONERROR);
#ifdef _DEBUG
			OutputDebugStringA("Failed to create new minidump!");
#endif
			Utils::OutputDebugLastError();
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		{
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	void Exception::SetMiniDumpType(bool codeseg, bool dataseg)
	{
		MiniDumpType = MiniDumpIgnoreInaccessibleMemory;
		MiniDumpType |= MiniDumpWithHandleData;
		MiniDumpType |= MiniDumpScanMemory;
		MiniDumpType |= MiniDumpWithProcessThreadData;
		MiniDumpType |= MiniDumpWithFullMemoryInfo;
		MiniDumpType |= MiniDumpWithThreadInfo;

		if (codeseg)
		{
			MiniDumpType |= MiniDumpWithCodeSegs;
		}

		if (dataseg)
		{
			MiniDumpType |= MiniDumpWithDataSegs;
		}
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilter_Stub(LPTOP_LEVEL_EXCEPTION_FILTER)
	{
		SetFilterHook.uninstall();
		LPTOP_LEVEL_EXCEPTION_FILTER result = ::SetUnhandledExceptionFilter(&ExceptionFilter);
		SetFilterHook.install();
		return result;
	}

	Exception::Exception()
	{
		SetMiniDumpType(Flags::HasFlag("bigminidumps"), Flags::HasFlag("reallybigminidumps"));

		SetFilterHook.initialize(::SetUnhandledExceptionFilter, SetUnhandledExceptionFilter_Stub, HOOK_JUMP);
		SetFilterHook.install();

		::SetUnhandledExceptionFilter(&ExceptionFilter);

		Utils::Hook(0x4B241F, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61DB44, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F17D, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F248, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F5E7, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();

#ifdef MAP_TEST
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
		SetFilterHook.uninstall();
	}
}
