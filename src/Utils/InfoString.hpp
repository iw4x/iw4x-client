#pragma once

namespace Utils
{
	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) : InfoString() { this->parse(buffer); };
		InfoString(const InfoString &obj) : keyValuePairs(obj.keyValuePairs) {};

		void set(std::string key, std::string value);
		std::string get(std::string key);

		std::string build();

		void dump();

		json11::Json to_json();

	private:
		std::map<std::string, std::string> keyValuePairs;
		void parse(std::string buffer);
	};
}
