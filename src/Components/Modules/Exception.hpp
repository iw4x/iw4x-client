

namespace Components
{
	class Exception : public Component
	{
	public:
		Exception();
		~Exception();

#ifdef DEBUG
		const char* GetName() { return "Exception"; };
#endif
		static LPTOP_LEVEL_EXCEPTION_FILTER Hook();

	private:
		static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

		static Utils::Hook SetFilterHook;
	};
}
