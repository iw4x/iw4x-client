#pragma once

namespace Utils
{
	namespace String
	{
		class VAProvider
		{
		public:
			VAProvider(size_t buffers = 8);

			char* get(const char* format, va_list ap);

		private:
			size_t currentBuffer;
			std::vector<std::pair<size_t,char*>> stringBuffers;

			Utils::Memory::Allocator allocator;
		};

		const char *VA(const char *fmt, ...);

		int IsSpace(int c);
		std::string ToLower(std::string input);
		std::string ToUpper(std::string input);
		bool EndsWith(std::string haystack, std::string needle);
		std::vector<std::string> Explode(const std::string& str, char delim);
		void Replace(std::string &string, std::string find, std::string replace);
		bool StartsWith(std::string haystack, std::string needle);
		std::string &LTrim(std::string &s);
		std::string &RTrim(std::string &s);
		std::string &Trim(std::string &s);

		std::string FormatTimeSpan(int milliseconds);
		std::string FormatBandwidth(size_t bytes, int milliseconds);

		std::string DumpHex(std::string data, std::string separator = " ");

		std::string XOR(std::string str, char value);

		std::string EncodeBase64(const char* input, const unsigned long inputSize);
		std::string EncodeBase64(const std::string& input);

		std::string EncodeBase128(const std::string& input);

		std::string GenerateUUIDString();
	}
}
