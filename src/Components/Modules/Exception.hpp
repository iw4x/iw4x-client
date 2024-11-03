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
		static __declspec(noreturn) void LongJmp_Internal_Stub(jmp_buf env, int status);

		static std::wstring GetErrorMessage();
		static std::string FormatMessageForClipboard(const std::wstring& message);

		static void DisplayErrorMessage(const std::wstring& title, const std::wstring& message);
		static void CopyMessageToClipboard(const char* error);

		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_Stub(LPTOP_LEVEL_EXCEPTION_FILTER);
		static HRESULT CALLBACK TaskDialogCallbackProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData);

		static int MiniDumpType;
		static Utils::Hook SetFilterHook;
	};
}
