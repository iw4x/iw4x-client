#include "STDInclude.hpp"

namespace Utils
{
	// Infostring class
	void InfoString::Set(std::string key, std::string value)
	{
		this->KeyValuePairs[key] = value;
	}

	std::string InfoString::Get(std::string key)
	{
		if (this->KeyValuePairs.find(key) != this->KeyValuePairs.end())
		{
			return this->KeyValuePairs[key];
		}

		return "";
	}

	void InfoString::Parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		std::vector<std::string> KeyValues = Utils::String::Explode(buffer, '\\');

		for (unsigned int i = 0; i < (KeyValues.size() - 1); i += 2)
		{
			this->KeyValuePairs[KeyValues[i]] = KeyValues[i + 1];
		}
	}

	std::string InfoString::Build()
	{
		std::string infoString;

		bool first = true;

		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); ++i)
		{
			if (first) first = false;
			else infoString.append("\\");

			infoString.append(i->first); // Key
			infoString.append("\\");
			infoString.append(i->second); // Value
		}

		return infoString;
	}

	void InfoString::Dump()
	{
		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); ++i)
		{
			OutputDebugStringA(Utils::String::VA("%s: %s", i->first.data(), i->second.data()));
		}
	}

	json11::Json InfoString::to_json()
	{
		return this->KeyValuePairs;
	}
}
