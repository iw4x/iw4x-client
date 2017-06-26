#pragma once

#define CHUNK 16384
#define DEFLATE_ZLIB false
#define DEFLATE_ZSTD true

namespace Utils
{
	namespace Compression
	{
		class Deflate
		{
		public:
			class ZLib
			{
			public:
				static std::string Compress(std::string data);
				static std::string Decompress(std::string data);
			};

			class ZStd
			{
			public:
				static std::string Compress(std::string data);
				static std::string Decompress(std::string data);
			};

			class Semaphore
			{
			public:
				Semaphore(bool zstd);
				~Semaphore();

			private:
				int state;
			};

		private:
			static std::mutex Mutex;
			static std::string Compress(std::string data);
			static std::string Decompress(std::string data);
		};
	};
}
