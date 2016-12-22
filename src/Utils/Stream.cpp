#include "STDInclude.hpp"

namespace Utils
{
	std::string Stream::Reader::readString()
	{
		std::string str;

		while (char byte = this->readByte())
		{
			str.append(&byte, 1);
		}

		return str;
	}

	const char* Stream::Reader::readCString()
	{
		return this->allocator->duplicateString(this->readString());
	}

	char Stream::Reader::readByte()
	{
		if ((this->position + 1) <= this->buffer.size())
		{
			return this->buffer[this->position++];
		}

		return 0;
	}

	void* Stream::Reader::read(size_t size, size_t count)
	{
		size_t bytes = size * count;

		if ((this->position + bytes) <= this->buffer.size())
		{
			void* _buffer = this->allocator->allocate(bytes);

			std::memcpy(_buffer, this->buffer.data() + this->position, bytes);
			this->position += bytes;

			return _buffer;
		}

		return nullptr;
	}

	bool Stream::Reader::end()
	{
		return (this->buffer.size() == this->position);
	}

	void Stream::Reader::seek(unsigned int _position)
	{
		if (this->buffer.size() >= _position)
		{
			this->position = _position;
		}
	}

	Stream::Stream() : criticalSectionState(0)
	{
		memset(this->blockSize, 0, sizeof(this->blockSize));
#ifdef DEBUG

        if(this->writeLog) return;
        if(fopen_s(&this->writeLog, "userraw/logs/zb_writes.log", "w"))
        {
            Components::Logger::Print("WARNING: Couldn't open write log. Writes from ZoneBuilder will not be logged.\n");
        }
#endif
	}

	Stream::Stream(size_t size) : Stream()
	{
		this->buffer.reserve(size);
	}

	Stream::~Stream()
	{
		this->buffer.clear();

		if (this->criticalSectionState != 0)
		{
			MessageBoxA(0, Utils::String::VA("Invalid critical section state '%i' for stream destruction!", this->criticalSectionState), "WARNING", MB_ICONEXCLAMATION);
		}

#ifdef DEBUG
        if(this->writeLog)
        {
            fclose(this->writeLog);
        }
#endif
	};

	size_t Stream::length()
	{
		return this->buffer.length();
	}

	size_t Stream::capacity()
	{
		return this->buffer.capacity();
	}

	char* Stream::save(const void* _str, size_t size, size_t count)
	{
		return this->save(this->getCurrentBlock(), _str, size, count);
	}

	char* Stream::save(Game::XFILE_BLOCK_TYPES stream, const void * _str, size_t size, size_t count)
	{
		//if (stream == XFILE_BLOCK_TEMP || stream == XFILE_BLOCK_VIRTUAL || stream == XFILE_BLOCK_PHYSICAL) // Only those seem to actually write data.
																											 // As I'm not sure though, I'll still write the data
																											 // Use IncreaseBlockSize to fill virtual streams
		auto data = this->data();

		if (this->isCriticalSection() && this->length() + (size * count) > this->capacity())
		{
			MessageBoxA(0, Utils::String::VA("Potential stream reallocation during critical operation detected! Writing data of the length 0x%X exceeds the allocated stream size of 0x%X\n", (size * count), this->capacity()), "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		this->buffer.append(static_cast<const char*>(_str), size * count);

        // log the write for zonebuilder debugging
        SAVE_LOG_WRITE(size * count);

		if (this->data() != data && this->isCriticalSection())
		{
			MessageBoxA(0, "Stream reallocation during critical operations not permitted!\nPlease increase the initial memory size or reallocate memory during non-critical sections!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		this->increaseBlockSize(stream, size * count);

		return this->at() - (size * count);
	}

	char* Stream::save(Game::XFILE_BLOCK_TYPES stream, int value, size_t count)
	{
		auto ret = this->length();

		for (size_t i = 0; i < count; ++i)
		{
			this->save(stream, &value, 4, 1);
		}

		return this->data() + ret;
	}

	char* Stream::saveString(std::string string)
	{
		return this->saveString(string.data()/*, string.size()*/);
	}

	char* Stream::saveString(const char* string)
	{
		return this->saveString(string, strlen(string));
	}

	char* Stream::saveString(const char* string, size_t len)
	{
		auto ret = this->length();

		if (string)
		{
			this->save(string, len);
		}

		this->saveNull();

		return this->data() + ret;
	}

	char* Stream::saveText(std::string string)
	{
		return this->save(string.data(), string.length());
	}

	char* Stream::saveByte(unsigned char byte, size_t count)
	{
		auto ret = this->length();

		for (size_t i = 0; i < count; ++i)
		{
			this->save(&byte, 1);
		}

		return this->data() + ret;
	}

	char* Stream::saveNull(size_t count)
	{
		return this->saveByte(0, count);
	}

	char* Stream::saveMax(size_t count)
	{
		return this->saveByte(static_cast<unsigned char>(-1), count);
	}

	void Stream::align(Stream::Alignment align)
	{
		uint32_t size = 2 << align;

		// Not power of 2!
		if (!size || (size & (size - 1))) return;
		--size;

		Game::XFILE_BLOCK_TYPES stream = this->getCurrentBlock();
		this->blockSize[stream] = ~size & (this->getBlockSize(stream) + size);
	}

	bool Stream::pushBlock(Game::XFILE_BLOCK_TYPES stream)
	{
		this->streamStack.push_back(stream);
		return this->isValidBlock(stream);
	}

	bool Stream::popBlock()
	{
		if (!this->streamStack.empty())
		{
			this->streamStack.pop_back();
			return true;
		}

		return false;
	}

	bool Stream::isValidBlock(Game::XFILE_BLOCK_TYPES stream)
	{
		return (stream < Game::MAX_XFILE_COUNT && stream >= Game::XFILE_BLOCK_TEMP);
	}

	void Stream::increaseBlockSize(Game::XFILE_BLOCK_TYPES stream, unsigned int size)
	{
		if (this->isValidBlock(stream))
		{
			this->blockSize[stream] += size;
		}
	}

	void Stream::increaseBlockSize(unsigned int size)
	{
		return this->increaseBlockSize(this->getCurrentBlock(), size);
	}

	Game::XFILE_BLOCK_TYPES Stream::getCurrentBlock()
	{
		if (!this->streamStack.empty())
		{
			return this->streamStack.back();
		}

		return Game::XFILE_BLOCK_INVALID;
	}

	char* Stream::at()
	{
		return reinterpret_cast<char*>(this->data() + this->length());
	}

	char* Stream::data()
	{
		return const_cast<char*>(this->buffer.data());
	}

	unsigned int Stream::getBlockSize(Game::XFILE_BLOCK_TYPES stream)
	{
		if (this->isValidBlock(stream))
		{
			return this->blockSize[stream];
		}

		return 0;
	}

	DWORD Stream::getPackedOffset()
	{
		Game::XFILE_BLOCK_TYPES block = this->getCurrentBlock();

		Stream::Offset offset;
		offset.block = block;
		offset.offset = this->getBlockSize(block);
		return offset.getPackedOffset();
	}

	void Stream::toBuffer(std::string& outBuffer)
	{
		outBuffer.clear();
		outBuffer.append(this->data(), this->length());
	}

	std::string Stream::toBuffer()
	{
		std::string _buffer;
		this->toBuffer(_buffer);
		return _buffer;
	}

	void Stream::enterCriticalSection()
	{
		++this->criticalSectionState;
	}

	void Stream::leaveCriticalSection()
	{
		--this->criticalSectionState;
	}

	bool Stream::isCriticalSection()
	{
		if (this->criticalSectionState < 0)
		{
			MessageBoxA(0, "CriticalSectionState in stream has been overrun!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		return (this->criticalSectionState != 0);
	}

#ifdef DEBUG

    FILE* Stream::writeLog = nullptr;
    int Stream::structLevel = 0;

    void Stream::enterStruct(const char* structName)
    {
        if(!this->writeLog) return;
        fprintf(this->writeLog, "%*s%s\n", this->structLevel++, "", structName);
    }

    void Stream::leaveStruct()
    {
        if(!this->writeLog) return;
        this->structLevel--;

        if(this->structLevel < 0) {
            Components::Logger::Print("Stream::exitStruct underflow! All following writes will not be logged!\n");
            fclose(this->writeLog);
            this->writeLog = nullptr;
            return;
        }

        fprintf(this->writeLog, "%*s-----\n", this->structLevel, "");
    }

    void Stream::logWrite(int writeLen)
    {
        if(!this->writeLog) return;
        fprintf(this->writeLog, "%*s%d\n", this->structLevel, "", writeLen);
    }

#endif
}
