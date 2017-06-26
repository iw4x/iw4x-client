#pragma once

#define CHUNK 16384

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

		class ZStd
		{
		public:
			static std::string Compress(std::string data);
			static std::string Decompress(std::string data);
		};
	};
}
