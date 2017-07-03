#include "STDInclude.hpp"

namespace Utils
{
	namespace Compression
	{
		std::mutex Deflate::Mutex;

		std::string Deflate::Compress(std::string data)
		{
			Utils::Memory::Allocator allocator;
			unsigned long length = (data.size() * 2);
			if (!length) length = 2;

			// Make sure the buffer is large enough
			if (length < 100) length *= 10;

			char* buffer = allocator.allocateArray<char>(length);

			int level = (ZWRAP_isUsingZSTDcompression() ? ZSTD_maxCLevel() : Z_BEST_COMPRESSION);
			if (compress2(reinterpret_cast<Bytef*>(buffer), &length, reinterpret_cast<Bytef*>(const_cast<char*>(data.data())), data.size(), level) != Z_OK)
			{
				return "";
			}

			data.clear();
			data.append(buffer, length);

			return data;
		}

		std::string Deflate::Decompress(std::string data)
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

		std::string Deflate::ZLib::Compress(std::string data)
		{
			Deflate::Semaphore _(DEFLATE_ZLIB);
			return Deflate::Compress(data);
		}

		std::string Deflate::ZLib::Decompress(std::string data)
		{
			Deflate::Semaphore _(DEFLATE_ZLIB);
			return Deflate::Decompress(data);
		}

		std::string Deflate::ZStd::Compress(std::string data, bool safe)
		{
			if (safe)
			{
				Utils::Memory::Allocator allocator;

				size_t size = data.size() + 100;
				size = ZSTD_compressBound(size);
				char* buffer = allocator.allocateArray<char>(size);

				size = ZSTD_compress(buffer, size, data.data(), data.size(), ZSTD_maxCLevel());
				if (size == 0 || ZSTD_isError(size)) return "";

				return std::string(buffer, size);
			}
			else
			{
				Deflate::Semaphore _(DEFLATE_ZSTD);
				return Deflate::Compress(data);
			}
		}

		std::string Deflate::ZStd::Decompress(std::string data, bool safe)
		{
			if (safe)
			{
				if (data.empty()) return "";

				Utils::Memory::Allocator allocator;

				size_t size = data.size() + 100;
				size = ZSTD_compressBound(size);
				size_t maxSize = size;
				char* buffer = allocator.allocateArray<char>(size);

				while(true)
				{
					size = ZSTD_decompress(buffer, size, data.data(), data.size());
					if (!ZSTD_isError(size)) break;

					maxSize++;
					maxSize *= 2;
					size = maxSize;

					allocator.free(buffer);
					buffer = allocator.allocateArray<char>(size);
				}

				return std::string(buffer, size);
			}
			else
			{
				Deflate::Semaphore _(DEFLATE_ZSTD);
				return Deflate::Decompress(data);
			}
		}

		Deflate::Semaphore::Semaphore(bool zstd)
		{
			Deflate::Mutex.lock();
			this->state = ZWRAP_isUsingZSTDcompression();
			ZWRAP_useZSTDcompression(zstd);
		}

		Deflate::Semaphore::~Semaphore()
		{
			ZWRAP_useZSTDcompression(this->state);
			Deflate::Mutex.unlock();
		}
	};
}
