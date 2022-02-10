#pragma once

namespace Components
{
	class Exception : public Component
	{
	public:
		Exception();
		~Exception();

		static LPTOP_LEVEL_EXCEPTION_FILTER Hook();

		static void SetMiniDumpType(bool codeseg, bool dataseg);

	private:
		static void SuspendProcess();
		static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
		static __declspec(noreturn) void ErrorLongJmp(jmp_buf _Buf, int _Value);
		static __declspec(noreturn) void LongJmp(jmp_buf _Buf, int _Value);

		static void CopyMessageToClipboard(const std::string& error);

		static int MiniDumpType;
		static Utils::Hook SetFilterHook;
	};
}
