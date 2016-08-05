#define UPLOAD_URL "https://reich.io/upload.php"

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

		static bool UploadMinidump(std::string filename);
	};
}
