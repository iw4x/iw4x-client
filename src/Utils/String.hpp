#pragma once

template <class Type, std::size_t n>
constexpr auto ARRAY_COUNT(Type(&)[n]) { return n; }

namespace Utils::String
{
	template <std::size_t Buffers, std::size_t MinBufferSize>
	class VAProvider
	{
	public:
		static_assert(Buffers != 0 && MinBufferSize != 0, "Buffers and MinBufferSize mustn't be 0");

		VAProvider() : currentBuffer(0) {}
		~VAProvider() = default;

		[[nodiscard]] const char* get(const char* format, va_list ap)
		{
			++this->currentBuffer %= ARRAY_COUNT(this->stringPool);
			auto entry = &this->stringPool[this->currentBuffer];

			if (!entry->size || !entry->buffer)
			{
				throw std::runtime_error("String pool not initialized");
			}

			while (true)
			{
				const auto res = vsnprintf_s(entry->buffer, entry->size, _TRUNCATE, format, ap);
				if (res > 0) break; // Success
				if (res == 0) return ""; // Error

				entry->doubleSize();
			}

			return entry->buffer;
		}

	private:
		class Entry
		{
		public:
			Entry(std::size_t _size = MinBufferSize) : size(_size), buffer(nullptr)
			{
				if (this->size < MinBufferSize) this->size = MinBufferSize;
				this->allocate();
			}

			~Entry()
			{
				if (this->buffer) Memory::GetAllocator()->free(this->buffer);
				this->size = 0;
				this->buffer = nullptr;
			}

			void allocate()
			{
				if (this->buffer) Memory::GetAllocator()->free(this->buffer);
				this->buffer = Memory::GetAllocator()->allocateArray<char>(this->size + 1);
			}

			void doubleSize()
			{
				this->size *= 2;
				this->allocate();
			}

			std::size_t size;
			char* buffer;
		};

		std::size_t currentBuffer;
		Entry stringPool[Buffers];
	};

	template <typename Arg> // This should display a nice "nullptr" instead of a number
	static void SanitizeFormatArgs(Arg& arg)
	{
		if constexpr (std::is_same_v<Arg, char*> || std::is_same_v<Arg, const char*>)
		{
			if (arg == nullptr)
			{
				arg = const_cast<char*>("nullptr");
			}
		}
	}

	[[nodiscard]] const char* VA(const char* fmt, ...);

	template <typename... Args>
	[[nodiscard]] const char* Format(std::string_view fmt, Args&&... args)
	{
		static thread_local std::string vaBuffer;
		vaBuffer.clear();

		(SanitizeFormatArgs(args), ...);
		std::vformat_to(std::back_inserter(vaBuffer), fmt, std::make_format_args(args...));
		return vaBuffer.data();
	}

	[[nodiscard]] std::string ToLower(const std::string& text);
	[[nodiscard]] std::string ToUpper(const std::string& text);

	template <class OutputIter>
	[[nodiscard]] OutputIter ApplyToLower(OutputIter container)
	{
		OutputIter result;
		std::ranges::transform(container, std::back_inserter(result), [](const std::string& s) -> std::string
		{
			return ToLower(s);
		});

		return result;
	}

	template <class OutputIter>
	[[nodiscard]] OutputIter ApplyToUpper(OutputIter container)
	{
		OutputIter result;
		std::ranges::transform(container, std::back_inserter(result), [](const std::string& s) -> std::string
		{
			return ToUpper(s);
		});

		return result;
	}

	[[nodiscard]] bool Compare(const std::string& lhs, const std::string& rhs);

	[[nodiscard]] std::vector<std::string> Split(const std::string& str, char delim);
	void Replace(std::string& str, const std::string& from, const std::string& to);

	[[nodiscard]] bool StartsWith(const std::string& haystack, const std::string& needle);
	[[nodiscard]] bool EndsWith(const std::string& haystack, const std::string& needle);

	[[nodiscard]] bool IsNumber(const std::string& str);

	std::string& LTrim(std::string& str);
	std::string& RTrim(std::string& str);
	void Trim(std::string& str);

	[[nodiscard]] std::string Convert(const std::wstring& wstr);
	[[nodiscard]] std::wstring Convert(const std::string& str);

	[[nodiscard]] std::string FormatTimeSpan(int milliseconds);
	[[nodiscard]] std::string FormatBandwidth(std::size_t bytes, int milliseconds);

	[[nodiscard]] std::string DumpHex(const std::string& data, const std::string& separator = " ");

	[[nodiscard]] std::string XOR(std::string str, char value);

	[[nodiscard]] std::string EncodeBase64(const char* input, unsigned long inputSize);
	[[nodiscard]] std::string EncodeBase64(const std::string& input);

	[[nodiscard]] std::string EncodeBase128(const std::string& input);
}
