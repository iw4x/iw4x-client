#pragma once

namespace Utils
{
	class Entities
	{
	public:
		Entities() = default;
		Entities(const std::string& buffer) : Entities() { this->parse(buffer); }
		Entities(const char* string, std::size_t lenPlusOne) : Entities(std::string(string, lenPlusOne - 1)) {}
		Entities(const Entities& obj) = default;

		[[nodiscard]] std::string build() const;

		std::vector<std::string> getModels();
		void deleteTriggers();
		void deleteWeapons(bool keepTurrets);
		void convertTurrets();

	private:
		enum
		{
			PARSE_AWAIT_KEY,
			PARSE_READ_KEY,
			PARSE_AWAIT_VALUE,
			PARSE_READ_VALUE,
		};

		std::vector<std::unordered_map<std::string, std::string>> entities;
		void parse(const std::string& buffer);
	};
}
