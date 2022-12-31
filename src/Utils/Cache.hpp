#pragma once

namespace Utils
{
	class Cache
	{
	public:
		static const char* Urls[];

		static std::string GetUrl(const std::string& url, const std::string& path);
		static std::string GetFile(const std::string& path, int timeout = 5000, const std::string& useragent = "IW4x");

	private:
		static std::mutex CacheMutex;
		static std::string ValidUrl;
	};
}
