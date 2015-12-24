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

	std::vector<std::string> Explode(const std::string& str, const std::string& delimiters)
	{
		std::vector<std::string> tokens;

		auto subStrBeginPos = str.find_first_not_of(delimiters, 0);
		auto subStrEndPos = str.find_first_of(delimiters, subStrBeginPos);

		while (std::string::npos != subStrBeginPos || std::string::npos != subStrEndPos)
		{
			tokens.push_back(str.substr(subStrBeginPos, subStrEndPos - subStrBeginPos));

			subStrBeginPos = str.find_first_not_of(delimiters, subStrEndPos);
			subStrEndPos = str.find_first_of(delimiters, subStrBeginPos);
		}

		return tokens;
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

	void InfoString::Parse(std::string buffer)
	{
		std::vector<std::string> KeyValues = Utils::Explode(buffer, "\\");

		for (unsigned int i = 0; i < (KeyValues.size() - 1); i+=2)
		{
			this->KeyValuePairs[KeyValues[i]] = KeyValues[i + 1];
		}
	}
}