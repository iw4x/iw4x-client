#include "STDInclude.hpp"

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

		if (IsWindow(Window::GetWindow()) != FALSE)
		{
			CloseWindow(Window::GetWindow());
			DestroyWindow(Window::GetWindow());

			std::this_thread::sleep_for(2s);

			// This makes sure we either destroy the windows or wait till they are destroyed
			MSG msg;
			while (IsWindow(Window::GetWindow()) != FALSE && GetMessage(&msg, nullptr, NULL, NULL))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// This only suspends the main game threads, which is enough for us
		Game::Sys_SuspendOtherThreads();
	}

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		const char* errorStr;
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			errorStr = "Termination because of a stack overflow.";
		}
		else
		{
			errorStr = Utils::String::VA("Fatal error (0x%08X) at 0x%08X.", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
		}

		Exception::SuspendProcess();

		bool doFullDump = Flags::HasFlag("bigdumps") || Flags::HasFlag("reallybigdumps");
		/*if (!doFullDump)
		{
			if (MessageBoxA(nullptr, 
				Utils::String::VA("%s\n\n" // errorStr
								  "Would you like to create a full crash dump for the developers (this can be 100mb or more)?\nNo will create small dumps that are automatically uploaded.", errorStr),
					              "IW4x Error!", MB_YESNO | MB_ICONERROR) == IDYES)
			{
				doFullDump = true;
			}
		}*/

		MessageBoxA(nullptr, errorStr, "ERROR", MB_ICONERROR);

		if (doFullDump)
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

		if (ExceptionInfo->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE)
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

#ifdef DEBUG
		// Display DEBUG branding, so we know we're on a debug build
		Renderer::OnFrame([]()
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
#endif
#if !defined(DEBUG) || defined(FORCE_EXCEPTION_HANDLER)
		Exception::SetFilterHook.initialize(SetUnhandledExceptionFilter, Exception::SetUnhandledExceptionFilterStub, HOOK_JUMP);
		Exception::SetFilterHook.install();

		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
#endif

		//Utils::Hook(0x4B241F, Exception::ErrorLongJmp, HOOK_CALL).install()->quick();
		Utils::Hook(0x6B8898, Exception::LongJmp, HOOK_JUMP).install()->quick();

		Command::Add("mapTest", [](Command::Params* params)
		{
			Game::UI_UpdateArenas();

			std::string command;
			for (int i = 0; i < (params->length() >= 2 ? atoi(params->get(1)) : *Game::arenaCount); ++i)
			{
				char* mapname = ArenaLength::NewArenas[i % *Game::arenaCount].mapName;

				if (!(i % 2)) command.append(Utils::String::VA("wait 250;disconnect;wait 750;", mapname)); // Test a disconnect
				else command.append(Utils::String::VA("wait 500;", mapname));                              // Test direct map switch
				command.append(Utils::String::VA("map %s;", mapname));
			}

			Command::Execute(command, false);
		});

		Command::Add("debug_exceptionhandler", [] (Command::Params*)
		{
			Logger::Print("Rerunning SetUnhandledExceptionHandler...\n");
			auto oldHandler = Exception::Hook();
			Logger::Print("Old exception handler was 0x%010X.\n", oldHandler);
		});

#pragma warning(push)
#pragma warning(disable:4740) // flow in or out of inline asm code suppresses global optimization
		Command::Add("debug_minidump", [](Command::Params*)
		{
			// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

			CONTEXT ContextRecord;
			EXCEPTION_RECORD ExceptionRecord;
			ZeroMemory(&ContextRecord, sizeof(CONTEXT));

			__asm
			{
				mov [ContextRecord.Eax], eax
				mov [ContextRecord.Ecx], ecx
				mov [ContextRecord.Edx], edx
				mov [ContextRecord.Ebx], ebx
				mov [ContextRecord.Esi], esi
				mov [ContextRecord.Edi], edi
				mov word ptr [ContextRecord.SegSs], ss
				mov word ptr [ContextRecord.SegCs], cs
				mov word ptr [ContextRecord.SegDs], ds
				mov word ptr [ContextRecord.SegEs], es
				mov word ptr [ContextRecord.SegFs], fs
				mov word ptr [ContextRecord.SegGs], gs

				pushfd
				pop [ContextRecord.EFlags]
			}

			ContextRecord.ContextFlags = CONTEXT_CONTROL;
			ContextRecord.Eip = reinterpret_cast<DWORD>(_ReturnAddress());
			ContextRecord.Esp = reinterpret_cast<DWORD>(_AddressOfReturnAddress());
			ContextRecord.Ebp = *reinterpret_cast<DWORD*>(_AddressOfReturnAddress()) - 1;

			ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

			ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
			ExceptionRecord.ExceptionAddress = _ReturnAddress();

			EXCEPTION_POINTERS eptr;
			eptr.ExceptionRecord = &ExceptionRecord;
			eptr.ContextRecord = &ContextRecord;

			Exception::ExceptionFilter(&eptr);
		});
#pragma warning(pop)

		// Check if folder exists && crash-helper exists
		
		if (Utils::IO::DirectoryExists("minidumps\\") && Utils::IO::FileExists("crash-helper.exe"))
		{
			if (!Utils::IO::DirectoryIsEmpty("minidumps\\"))
			{
				STARTUPINFOA        sInfo;
				PROCESS_INFORMATION pInfo;

				ZeroMemory(&sInfo, sizeof(sInfo));
				ZeroMemory(&pInfo, sizeof(pInfo));
				sInfo.cb = sizeof(sInfo);

				CreateProcessA("crash-helper.exe", const_cast<char*>(Utils::String::VA("crash-helper.exe %s", VERSION)), nullptr, nullptr, false, NULL, nullptr, nullptr, &sInfo, &pInfo);

				if (pInfo.hThread && pInfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hThread);
				if (pInfo.hProcess && pInfo.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pInfo.hProcess);
			}
		}
	}

	Exception::~Exception()
	{
		Exception::SetFilterHook.uninstall();
	}
}
