#pragma once

namespace Utils
{
	namespace String
	{
		template <size_t Buffers, size_t MinBufferSize>
		class VAProvider
		{
		public:
			static_assert(Buffers != 0 && MinBufferSize != 0, "Buffers and MinBufferSize mustn't be 0");

			VAProvider() : currentBuffer(0) {}
			~VAProvider() {}

			const char* get(const char* format, va_list ap)
			{
				++this->currentBuffer %= ARRAYSIZE(this->stringPool);
				auto entry = &this->stringPool[this->currentBuffer];

				if (!entry->size || !entry->buffer)
				{
					throw std::runtime_error("String pool not initialized");
				}

				while (true)
				{
					int res = vsnprintf_s(entry->buffer, entry->size, _TRUNCATE, format, ap);
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
				Entry(size_t _size = MinBufferSize) : size(_size), buffer(nullptr)
				{
					if (this->size < MinBufferSize) this->size = MinBufferSize;
					this->allocate();
				}

				~Entry()
				{
					if (this->buffer) Utils::Memory::GetAllocator()->free(this->buffer);
					this->size = 0;
					this->buffer = nullptr;
				}

				void allocate()
				{
					if (this->buffer) Utils::Memory::GetAllocator()->free(this->buffer);
					this->buffer = Utils::Memory::GetAllocator()->allocateArray<char>(this->size + 1);
				}

				void doubleSize()
				{
					this->size *= 2;
					this->allocate();
				}

				size_t size;
				char* buffer;
			};

			size_t currentBuffer;
			Entry stringPool[Buffers];
		};

		const char *VA(const char *fmt, ...);

		int IsSpace(int c);
		std::string ToLower(std::string input);
		std::string ToUpper(std::string input);
		bool EndsWith(const std::string& haystack, const std::string& needle);
		std::vector<std::string> Explode(const std::string& str, char delim);
		void Replace(std::string &string, const std::string& find, const std::string& replace);
		bool StartsWith(const std::string& haystack, const std::string& needle);
		std::string &LTrim(std::string &s);
		std::string &RTrim(std::string &s);
		std::string &Trim(std::string &s);

		std::string FormatTimeSpan(int milliseconds);
		std::string FormatBandwidth(size_t bytes, int milliseconds);

		std::string DumpHex(const std::string& data, const std::string& separator = " ");

		std::string XOR(const std::string str, char value);

		std::string EncodeBase64(const char* input, const unsigned long inputSize);
		std::string EncodeBase64(const std::string& input);

		std::string EncodeBase128(const std::string& input);
	}
}
