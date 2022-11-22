#include <STDInclude.hpp>

namespace Utils
{
	std::string GetMimeType(const std::string& url)
	{
		wchar_t* mimeType = nullptr;
		FindMimeFromData(nullptr, std::wstring(url.begin(), url.end()).data(), nullptr, 0, nullptr, 0, &mimeType, 0);

		if (mimeType)
		{
			std::wstring wMimeType(mimeType);
			return String::Convert(wMimeType);
		}

		return "application/octet-stream";
	}

	std::string ParseChallenge(const std::string& data)
	{
		const auto pos = data.find_first_of("\n ");
		if (pos == std::string::npos) return data;
		return data.substr(0, pos);
	}

	void OutputDebugLastError()
	{
		DWORD errorMessageID = ::GetLastError();
		OutputDebugStringA(Utils::String::VA("Last error code: 0x%08X (%s)\n", errorMessageID, GetLastWindowsError().data()));
	}

	std::string GetLastWindowsError()
	{
		DWORD errorMessageID = ::GetLastError();
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);
		std::string message(messageBuffer, size);
		LocalFree(messageBuffer);
		return message;
	}

	bool IsWineEnvironment()
	{
		HMODULE hntdll = GetModuleHandleA("ntdll.dll");
		if (!hntdll) return false;

		return (GetProcAddress(hntdll, "wine_get_version") != nullptr);
	}

	unsigned long GetParentProcessId()
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

		Memory::Allocator allocator;
		allocator.reference(hSnapshot, [](void* handle) { CloseHandle(handle); });

		PROCESSENTRY32 pe32;
		ZeroMemory(&pe32, sizeof(pe32));
		pe32.dwSize = sizeof(pe32);

		DWORD pid = GetCurrentProcessId();
		while (Process32Next(hSnapshot, &pe32))
		{
			if (pe32.th32ProcessID == pid)
			{
				return pe32.th32ParentProcessID;
			}
		}

		return 0;
	}

	std::size_t GetModuleSize(HMODULE module)
	{
		PIMAGE_DOS_HEADER header = PIMAGE_DOS_HEADER(module);
		PIMAGE_NT_HEADERS ntHeader = PIMAGE_NT_HEADERS(DWORD(module) + header->e_lfanew);
		return ntHeader->OptionalHeader.SizeOfImage;
	}

	void* GetThreadStartAddress(HANDLE hThread)
	{
		HMODULE ntdll = GetNTDLL();
		if (!ntdll) return nullptr;


		static uint8_t ntQueryInformationThread[] = { 0xB1, 0x8B, 0xAE, 0x8A, 0x9A, 0x8D, 0x86, 0xB6, 0x91, 0x99, 0x90, 0x8D, 0x92, 0x9E, 0x8B, 0x96, 0x90, 0x91, 0xAB, 0x97, 0x8D, 0x9A, 0x9E, 0x9B }; // NtQueryInformationThread
		NtQueryInformationThread_t NtQueryInformationThread = NtQueryInformationThread_t(GetProcAddress(ntdll, String::XOR(std::string(reinterpret_cast<char*>(ntQueryInformationThread), sizeof ntQueryInformationThread), -1).data()));
		if (!NtQueryInformationThread) return nullptr;

		HANDLE dupHandle, currentProcess = GetCurrentProcess();
		if (!DuplicateHandle(currentProcess, hThread, currentProcess, &dupHandle, THREAD_QUERY_INFORMATION, FALSE, 0))
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return nullptr;
		}

		void* address = nullptr;
		NTSTATUS status = NtQueryInformationThread(dupHandle, ThreadQuerySetWin32StartAddress, &address, sizeof(address), nullptr);
		CloseHandle(dupHandle);

		if (status != 0) return nullptr;
		return address;
	}

	void SetLegacyEnvironment()
	{
		wchar_t binaryPath[512]{};
		GetModuleFileNameW(GetModuleHandleW(nullptr), binaryPath, sizeof(binaryPath) / sizeof(wchar_t));

		auto* exeBaseName = std::wcsrchr(binaryPath, L'\\');
		exeBaseName[0] = L'\0';

		// Make the game work without the xlabs launcher
		SetCurrentDirectoryW(binaryPath);
	}

	void SetEnvironment()
	{
		wchar_t* buffer{};
		std::size_t size{};
		if (_wdupenv_s(&buffer, &size, L"XLABS_MW2_INSTALL") != 0 || buffer == nullptr)
		{
			SetLegacyEnvironment();
			return;
		}

		const auto _0 = gsl::finally([&] { std::free(buffer); });

		SetCurrentDirectoryW(buffer);
		SetDllDirectoryW(buffer);
	}

	HMODULE GetNTDLL()
	{
		static uint8_t ntdll[] = { 0x91, 0x8B, 0x9B, 0x93, 0x93, 0xD1, 0x9B, 0x93, 0x93 }; // ntdll.dll
		return GetModuleHandleA(Utils::String::XOR(std::string(reinterpret_cast<char*>(ntdll), sizeof ntdll), -1).data());
	}

	void SafeShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
	{
		[=]
		{
			__try
			{
				ShellExecuteA(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
			}
			__finally
			{}
		}();

		std::this_thread::yield();
	}

	void OpenUrl(const std::string& url)
	{
		SafeShellExecute(nullptr, "open", url.data(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	bool HasIntersection(unsigned int base1, unsigned int len1, unsigned int base2, unsigned int len2)
	{
		return !(base1 + len1 <= base2 || base2 + len2 <= base1);
	}
}
