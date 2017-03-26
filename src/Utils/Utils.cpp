#include "STDInclude.hpp"

namespace Utils
{
	std::string GetMimeType(std::string url)
	{
		wchar_t* mimeType = nullptr;
		FindMimeFromData(nullptr, std::wstring(url.begin(), url.end()).data(), nullptr, 0, nullptr, 0, &mimeType, 0);

		if (mimeType)
		{
			std::wstring wMimeType(mimeType);
			return std::string(wMimeType.begin(), wMimeType.end());
		}

		return "application/octet-stream";
	}

	std::string ParseChallenge(std::string data)
	{
		auto pos = data.find_first_of("\n ");
		if (pos == std::string::npos) return data;
		return data.substr(0, pos).data();
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

		Utils::Memory::Allocator allocator;
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

	size_t GetModuleSize(HMODULE module)
	{
		PIMAGE_DOS_HEADER header = PIMAGE_DOS_HEADER(module);
		PIMAGE_NT_HEADERS ntHeader = PIMAGE_NT_HEADERS(DWORD(module) + header->e_lfanew);
		return ntHeader->OptionalHeader.SizeOfImage;
	}

	bool HasIntercection(unsigned int base1, unsigned int len1, unsigned int base2, unsigned int len2)
	{
		return !((base1 + len1) < base2 || (base2 + len2) < base1);
	}
}
