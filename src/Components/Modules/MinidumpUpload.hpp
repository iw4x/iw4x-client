#pragma once

#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma warning(pop)

extern const int MiniDumpTiny;

namespace Components
{
	// This class holds a Minidump and allows easy access to its aspects.
	class Minidump
	{
	public:
		~Minidump();

		static Minidump* Create(std::string path, LPEXCEPTION_POINTERS exceptionInfo, int type = MiniDumpTiny);
		static Minidump* Open(std::string path);
		bool GetStream(MINIDUMP_STREAM_TYPE type, PMINIDUMP_DIRECTORY* directoryPtr, PVOID* streamPtr, ULONG* streamSizePtr);
		bool Check();
		std::string ToString();

	private:
		Minidump();

		bool EnsureFileMapping();

		static Minidump* Initialize(std::string path, DWORD fileShare = FILE_SHARE_READ | FILE_SHARE_WRITE);

		HANDLE fileHandle;
		HANDLE mapFileHandle;
	};

	class MinidumpUpload : public Component
	{
	public:
#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "MinidumpUpload"; };
#endif
		MinidumpUpload();
		~MinidumpUpload();

		// Uploads the given minidump.
		static bool Upload(Minidump* minidump);

		// Generates a new minidump and saves it to the folder for queued minidumps.
		static Minidump* CreateQueuedMinidump(LPEXCEPTION_POINTERS exceptionInfo, int minidumpType = MiniDumpTiny);

		// Browses the folder for queued minidumps and uploads each queued minidump.
		// On Release builds this will also delete every successfully uploaded minidump.
		static bool UploadQueuedMinidumps();

	private:
		std::thread uploadThread;

		// Encodes the given minidump so that it can be uploaded in a proper format.
		// Internally, this will compress the minidump and decorate it with proper markers and first-look headers.
		static std::string Encode(std::string data, std::map<std::string, std::string> extraHeaders = {});

		// Ensures the queued minidumps folder exists. Will return false if the directory can't be created and does not exist.
		static bool EnsureQueuedMinidumpsFolderExists();

		// Contains the path to the minidumps folder.
		static const std::string queuedMinidumpsFolder;

#ifdef DISABLE_BITMESSAGE
		static const std::vector<std::string> targetUrls;
#else
		static const std::string targetAddress;
		static const unsigned int maxSegmentSize;
#endif
	};
}
