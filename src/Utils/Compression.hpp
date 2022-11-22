#pragma once

#define CHUNK 16384
#define DEFLATE_ZLIB false
#define DEFLATE_ZSTD true

namespace Utils::Compression
{
	class ZLib
	{
	public:
		static std::string Compress(const std::string& data);
		static std::string Decompress(const std::string& data);
	};
}
