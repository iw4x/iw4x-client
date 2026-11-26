#include "Exception.hpp"
#include "AssetHandler.hpp"

// The runtime's assert() handler (see Assert_Hk()), which <assert.h> only
// declares without NDEBUG, while the runtime has it in all builds.
//
#ifdef NDEBUG
extern "C" void __cdecl
_wassert (const wchar_t* message, const wchar_t* file, unsigned line);
#endif

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
  // The game passes the localized version of the first error as the
  // format. We've seen it come out empty (with an image that failed to
  // load during startup), which leaves the player with an empty dialog and
  // us with an empty report. So in that case we show the original message
  // instead. Note that neither is a format string, since either may well
  // contain a '%'.
  //
  // Note also that on x86 the game passes the original message along as the
  // argument. x64 leaves it out of the call, so we read it from the buffer
  // that Com_Error() keeps it in (Game::com_errorMessage).
  //
  void Exception::Sys_Error_Recursive_Hk (const char* format)
  {
    Game::Sys_Error ("%s", *format != '\0' ? format : Game::com_errorMessage);
  }

  // A failed assert(). The C runtime would show it in a message box of
  // its own, which says little once the game is fullscreen and nothing
  // in the console log. So we print it to the log and the debugger, then
  // end with IW4's fatal error, which shows the same message.
  //
  void Exception::Assert_Hk (const wchar_t* expression, const wchar_t* file, unsigned line)
  {
    const std::string m (std::format ("assertion failed: {} ({}:{})",
                                      Utils::String::Convert (expression),
                                      Utils::String::Convert (file),
                                      line));

    Logger::Print ("{}\n", m);
    OutputDebugStringA ((m + '\n').c_str ());

    Game::Sys_Error ("%s", m.c_str ());

    // Sys_Error() doesn't return, but make sure we don't either.
    //
    std::abort ();
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

    // Route the client's failed assert()s through IW4 (see Assert_Hk()).
    // The runtime is linked statically, so _wassert() is ours to patch.
    //
    Utils::Hook (reinterpret_cast<void*> (&_wassert), reinterpret_cast<void*> (&Assert_Hk), HOOK_JUMP).install ()->quick ();

    Utils::Hook(0x1401F3797, LongJmp_Internal_Stub, HOOK_CALL).install()->quick(); // Com_Error()
    Utils::Hook(0x1401F35A3, Sys_Error_Recursive_Hk, HOOK_CALL).install()->quick(); // Com_Error()

    // On x86 we hook the longjmp() of Scr_Error() and a few others. x64
    // inlines Scr_Error() into the other script error functions and into
    // VM_Execute(), each copy with its own longjmp() call, which is why the
    // list is this long.
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
