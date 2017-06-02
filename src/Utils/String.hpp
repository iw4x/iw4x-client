#pragma once

namespace Utils
{
	namespace String
	{
		template <size_t Buffers, size_t MinBufferSize>
		class VAProvider
		{
		public:
			typename std::enable_if<(Buffers != 0 && MinBufferSize != 0), char*>::type
			get(const char* format, va_list ap)
			{
				std::lock_guard<std::mutex> _(this->accessMutex);

				auto threadBuffers = this->stringBuffers.find(std::this_thread::get_id());
				if (threadBuffers == this->stringBuffers.end())
				{
					this->stringBuffers[std::this_thread::get_id()] = Pool();
					threadBuffers = this->stringBuffers.find(std::this_thread::get_id());
				}

				if (!threadBuffers->second.stringPool.size()) threadBuffers->second.stringPool.resize(Buffers);

				++threadBuffers->second.currentBuffer %= threadBuffers->second.stringPool.size();
				auto& entry = threadBuffers->second.stringPool[threadBuffers->second.currentBuffer];

				if (!entry.size || !entry.buffer)
				{
					entry = Entry(MinBufferSize);
				}

				while (true)
				{
					int res = vsnprintf_s(entry.buffer, entry.size, _TRUNCATE, format, ap);
					if (res > 0) break; // Success
					if (res == 0) return ""; // Error

					entry.doubleSize();
				}

				return entry.buffer;
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
					if(this->buffer) Utils::Memory::GetAllocator()->free(this->buffer);
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

			class Pool
			{
			public:
				Pool() : currentBuffer(0)
				{
					this->stringPool.resize(Buffers);
				}

				size_t currentBuffer;
				std::vector<Entry> stringPool;
			};

			std::mutex accessMutex;
			std::unordered_map<std::thread::id, Pool> stringBuffers;
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
