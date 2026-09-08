#include "Exception.hpp"

namespace Components
{
	Utils::Hook Exception::SetFilterHook;
	LPTOP_LEVEL_EXCEPTION_FILTER Exception::ChainedFilter;

	__declspec(noreturn) void Exception::LongJmp_Internal_Stub(jmp_buf env, int status)
	{
		AssetHandler::ResetBypassState();
		Game::longjmp_internal(env, status);
	}

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		if (ChainedFilter)
		{
			ChainedFilter(ExceptionInfo);
		}

		TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void Exception::ChainFilterInstalledBy(const std::function<void()>& installer)
	{
		SetFilterHook.uninstall();

		installer();

		const auto chained = ::SetUnhandledExceptionFilter(&ExceptionFilter);
		if (chained != &ExceptionFilter)
		{
			ChainedFilter = chained;
		}

		SetFilterHook.install();
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
		SetFilterHook.initialize(::SetUnhandledExceptionFilter, SetUnhandledExceptionFilter_Stub, HOOK_JUMP);
		SetFilterHook.install();

		::SetUnhandledExceptionFilter(&ExceptionFilter);

		Utils::Hook(0x4B241F, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61DB44, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F17D, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F248, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F5E7, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
	}

	Exception::~Exception()
	{
		SetFilterHook.uninstall();
	}
}
