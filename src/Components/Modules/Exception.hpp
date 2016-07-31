#define UPLOAD_URL "https://momo5502.com/test/upload.php"

namespace Components
{
	class Exception : public Component
	{
	public:
		Exception();
		const char* GetName() { return "Exception"; };

	private:
		static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

		static void UploadMinidump(std::string filename);
	};
}
