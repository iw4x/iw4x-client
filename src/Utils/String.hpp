namespace Utils
{
	namespace String
	{
		template <typename ...Args>
		const char* VA(std::string message, Args && ...args)
		{
			#define VA_BUFFER_COUNT	4
			#define VA_BUFFER_SIZE	65536

			static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
			static int g_vaNextBufferIndex = 0;

			char* buffer = g_vaBuffer[g_vaNextBufferIndex];
			std::string str = fmt::sprintf(message, std::forward<Args>(args)...);
			strncpy_s(g_vaBuffer[g_vaNextBufferIndex], str.data(), VA_BUFFER_SIZE);
			g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;

			return buffer;
		}

		std::string StrToLower(std::string input);
		std::string StrToUpper(std::string input);
		bool EndsWith(std::string haystack, std::string needle);
		std::vector<std::string> Explode(const std::string& str, char delim);
		void Replace(std::string &string, std::string find, std::string replace);
		bool StartsWith(std::string haystack, std::string needle);
		std::string &LTrim(std::string &s);
		std::string &RTrim(std::string &s);
		std::string &Trim(std::string &s);

		std::string FormatTimeSpan(int milliseconds);

		std::string DumpHex(std::string data, std::string separator = " ");

		std::string XORString(std::string str, char value);
	}
}
