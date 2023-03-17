#include <STDInclude.hpp>
#include "WebIO.hpp"

namespace Utils
{
	const char* Cache::Urls[] =
	{
		"https://raw.githubusercontent.com/XLabsProject/iw4x-client",
		"https://xlabs.dev",
	};

	std::string Cache::ValidUrl;
	std::mutex Cache::CacheMutex;

	std::string Cache::GetUrl(const std::string& url, const std::string& path)
	{
		return url + path;
	}

	std::string Cache::GetFile(const std::string& path, int timeout, const std::string& useragent)
	{
		std::lock_guard _(CacheMutex);

		if (ValidUrl.empty())
		{
			for (std::size_t i = 0; i < ARRAYSIZE(Urls); ++i)
			{
				std::string result = WebIO(useragent, GetUrl(Urls[i], path)).setTimeout(timeout)->get();

				if (!result.empty())
				{
					ValidUrl = Urls[i];
					return result;
				}
			}

			return {};
		}

		return WebIO(useragent, GetUrl(ValidUrl, path)).setTimeout(timeout)->get();
	}
}
