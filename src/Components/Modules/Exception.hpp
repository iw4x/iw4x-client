#pragma once

namespace Components
{
	class Exception : public Component
	{
	public:
		Exception();
		~Exception();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Exception"; };
#endif
		static LPTOP_LEVEL_EXCEPTION_FILTER Hook();

		static int MiniDumpType;
		static void SetMiniDumpType();

	private:
		static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
		static __declspec(noreturn) void ErrorLongJmp(jmp_buf _Buf, int _Value);

		static Utils::Hook SetFilterHook;
	};
}
