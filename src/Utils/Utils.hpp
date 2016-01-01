namespace Utils
{
	const char *VA(const char *fmt, ...);
	std::string StrToLower(std::string input);
	bool EndsWith(const char* heystack, const char* needle);
	std::vector<std::string> Explode(const std::string& str, char delim);
	void Replace(std::string &string, std::string find, std::string replace);
	unsigned int OneAtATime(char *key, size_t len);
	std::string &LTrim(std::string &s);
	std::string &RTrim(std::string &s);
	std::string &Trim(std::string &s);

	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) :InfoString() { this->Parse(buffer); };
		InfoString(const InfoString &obj) { this->KeyValuePairs = obj.KeyValuePairs; };

		void Set(std::string key, std::string value);
		std::string Get(std::string key);

		std::string Build();

		void Dump();

	private:
		std::map<std::string, std::string> KeyValuePairs;
		void Parse(std::string buffer);
	};

	template <typename T> void Merge(std::vector<T> &target, std::vector<T> &source)
	{
		for (auto &entry : source)
		{
			target.push_back(entry);
		}
	}
}
