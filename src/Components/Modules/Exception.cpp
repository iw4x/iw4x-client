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

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		auto minidump = MinidumpUpload::CreateQueuedMinidump(ExceptionInfo, Exception::MiniDumpType);
		if (!minidump)
		{
			OutputDebugStringA("Failed to create new minidump!");
			Utils::OutputDebugLastError();
		}
		else
		{
			delete minidump;
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			Logger::Error("Termination because of a stack overflow.\n");
		}
		else
		{
			Logger::Error("Fatal error (0x%08X) at 0x%08X.", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
		}

		//TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);

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

	void Exception::SetMiniDumpType()
	{
		Exception::MiniDumpType = MiniDumpIgnoreInaccessibleMemory;
		Exception::MiniDumpType |= MiniDumpWithUnloadedModules;
		Exception::MiniDumpType |= MiniDumpWithThreadInfo;
		Exception::MiniDumpType |= MiniDumpWithFullMemoryInfo;
		Exception::MiniDumpType |= MiniDumpWithHandleData;
		Exception::MiniDumpType |= MiniDumpWithTokenInformation;
		Exception::MiniDumpType |= MiniDumpWithProcessThreadData;
		Exception::MiniDumpType |= MiniDumpWithFullAuxiliaryState;

		if (Flags::HasFlag("bigminidumps"))
		{
			Exception::MiniDumpType |= MiniDumpWithModuleHeaders;
			Exception::MiniDumpType |= MiniDumpWithCodeSegs;
		}
		else if (Flags::HasFlag("reallybigminidumps"))
		{
			Exception::MiniDumpType |= MiniDumpWithModuleHeaders;
			Exception::MiniDumpType |= MiniDumpWithCodeSegs;
			Exception::MiniDumpType |= MiniDumpWithDataSegs;
		}
	}

	Exception::Exception()
	{
		Exception::SetMiniDumpType();

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
	}

	Exception::~Exception()
	{
		Exception::SetFilterHook.uninstall();
	}
}
