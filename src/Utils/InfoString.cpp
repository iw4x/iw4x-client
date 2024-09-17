#include <STDInclude.hpp>
#include "InfoString.hpp"

namespace Utils
{
	InfoString::InfoString(const std::string& buffer)
	{
		this->parse(buffer);
	}

	void InfoString::set(const std::string& key, const std::string& value)
	{
		this->keyValuePairs_[key] = value;
	}

	void InfoString::remove(const std::string& key)
	{
		this->keyValuePairs_.erase(key);
	}

	std::string InfoString::get(const std::string& key) const
	{
		if (const auto value = this->keyValuePairs_.find(key); value != this->keyValuePairs_.end())
		{
			return value->second;
		}

		return {};
	}

	void InfoString::parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		const auto keyValues = String::Split(buffer, '\\');
		for (std::size_t i = 0; !keyValues.empty() && i < (keyValues.size() - 1); i += 2)
		{
			const auto& key = keyValues[i];
			const auto& value = keyValues[i + 1];

			if (!this->keyValuePairs_.contains(key))
			{
				this->keyValuePairs_[key] = value;
			}
		}
	}

	std::string InfoString::build() const
	{
		std::string infoString;
		for (const auto& [key, value] : this->keyValuePairs_)
		{
			infoString.append("\\");
			infoString.append(key);
			infoString.append("\\");
			infoString.append(value);
		}

		return infoString;
	}

#ifdef _DEBUG
	void InfoString::dump()
	{
		for (const auto& [key, value] : this->keyValuePairs_)
		{
			OutputDebugStringA(String::VA("%s: %s\n", key.data(), value.data()));
		}
	}
#endif

	nlohmann::json InfoString::to_json() const
	{
		return this->keyValuePairs_;
	}
}
