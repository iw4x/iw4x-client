#pragma once

namespace Utils
{
	class InfoString
	{
	public:
		InfoString() = default;
		explicit InfoString(const std::string& buffer);

		void set(const std::string& key, const std::string& value);
		void remove(const std::string& key);

		[[nodiscard]] std::string get(const std::string& key) const;
		[[nodiscard]] std::string build() const;

#ifdef _DEBUG
		void dump();
#endif

		[[nodiscard]] nlohmann::json to_json() const;

	private:
		std::unordered_map<std::string, std::string> keyValuePairs_;

		void parse(std::string buffer);
	};
}
