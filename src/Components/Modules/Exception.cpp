#include "STDInclude.hpp"

// Stuff causes warnings
#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma warning(pop)

namespace Components
{
	Utils::Hook Exception::SetFilterHook;

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		auto minidump = MinidumpUpload::Singleton->CreateQueuedMinidump(ExceptionInfo);
		if (minidump == NULL)
		{
			OutputDebugStringA("Failed to create new minidump!");
			Utils::OutputDebugLastError();
		}

		printf("Trying to print an error message from ExceptionFilter...");
		switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
		{
		case EXCEPTION_STACK_OVERFLOW:
			Logger::Error("Termination because of a stack overflow.\n");
			break;
		default:
			Logger::Error("Fatal error (0x%08X) at 0x%08X.", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
			break;
		}

		//TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);

		return EXCEPTION_CONTINUE_SEARCH;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER)
	{
		Exception::SetFilterHook.Uninstall();
		LPTOP_LEVEL_EXCEPTION_FILTER retval = SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
		Exception::SetFilterHook.Install();
		return retval;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER Exception::Hook()
	{
		return SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
	}

	Exception::Exception()
	{
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
		Exception::SetFilterHook.Initialize(SetUnhandledExceptionFilter, Exception::SetUnhandledExceptionFilterStub, HOOK_JUMP);
		Exception::SetFilterHook.Install();

		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
#endif

		Command::Add("mapTest", [](Command::Params params)
		{
			std::string command;

			int max = (params.Length() >= 2 ? atoi(params[1]) : 16), current = 0;

			for (int i = 0;;)
			{
				char* mapname = Game::mapnames[i];
				if (!*mapname)
				{
					i = 0;
					continue;
				}

				if (!(i % 2)) command.append(fmt::sprintf("wait 250;disconnect;wait 750;", mapname)); // Test a disconnect
				else command.append(fmt::sprintf("wait 500;", mapname));                             // Test direct map switch
				command.append(fmt::sprintf("map %s;", mapname));

				++i, ++current;

				if (current >= max) break;
			}

			Command::Execute(command, false);
		});
		Command::Add("debug_exceptionhandler", [](Command::Params)
		{
			Logger::Print("Rerunning SetUnhandledExceptionHandler...\n");
			auto oldHandler = Exception::Hook();
			Logger::Print("Old exception handler was 0x%010X.\n", oldHandler);
		});
#pragma warning(push)
#pragma warning(disable:4740) // flow in or out of inline asm code suppresses global optimization
		Command::Add("debug_minidump", [](Command::Params)
		{
			// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

			EXCEPTION_RECORD ExceptionRecord;
			CONTEXT ContextRecord;
			memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_

			__asm {
				mov dword ptr[ContextRecord.Eax], eax
				mov dword ptr[ContextRecord.Ecx], ecx
				mov dword ptr[ContextRecord.Edx], edx
				mov dword ptr[ContextRecord.Ebx], ebx
				mov dword ptr[ContextRecord.Esi], esi
				mov dword ptr[ContextRecord.Edi], edi
				mov word ptr[ContextRecord.SegSs], ss
				mov word ptr[ContextRecord.SegCs], cs
				mov word ptr[ContextRecord.SegDs], ds
				mov word ptr[ContextRecord.SegEs], es
				mov word ptr[ContextRecord.SegFs], fs
				mov word ptr[ContextRecord.SegGs], gs
				pushfd
				pop[ContextRecord.EFlags]
			}

			ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
			ContextRecord.Eip = (ULONG)_ReturnAddress();
			ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
			ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);


#elif defined (_IA64_) || defined (_AMD64_)

			/* Need to fill up the Context in IA64 and AMD64. */
			RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

			ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

			ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

			ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
			ExceptionRecord.ExceptionAddress = _ReturnAddress();


			EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
			memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
			CONTEXT* pContextRecord = new CONTEXT;
			memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

			auto eptr = new EXCEPTION_POINTERS;
			eptr->ExceptionRecord = pExceptionRecord;
			eptr->ContextRecord = pContextRecord;

			Exception::ExceptionFilter(eptr);
		});
#pragma warning(pop)
	}

	Exception::~Exception()
	{
		Exception::SetFilterHook.Uninstall();
	}
}
