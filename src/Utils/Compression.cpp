#include "STDInclude.hpp"

namespace Utils
{
	namespace Compression
	{
		std::string ZLib::Compress(std::string data)
		{
			unsigned long length = (data.size() * 2);
			char* buffer = Utils::Memory::AllocateArray<char>(length);

			if (compress2(reinterpret_cast<Bytef*>(buffer), &length, reinterpret_cast<Bytef*>(const_cast<char*>(data.data())), data.size(), Z_BEST_COMPRESSION) != Z_OK)
			{
				Utils::Memory::Free(buffer);
				return "";
			}

			data.clear();
			data.append(buffer, length);

			Utils::Memory::Free(buffer);

			return data;
		}

		std::string ZLib::Decompress(std::string data)
		{
			z_stream stream;
			ZeroMemory(&stream, sizeof(stream));
			std::string buffer;

			if (inflateInit(&stream) != Z_OK)
			{
				return "";
			}

			int ret = 0;
			uint8_t* dest = Utils::Memory::AllocateArray<uint8_t>(CHUNK);
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
						Utils::Memory::Free(dest);
						return "";
					}
					
					buffer.append(reinterpret_cast<const char*>(dest), CHUNK - stream.avail_out);

				} while (stream.avail_out == 0);

			} while (ret != Z_STREAM_END);

			inflateEnd(&stream);

			Utils::Memory::Free(dest);

			return buffer;
		}
	};
}
