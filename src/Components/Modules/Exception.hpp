#pragma once

namespace Components
{
  class Exception : public Component
  {
  public:
    Exception();
    ~Exception();

    static void ChainFilterInstalledBy(const std::function<void()>& installer);

  private:
    static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
    static __declspec(noreturn) void LongJmp_Internal_Stub(jmp_buf env, int status);
    static void Sys_Error_Recursive_Hk (const char* format);
    static __declspec(noreturn) void Assert_Hk (const wchar_t* expression, const wchar_t* file, unsigned line);

    static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_Stub(LPTOP_LEVEL_EXCEPTION_FILTER);

    static Utils::Hook SetFilterHook;
    static LPTOP_LEVEL_EXCEPTION_FILTER ChainedFilter;
  };
}
