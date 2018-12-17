#include "STDInclude.hpp"

namespace Utils
{
	const char* Cache::Urls[] =
	{
		"https://iw4xcachep26muba.onion.to",
		"https://iw4xcachep26muba.tor2web.xyz",
		"https://iw4xcachep26muba.onion.ws",
		"https://iw4xcachep26muba.onion.sh",
		"https://iw4xcachep26muba.onion.pet",

		// Links below are dead
		// Still, let's keep them in case they come back
		"https://iw4xcachep26muba.onion.rip",
		"https://iw4xcachep26muba.onion.nu",
		"https://iw4xcachep26muba.onion.guide",
		"https://iw4xcachep26muba.onion.casa",
		"https://iw4xcachep26muba.hiddenservice.net",
		"https://iw4xcachep26muba.onion.cab",
		"https://iw4xcachep26muba.onion.link",

		// Not registered yet
		//"https://iw4xcachejnetuln.onion.to",
		//"https://iw4xcachedjodc4y.onion.to",
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

	void Cache::Uninitialize()
	{
		std::lock_guard<std::mutex> _(Cache::CacheMutex);
		Cache::ValidUrl.clear();
	}
}
