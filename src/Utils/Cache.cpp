#include "STDInclude.hpp"

namespace Utils
{
	const char* Cache::urls[] =
	{
		"https://iw4xcachep26muba.onion.to"/*,
		"https://iw4xcachejnetuln.onion.to",
		"https://iw4xcachedjodc4y.onion.to",
		*/
	};
	std::string Cache::validUrl;
	std::mutex Cache::CacheMutex;

	Cache::Cache(std::string path)
	{
		this->Path = path;
	}

	std::string Cache::GetUrl()
	{
		if (Cache::validUrl.empty())
			return Cache::urls[0] + this->Path;
		else
			return Cache::validUrl + this->Path;
	}

	std::string Cache::GetUrl(std::string url, std::string path)
	{
		return url + path;
	}

	std::string Cache::GetFile(int timeout, std::string useragent)
	{
		if (Cache::validUrl.empty())
		{
			std::lock_guard<std::mutex> _(Cache::CacheMutex);
			for (int i = 0; i < ARRAY_SIZE(Cache::urls); i++)
			{
				std::string result = Utils::WebIO(useragent, this->GetUrl(Cache::urls[i], this->Path)).setTimeout(timeout)->get();
				if (!result.empty())
				{
					Cache::validUrl = Cache::urls[i];
					return result;
				}
			}
		}
		else
		{
			return Utils::WebIO(useragent, this->GetUrl(Cache::validUrl, this->Path)).setTimeout(timeout)->get();
		}
	}
}
