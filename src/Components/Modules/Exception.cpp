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

  // Show the error Com_Error() was handling when another one happened. This
  // replaces its Sys_Error() call for such a recursive error.
  //
  // The game passes com_errorMessage, the localized version of the first
  // error, as the format and the original message as the argument. We've
  // seen com_errorMessage come out empty (with an image that failed to load
  // during startup), leaving the player with an empty dialog and us with an
  // empty report. So we show the original message when that happens. Note
  // that neither is a format string: either may well contain a '%'.
  //
  void Exception::Sys_Error_Recursive_Hk (const char* format, const char* message)
  {
    Game::Sys_Error ("%s", *format != '\0' ? format : message);
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
    Utils::Hook (0x4B2317, Sys_Error_Recursive_Hk, HOOK_CALL).install ()->quick (); // Com_Error()
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
