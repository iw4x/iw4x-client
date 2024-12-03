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
#ifdef DEBUG
		DWORD errorMessageID = ::GetLastError();
		OutputDebugStringA(String::VA("Last error code: 0x%08X (%s)\n", errorMessageID, GetLastWindowsError().data()));
#endif
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

	std::string GetWindowsVersion()
	{
		const auto ntdll = Utils::Library("ntdll.dll");
		const auto rtlGetVersion = ntdll.getProc<LONG(WINAPI*)(PRTL_OSVERSIONINFOW)>("RtlGetVersion");

		if (rtlGetVersion)
		{
			RTL_OSVERSIONINFOW versionInfo = {};
			versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
			rtlGetVersion(&versionInfo);

			const auto major = versionInfo.dwMajorVersion;
			const auto minor = versionInfo.dwMinorVersion;
			const auto build = versionInfo.dwBuildNumber;
			const auto arch  = Utils::GetWindowsArchitecture();

			if (major == 10 && build >= 22000) return std::format("Windows 11 (Build {}) {}", build, arch);
			if (major == 10)				   return std::format("Windows 10 (Build {}) {}", build, arch);
			if (major == 6 && minor == 3)	   return std::format("Windows 8.1 (Build {}) {}", build, arch);
			if (major == 6 && minor == 2)	   return std::format("Windows 8.0 (Build {}) {}", build, arch);
			if (major == 6 && minor == 1)	   return std::format("Windows 7 (Build {}) {}", build, arch);
			if (major == 6 && minor == 0)	   return std::format("Windows Vista (Build {}) {}", build, arch);
			if (major == 5 && minor == 2)	   return std::format("Windows XP Professional (Build {}) {}", build, arch);
			if (major == 5 && minor == 1)	   return std::format("Windows XP (Build {}) {}", build, arch);
		}

		return "Unknown Version";
	}

	std::string GetWindowsArchitecture()
	{
		SYSTEM_INFO sysInfo;
		::GetNativeSystemInfo(&sysInfo);

		switch (sysInfo.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_AMD64: return "64 Bit";
			case PROCESSOR_ARCHITECTURE_INTEL: return "32 Bit";
			case PROCESSOR_ARCHITECTURE_ARM:   return "ARM";
			default: return "Unknown Architecture";
		}
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


		NtQueryInformationThread_t NtQueryInformationThread = NtQueryInformationThread_t(GetProcAddress(ntdll, "NtQueryInformationThread"));
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

		// Make the game work without the AlterWare launcher
		SetCurrentDirectoryW(binaryPath);
	}

	/**
	 * IW4x_INSTALL should point to where the IW4x rawfiles/client files are
	 * or the current working dir
	*/
	void SetEnvironment()
	{
		char* buffer{};
		std::size_t size{};
		if (_dupenv_s(&buffer, &size, "IW4x_INSTALL") != 0 || buffer == nullptr)
		{
			SetLegacyEnvironment();
			return;
		}

		const auto _0 = gsl::finally([&] { std::free(buffer); });

		SetCurrentDirectoryA(buffer);
		SetDllDirectoryA(buffer);
	}

	/**
	 * Points to where the Modern Warfare 2 folder is
	*/
	std::filesystem::path GetBaseFilesLocation()
	{
		char* buffer{};
		std::size_t size{};
		if (_dupenv_s(&buffer, &size, "BASE_INSTALL") != 0 || buffer == nullptr)
		{
			return {};
		}

		const auto _0 = gsl::finally([&] { std::free(buffer); });

		try
		{
			std::filesystem::path result = buffer;
			return result;
		}
		catch (const std::exception& ex)
		{
			printf("Failed to convert '%s' to native file system path. Got error '%s'\n", buffer, ex.what());
			return {};
		}
	}

	std::wstring GetLaunchParameters()
	{
		std::wstring args;

		int numArgs;
		auto* const argv = CommandLineToArgvW(GetCommandLineW(), &numArgs);

		if (argv)
		{
			for (auto i = 1; i < numArgs; ++i)
			{
				std::wstring arg(argv[i]);
				args.append(arg);
				args.append(L" ");
			}

			LocalFree(argv);
		}

		return args;
	}

	HMODULE GetNTDLL()
	{
		return GetModuleHandleA("ntdll.dll");
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
