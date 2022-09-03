#include <STDInclude.hpp>

namespace Utils
{
	const char* Cache::Urls[] =
	{
		"https://xlabs.dev",
		"https://raw.githubusercontent.com/XLabsProject/iw4x-client"
	};

	std::string Cache::ValidUrl;
	std::mutex Cache::CacheMutex;

	std::string Cache::GetStaticUrl(const std::string& path)
	{
		return Cache::Urls[0] + path;
	}

	std::string Cache::GetUrl(const std::string& url, const std::string& path)
	{
		return url + path;
	}

	std::string Cache::GetFile(const std::string& path, int timeout, const std::string& useragent)
	{
		std::lock_guard<std::mutex> _(Cache::CacheMutex);

		if (Cache::ValidUrl.empty())
		{
			InternetSetCookieA("https://onion.casa", "disclaimer_accepted", "1");
			InternetSetCookieA("https://hiddenservice.net", "disclaimer_accepted", "1");

			for (int i = 0; i < ARRAYSIZE(Cache::Urls); ++i)
			{
				std::string result = Utils::WebIO(useragent, Cache::GetUrl(Cache::Urls[i], path)).setTimeout(timeout)->get();

				if (!result.empty())
				{
					Cache::ValidUrl = Cache::Urls[i];
					return result;
				}
			}

			return "";
		}
		else
		{
			return Utils::WebIO(useragent, Cache::GetUrl(Cache::ValidUrl, path)).setTimeout(timeout)->get();
		}
	}
}
