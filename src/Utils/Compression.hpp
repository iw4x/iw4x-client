#pragma once

#define CHUNK 16384
#define DEFLATE_ZLIB false
#define DEFLATE_ZSTD true

namespace Utils
{
	namespace Compression
	{
		class ZLib
		{
		public:
			static std::string Compress(std::string data);
			static std::string Decompress(std::string data);
		};
	};
}
