#include "STDInclude.hpp"

#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		65536

namespace Utils
{
	const char *VA(const char *fmt, ...)
	{
		static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int g_vaNextBufferIndex = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = g_vaBuffer[g_vaNextBufferIndex];
		vsnprintf(g_vaBuffer[g_vaNextBufferIndex], VA_BUFFER_SIZE, fmt, ap);
		g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	std::string StrToLower(std::string input)
	{
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		return input;
	}

	bool EndsWith(std::string haystack, std::string needle)
	{
		return (strstr(haystack.data(), needle.data()) == (haystack.data() + haystack.size() - needle.size()));
	}

	// Complementary function for memset, which checks if a memory is set
	bool MemIsSet(void* mem, char chr, size_t length) 
	{
		char* memArr = reinterpret_cast<char*>(mem);

		for (size_t i = 0; i < length; i++)
		{
			if (memArr[i] != chr)
			{
				return false;
			}
		}

		return true;
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

	bool StartsWith(std::string haystack, std::string needle)
	{
		return (haystack.size() >= needle.size() && !strncmp(needle.data(), haystack.data(), needle.size()));
	}

	unsigned int OneAtATime(const char *key, size_t len)
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

	// trim from start
	std::string &LTrim(std::string &s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	std::string &RTrim(std::string &s) 
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	std::string &Trim(std::string &s)
	{
		return LTrim(RTrim(s));
	}

	std::string FormatTimeSpan(int milliseconds)
	{
		int secondsTotal = milliseconds / 1000;
		int seconds = secondsTotal % 60;
		int minutesTotal = secondsTotal / 60;
		int minutes = minutesTotal % 60;
		int hoursTotal = minutesTotal / 60;

		return Utils::VA("%02d:%02d:%02d", hoursTotal, minutes, seconds);
	}

	std::string ParseChallenge(std::string data)
	{
		// Ensure line break
		data.append("\n");
		return data.substr(0, data.find_first_of("\n")).data();
	}

	// TODO: Use modern file reading methods
	bool FileExists(std::string file)
	{
		FILE* fp;
		fopen_s(&fp, file.data(), "r");

		if (fp)
		{
			fclose(fp);
			return true;
		}

		return false;
	}

	void WriteFile(std::string file, std::string data)
	{
		std::ofstream stream(file, std::ios::binary);

		if (stream.is_open())
		{
			stream.write(data.data(), data.size());
			stream.close();
		}
	}

	std::string ReadFile(std::string file)
	{
		std::string buffer;

		if (FileExists(file))
		{
			std::ifstream stream(file, std::ios::binary);
			std::streamsize size = 0;

			stream.seekg(0, std::ios::end);
			size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				buffer.clear();
				buffer.resize((uint32_t)size);

				stream.read((char *)buffer.data(), size);
			}

			stream.close();
		}

		return buffer;
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

	namespace Message
	{
		void WriteBuffer(std::string& message, std::string buffer)
		{
			DWORD length = buffer.size();
			message.append(reinterpret_cast<char*>(&length), 4);
			message.append(buffer);
		}

		bool ReadBuffer(std::string& message, std::string& buffer)
		{
			if (message.size() < 4) return false;

			char* messagePtr = const_cast<char*>(message.data());

			DWORD length = *reinterpret_cast<DWORD*>(messagePtr);

			if (message.size() < (length + 4)) return false;

			buffer.clear();
			buffer.append(messagePtr + 4, length);

			message = std::string(messagePtr + 4 + length, message.size() - (4 + length));
			return true;
		}
	}
}