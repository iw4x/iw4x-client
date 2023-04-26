#include <STDInclude.hpp>
#ifdef ENABLE_BASE128
#include "base128.h"
#endif

namespace Utils::String
{
	const char* VA(const char* fmt, ...)
	{
		static VAProvider<4, 256> globalProvider;
		static thread_local VAProvider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result;
		if (Components::Loader::IsUninitializing()) result = globalProvider.get(fmt, ap);
		else result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::string ToLower(const std::string& text)
	{
		std::string result;
		std::ranges::transform(text, std::back_inserter(result), [](const unsigned char input) -> char
		{
			return static_cast<char>(std::tolower(input));
		});

		return result;
	}

	std::string ToUpper(const std::string& text)
	{
		std::string result;
		std::ranges::transform(text, std::back_inserter(result), [](const unsigned char input) -> char
		{
			return static_cast<char>(std::toupper(input));
		});

		return result;
	}

	bool Compare(const std::string& lhs, const std::string& rhs)
	{
		return std::ranges::equal(lhs, rhs, [](const unsigned char a, const unsigned char b) -> bool
		{
			return std::tolower(a) == std::tolower(b);
		});
	}

	std::string DumpHex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (std::size_t i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(VA("%02X", data[i] & 0xFF));
		}

		return result;
	}

	std::string XOR(std::string str, char value)
	{
		for (std::size_t i = 0; i < str.size(); ++i)
		{
			str[i] ^= static_cast<unsigned char>(value);
		}

		return str;
	}

	std::vector<std::string> Split(const std::string& str, const char delim)
	{
		std::stringstream ss(str);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from S1x)
		}

		return elems;
	}

	void Replace(std::string& str, const std::string& from, const std::string& to)
	{
		std::size_t nPos = 0;

		while ((nPos = str.find(from, nPos)) != std::string::npos)
		{
			str = str.replace(nPos, from.length(), to);
			nPos += to.length();
		}
	}

	bool StartsWith(const std::string& haystack, const std::string& needle)
	{
		return haystack.find(needle) == 0; // If the pos of the first found char is 0, string starts with 'needle'
	}

	bool EndsWith(const std::string& haystack, const std::string& needle)
	{
		if (needle.size() > haystack.size()) return false;
		return std::equal(needle.rbegin(), needle.rend(), haystack.rbegin());
	}

	bool IsNumber(const std::string& str)
	{
		return !str.empty() && std::find_if(str.begin(), str.end(), [](unsigned char input)
		{
			return !std::isdigit(input);
		}) == str.end();
	}

	// Trim from start
	std::string& LTrim(std::string& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](const unsigned char input)
		{
			return !std::isspace(input);
		}));

		return str;
	}

	// Trim from end
	std::string& RTrim(std::string& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](const  unsigned char input)
		{
			return !std::isspace(input);
		}).base(), str.end());

		return str;
	}

	// Trim from both ends
	void Trim(std::string& str)
	{
		LTrim(RTrim(str));
	}

	std::string Convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring Convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}

	std::string FormatTimeSpan(int milliseconds)
	{
		int secondsTotal = milliseconds / 1000;
		int seconds = secondsTotal % 60;
		int minutesTotal = secondsTotal / 60;
		int minutes = minutesTotal % 60;
		int hoursTotal = minutesTotal / 60;

		return VA("%02d:%02d:%02d", hoursTotal, minutes, seconds);
	}

	std::string FormatBandwidth(std::size_t bytes, int milliseconds)
	{
		static const char* sizes[] =
		{
			"B",
			"KB",
			"MB",
			"GB",
			"TB",
		};

		if (!milliseconds) return "0.00 B/s";

		double bytesPerSecond = (1000.0 / milliseconds) * bytes;

		std::size_t i;
		for (i = 0; bytesPerSecond > 1000 && i < ARRAYSIZE(sizes); ++i) // 1024 or 1000?
		{
			bytesPerSecond /= 1000;
		}

		return VA("%.2f %s/s", static_cast<float>(bytesPerSecond), sizes[i]);
	}

#ifdef ENABLE_BASE64
	// Encodes a given string in Base64
	std::string EncodeBase64(const char* input, const unsigned long inputSize)
	{
		unsigned long outlen = long(inputSize + (inputSize / 3.0) + 16);
		unsigned char* outbuf = new unsigned char[outlen]; //Reserve output memory
		base64_encode(reinterpret_cast<unsigned char*>(const_cast<char*>(input)), inputSize, outbuf, &outlen);
		std::string ret(reinterpret_cast<char*>(outbuf), outlen);
		delete[] outbuf;
		return ret;
	}

	// Encodes a given string in Base64
	std::string EncodeBase64(const std::string& input)
	{
		return EncodeBase64(input.data(), input.size());
	}
#endif

#ifdef ENABLE_BASE128
	// Encodes a given string in Base128
	std::string EncodeBase128(const std::string& input)
	{
		base128 encoder;

		void* inbuffer = const_cast<char*>(input.data());
		char* buffer = encoder.encode(inbuffer, input.size());
		/*
		Interesting to see that the buffer returned by the encoder is not a standalone string copy
		but instead is a pointer to the internal "encoded" field of the encoder. So if you deinitialize
		the encoder that string will probably become garbage.
		*/
		std::string retval(buffer);
		return retval;
	}
#endif
}
