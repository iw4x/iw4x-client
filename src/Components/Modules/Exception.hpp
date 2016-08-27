#define UPLOAD_URL "https://reich.io/upload.php"
#define BITMESSAGE_UPLOAD_IDENTITY "BM-NBhqCHraxGyUT38cfhRQYWx29gJ7QePa"
#define BITMESSAGE_SIZE_LIMIT 25600

namespace Components
{
	class Exception : public Component
	{
	public:
		Exception();

#ifdef DEBUG
		const char* GetName() { return "Exception"; };
#endif

	private:
		static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
		static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

		static bool UploadMinidump(std::string filename);
		static bool UploadMinidump2BM(std::string filename);
	};
}
