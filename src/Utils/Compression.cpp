#include "STDInclude.hpp"

namespace Utils
{
	namespace Compression
	{
		std::string ZLib::Compress(std::string data)
		{
			z_stream stream;
			ZeroMemory(&stream, sizeof(stream));

			char* buffer = new char[data.size() * 2];

			if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK) 
			{ 
				delete[] buffer;
				return "";
			}

			stream.next_out = reinterpret_cast<uint8_t*>(buffer);
			stream.next_in = reinterpret_cast<const uint8_t*>(data.data());
			stream.avail_out = data.size() * 2;
			stream.avail_in = data.size();

			if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
			{ 
				delete[] buffer;
				return "";
			}

			if (deflateEnd(&stream) != Z_OK)
			{
				delete[] buffer;
				return "";
			}

			data.clear();
			data.append(buffer, stream.total_out);

			delete[] buffer;

			return data;
		}

		std::string ZLib::Decompress(std::string data)
		{
			z_stream stream;
			ZeroMemory(&stream, sizeof(stream));
			std::string buffer;

			if (inflateInit(&stream) != Z_OK)
			{
				return buffer;
			}

			int ret = 0;
			uint8_t dest[CHUNK] = { 0 };
			const char* dataPtr = data.data();

			do 
			{
				stream.avail_in = min(CHUNK, data.size() - (dataPtr - data.data()));
				stream.next_in = reinterpret_cast<const uint8_t*>(dataPtr);

				do 
				{
					stream.avail_out = CHUNK;
					stream.next_out = dest;

					ret = inflate(&stream, Z_NO_FLUSH);
					if (ret == Z_STREAM_ERROR)
					{
						inflateEnd(&stream);
					}
					
					buffer.append(reinterpret_cast<const char*>(dest), CHUNK - stream.avail_out);

				} while (stream.avail_out == 0);

			} while (ret != Z_STREAM_END);

			inflateEnd(&stream);

			return buffer;
		}
	};
}
