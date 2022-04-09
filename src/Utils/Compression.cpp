#include <STDInclude.hpp>

namespace Utils
{
	namespace Compression
	{
		std::string ZLib::Compress(const std::string& data)
		{
			Utils::Memory::Allocator allocator;
			unsigned long length = (data.size() * 2);
			if (!length) length = 2;

			// Make sure the buffer is large enough
			if (length < 100) length *= 10;

			char* buffer = allocator.allocateArray<char>(length);
			if (compress2(reinterpret_cast<Bytef*>(buffer), &length, reinterpret_cast<Bytef*>(const_cast<char*>(data.data())), data.size(), Z_BEST_COMPRESSION) != Z_OK)
			{
				return "";
			}

			return std::string(buffer, length);
		}

		std::string ZLib::Decompress(const std::string& data)
		{
			z_stream stream;
			ZeroMemory(&stream, sizeof(stream));
			std::string buffer;

			if (inflateInit(&stream) != Z_OK)
			{
				return "";
			}

			int ret;
			Utils::Memory::Allocator allocator;

			uint8_t* dest = allocator.allocateArray<uint8_t>(CHUNK);
			const char* dataPtr = data.data();

			do
			{
				stream.avail_in = std::min(static_cast<size_t>(CHUNK), data.size() - (dataPtr - data.data()));
				stream.next_in = reinterpret_cast<const uint8_t*>(dataPtr);
				dataPtr += stream.avail_in;

				do
				{
					stream.avail_out = CHUNK;
					stream.next_out = dest;

					ret = inflate(&stream, Z_NO_FLUSH);
					if (ret != Z_OK && ret != Z_STREAM_END)
					{
						inflateEnd(&stream);
						return "";
					}

					buffer.append(reinterpret_cast<const char*>(dest), CHUNK - stream.avail_out);

				} while (stream.avail_out == 0);

			} while (ret != Z_STREAM_END);

			inflateEnd(&stream);
			return buffer;
		}
	}
}
