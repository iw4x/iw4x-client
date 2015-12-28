#include "..\STDInclude.hpp"

#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		4096

namespace Utils
{
	const char *VA(const char *fmt, ...)
	{
		static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int g_vaNextBufferIndex = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = g_vaBuffer[g_vaNextBufferIndex];
		vsprintf_s(g_vaBuffer[g_vaNextBufferIndex], fmt, ap);
		g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	std::string StrToLower(std::string input)
	{
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		return input;
	}

	bool EndsWith(const char* heystack, const char* needle)
	{
		return (strstr(heystack, needle) == (heystack + strlen(heystack) - strlen(needle)));
	}

	std::vector<std::string> Explode(const std::string& str, char delim)
	{
		std::vector<std::string> result;
		std::istringstream iss(str);

		for (std::string token; std::getline(iss, token, delim);)
		{
			std::string _entry = std::move(token);

			// Remove trailing 0x0 bytes
			while (_entry.size() && !_entry[_entry.size() - 1])
			{
				_entry = _entry.substr(0, _entry.size() - 1);
			}

			result.push_back(_entry);
		}

		return result;
	}

	void Replace(std::string &string, std::string find, std::string replace)
	{
		size_t nPos = 0;

		while ((nPos = string.find(find, nPos)) != std::string::npos)
		{
			string = string.replace(nPos, find.length(), replace);
			nPos += replace.length();
		}
	}

	unsigned int OneAtATime(char *key, size_t len)
	{
		unsigned int hash, i;
		for (hash = i = 0; i < len; ++i)
		{
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

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

	std::string InfoString::Build()
	{
		std::string infoString;

		bool first = true;

		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); i++)
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
		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); i++)
		{
			OutputDebugStringA(Utils::VA("%s: %s", i->first.data(), i->second.data()));
		}
	}

	void InfoString::Parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		std::vector<std::string> KeyValues = Utils::Explode(buffer, '\\');

		for (unsigned int i = 0; i < (KeyValues.size() - 1); i+=2)
		{
			this->KeyValuePairs[KeyValues[i]] = KeyValues[i + 1];
		}
	}
}