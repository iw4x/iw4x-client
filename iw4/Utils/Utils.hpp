namespace Utils
{
	const char *VA(const char *fmt, ...);
	std::string StrToLower(std::string input);
	bool EndsWith(const char* heystack, const char* needle);
	std::vector<std::string> Explode(const std::string& str, const std::string& delimiters);
	void Replace(std::string &string, std::string find, std::string replace);

	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) :InfoString() { this->Parse(buffer); };

		void Set(std::string key, std::string value);
		std::string Get(std::string key);

		std::string Build();

	private:
		std::map<std::string, std::string> KeyValuePairs;
		void Parse(std::string buffer);
	};
}
