#include "STDInclude.hpp"

namespace Utils
{
	void InfoString::set(const std::string& key, const std::string& value)
	{
		this->keyValuePairs[key] = value;
	}

	std::string InfoString::get(const std::string& key)
	{
		if (this->keyValuePairs.find(key) != this->keyValuePairs.end())
		{
			return this->keyValuePairs[key];
		}

		return "";
	}

	void InfoString::parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		std::vector<std::string> KeyValues = Utils::String::Explode(buffer, '\\');

		for (unsigned int i = 0; i < (KeyValues.size() - 1); i += 2)
		{
			this->keyValuePairs[KeyValues[i]] = KeyValues[i + 1];
		}
	}

	std::string InfoString::build()
	{
		std::string infoString;

		bool first = true;

		for (auto i = this->keyValuePairs.begin(); i != this->keyValuePairs.end(); ++i)
		{
			if (first) first = false;
			else infoString.append("\\");

			infoString.append(i->first); // Key
			infoString.append("\\");
			infoString.append(i->second); // Value
		}

		return infoString;
	}

	void InfoString::dump()
	{
		for (auto i = this->keyValuePairs.begin(); i != this->keyValuePairs.end(); ++i)
		{
			OutputDebugStringA(Utils::String::VA("%s: %s", i->first.data(), i->second.data()));
		}
	}

	json11::Json InfoString::to_json()
	{
		return this->keyValuePairs;
	}
}
