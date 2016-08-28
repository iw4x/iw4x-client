#include "STDInclude.hpp"

namespace Utils
{
	std::string GetMimeType(std::string url)
	{
		wchar_t* mimeType = nullptr;
		FindMimeFromData(NULL, std::wstring(url.begin(), url.end()).data(), NULL, 0, NULL, 0, &mimeType, 0);

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
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		std::string message(messageBuffer, size);

		OutputDebugStringA(Utils::String::VA("Last error code: 0x%08X (%s)\n", errorMessageID, message));

		LocalFree(messageBuffer);
	}
}
