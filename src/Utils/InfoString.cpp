#include <STDInclude.hpp>

namespace Utils
{
	void InfoString::set(const std::string& key, const std::string& value)
	{
		this->keyValuePairs[key] = value;
	}

	std::string InfoString::get(const std::string& key)
	{
		const auto value = this->keyValuePairs.find(key);
		if (value != this->keyValuePairs.end())
		{
			return value->second;
		}

		return "";
	}

	void InfoString::parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		auto KeyValues = Utils::String::Split(buffer, '\\');

		for (size_t i = 0; !KeyValues.empty() && i < (KeyValues.size() - 1); i += 2)
		{
			const auto& key = KeyValues[i];
			const auto& value = KeyValues[i + 1];
			this->keyValuePairs[key] = value;
		}
	}

	std::string InfoString::build()
	{
		std::string infoString;

		auto first = true;

		for (const auto& [key, value] : this->keyValuePairs)
		{
			if (first) first = false;
			else infoString.append("\\");

			infoString.append(key);
			infoString.append("\\");
			infoString.append(value);
		}

		return infoString;
	}

	void InfoString::dump()
	{
		for (const auto& [key, value] : this->keyValuePairs)
		{
			OutputDebugStringA(Utils::String::VA("%s: %s\n", key.data(), value.data()));
		}
	}

	json11::Json InfoString::to_json()
	{
		return this->keyValuePairs;
	}
}
