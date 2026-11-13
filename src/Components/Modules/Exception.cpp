#include "Exception.hpp"
#include "AssetHandler.hpp"

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
  // The game passes the com_errorMessage dvar, the localized version of the
  // first error, as the format. We've seen it come out empty (with an image
  // that failed to load during startup), leaving the player with an empty
  // dialog and us with an empty report. So we show the original message
  // when that happens. Note that neither is a format string: either may
  // well contain a '%'.
  //
  // Note also that x64 leaves the original message out of the call, so we
  // read it from the buffer Com_Error() keeps it in.
  //
  void Exception::Sys_Error_Recursive_Hk (const char* format)
  {
    Game::Sys_Error ("%s", *format != '\0' ? format : Game::com_errorMessage);
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
    SetFilterHook.initialize(reinterpret_cast<void*>(::SetUnhandledExceptionFilter), reinterpret_cast<void*>(SetUnhandledExceptionFilter_Stub), HOOK_JUMP);
    SetFilterHook.install();

    ::SetUnhandledExceptionFilter(&ExceptionFilter);

    Utils::Hook(0x1401F3797, LongJmp_Internal_Stub, HOOK_CALL).install()->quick(); // Com_Error()
    Utils::Hook(0x1401F35A3, Sys_Error_Recursive_Hk, HOOK_CALL).install()->quick(); // Com_Error()

    // Script errors.
    //
    // Note that x64 inlines Scr_Error() into the other script error
    // functions and VM_Execute(), so each copy has its own longjmp() call.
    //
    static constexpr uintptr_t scriptErrorJumps[] =
    {
      0x140227DCE, 0x140228025, 0x140228045, 0x1402280D2, 0x1402280F2,
      0x14022822A, 0x14022824A, 0x1402282DC, 0x1402282FC, 0x1402284C1,
      0x1402284E1, 0x140228570, 0x14022860A, 0x14022862A, 0x1402286B2,
      0x1402286D2, 0x140228772, 0x140228792, 0x140228976, 0x140228996,
      0x140228A20, 0x140228A9C, 0x140228B0D, 0x140228BAC, 0x140228BCC,
      0x14022C887, 0x14022C8A7, 0x14022C8C7, 0x14022C8E7, 0x14022C907,
      0x14022C927, 0x14022C947, 0x14022C967, 0x14022C987, 0x14022C9A7,
      0x14022C9C7, 0x14022C9E7, 0x14022CA07, 0x14022CA27, 0x14022CA47,
      0x14022CA67, 0x14022CA87, 0x14022CAA7, 0x14022CAC7, 0x14022CAE7,
      0x14022CB07, 0x14022CB27, 0x14022CB47, 0x14022CB67, 0x14022CB87,
      0x14022CBA7, 0x14022CBC7, 0x14022CBE7, 0x14022CC07, 0x14022CC27,
    };

    for (const uintptr_t address : scriptErrorJumps)
    {
      Utils::Hook(address, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
    }
  }

  Exception::~Exception()
  {
    SetFilterHook.uninstall();
  }
}
