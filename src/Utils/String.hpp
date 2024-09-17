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

		VAProvider() : currentBuffer_(0) {}

		[[nodiscard]] const char* get(const char* format, va_list ap)
		{
			++this->currentBuffer_ %= ARRAY_COUNT(this->stringPool_);
			auto entry = &this->stringPool_[this->currentBuffer_];

			if (!entry->size_ || !entry->buffer_)
			{
				throw std::runtime_error("String pool not initialized");
			}

			while (true)
			{
				const auto res = vsnprintf_s(entry->buffer_, entry->size_, _TRUNCATE, format, ap);
				if (res > 0) break; // Success
				if (res == 0) return ""; // Error

				entry->doubleSize();
			}

			return entry->buffer_;
		}

	private:
		class Entry
		{
		public:
			Entry(std::size_t size = MinBufferSize) : size_(size), buffer_(nullptr)
			{
				if (this->size_ < MinBufferSize) this->size_ = MinBufferSize;
				this->allocate();
			}

			~Entry()
			{
				if (this->buffer_) Memory::GetAllocator()->free(this->buffer_);
				this->size_ = 0;
				this->buffer_ = nullptr;
			}

			void allocate()
			{
				if (this->buffer_) Memory::GetAllocator()->free(this->buffer_);
				this->buffer_ = Memory::GetAllocator()->allocateArray<char>(this->size_ + 1);
			}

			void doubleSize()
			{
				this->size_ *= 2;
				this->allocate();
			}

			std::size_t size_;
			char* buffer_;
		};

		std::size_t currentBuffer_;
		Entry stringPool_[Buffers];
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
		(SanitizeFormatArgs(args), ...);
		std::vformat(fmt, std::make_format_args(args...)).swap(vaBuffer);
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
