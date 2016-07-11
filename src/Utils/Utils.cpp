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
}
