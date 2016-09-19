namespace Utils
{
	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) : InfoString() { this->Parse(buffer); };
		InfoString(const InfoString &obj) : KeyValuePairs(obj.KeyValuePairs) {};

		void Set(std::string key, std::string value);
		std::string Get(std::string key);

		std::string Build();

		void Dump();

		json11::Json to_json();

	private:
		std::map<std::string, std::string> KeyValuePairs;
		void Parse(std::string buffer);
	};
}
