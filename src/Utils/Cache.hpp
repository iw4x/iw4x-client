namespace Utils
{
	class Cache
	{
	public:
		Cache(std::string path);
		std::string GetUrl();
		std::string GetFile(int timeout = 5000, std::string useragent = "IW4x");

	private:
		static std::mutex CacheMutex;
		static const char* urls[];
		static std::string validUrl;
		std::string Path;
		std::string GetUrl(std::string url, std::string path);
	};
}
