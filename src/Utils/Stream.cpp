#include <STDInclude.hpp>

namespace Utils
{
	std::string Stream::Reader::readString()
	{
		std::string str;

		while (char byte = this->readByte())
		{
			str.push_back(byte);
		}

		return str;
	}

	const char* Stream::Reader::readCString()
	{
		return this->allocator_->duplicateString(this->readString());
	}

	char Stream::Reader::readByte()
	{
		if ((this->position_ + 1) <= this->buffer_.size())
		{
			return this->buffer_[this->position_++];
		}

		throw std::runtime_error("Reading past the buffer");
	}

	void* Stream::Reader::read(size_t size, std::size_t count)
	{
		auto bytes = size * count;

		if ((this->position_ + bytes) <= this->buffer_.size())
		{
			auto* buffer = this->allocator_->allocate(bytes);
			std::memcpy(buffer, this->buffer_.data() + this->position_, bytes);
			this->position_ += bytes;

			return buffer;
		}

		throw std::runtime_error("Reading past the buffer");
	}

	bool Stream::Reader::end() const
	{
		return (this->buffer_.size() == this->position_);
	}

	void Stream::Reader::seek(unsigned int position)
	{
		if (this->buffer_.size() >= position)
		{
			this->position_ = position;
		}
	}

	void Stream::Reader::seekRelative(unsigned int position)
	{
		return this->seek(position + this->position_);
	}

	void* Stream::Reader::readPointer()
	{
		auto* pointer = this->read<void*>();
		if (!this->hasPointer(pointer))
		{
			this->pointerMap_[pointer] = nullptr;
		}
		return pointer;
	}

	void Stream::Reader::mapPointer(void* oldPointer, void* newPointer)
	{
		if (this->hasPointer(oldPointer))
		{
			this->pointerMap_[oldPointer] = newPointer;
		}
	}

	bool Stream::Reader::hasPointer(void* pointer) const
	{
		return this->pointerMap_.contains(pointer);
	}

	Stream::Stream() : ptrAssertion(false), criticalSectionState(0)
	{
		std::memset(this->blockSize, 0, sizeof(this->blockSize));

#ifdef WRITE_LOGS
		this->structLevel = 0;
		Utils::IO::WriteFile("userraw/logs/zb_writes.log", "", false);
#endif
	}

	Stream::Stream(size_t size) : Stream()
	{
		this->buffer_.reserve(size);
	}

	Stream::~Stream()
	{
		this->buffer_.clear();

		if (this->criticalSectionState != 0)
		{
			MessageBoxA(nullptr, String::VA("Invalid critical section state '%i' for stream destruction!", this->criticalSectionState), "WARNING", MB_ICONEXCLAMATION);
		}
	};

	std::size_t Stream::length() const
	{
		return this->buffer_.length();
	}

	std::size_t Stream::capacity() const
	{
		return this->buffer_.capacity();
	}

	void Stream::assertPointer(const void* pointer, std::size_t length)
	{
		if (!this->ptrAssertion) return;

		for (auto& entry : this->ptrList)
		{
			unsigned int ePtr = reinterpret_cast<unsigned int>(entry.first);
			unsigned int tPtr = reinterpret_cast<unsigned int>(pointer);

			if (HasIntersection(ePtr, entry.second, tPtr, length))
			{
				MessageBoxA(nullptr, "Duplicate data written!", "ERROR", MB_ICONERROR);
#ifdef _DEBUG
				__debugbreak();
#endif
			}
		}

		this->ptrList.push_back({ pointer, length });
	}

	char* Stream::save(const void* str, std::size_t size, std::size_t count)
	{
		return this->save(this->getCurrentBlock(), str, size, count);
	}

	char* Stream::save(Game::XFILE_BLOCK_TYPES stream, const void * str, std::size_t size, std::size_t count)
	{
		// Only those seem to actually write data.
		// everything else is allocated at runtime but XFILE_BLOCK_RUNTIME is the only one that actually allocates anything
		// clearly half of this stuff is unused
		if (stream == Game::XFILE_BLOCK_RUNTIME)
		{
			this->increaseBlockSize(stream, size * count);
			return this->at();
		}

		auto* data = this->data();

		if (this->isCriticalSection() && this->length() + (size * count) > this->capacity())
		{
			MessageBoxA(nullptr, String::VA("Potential stream reallocation during critical operation detected! Writing data of the length 0x%X exceeds the allocated stream size of 0x%X\n", (size * count), this->capacity()), "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		this->buffer_.append(static_cast<const char*>(str), size * count);

		if (this->data() != data && this->isCriticalSection())
		{
			MessageBoxA(nullptr, "Stream reallocation during critical operations not permitted!\nPlease increase the initial memory size or reallocate memory during non-critical sections!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		this->increaseBlockSize(stream, size * count);
		this->assertPointer(str, size * count);

		return this->at() - (size * count);
	}

	char* Stream::save(Game::XFILE_BLOCK_TYPES stream, int value, std::size_t count)
	{
		auto ret = this->length();

		for (size_t i = 0; i < count; ++i)
		{
			this->save(stream, &value, 4, 1);
		}

		return this->data() + ret;
	}

	char* Stream::saveString(const std::string& string)
	{
		return this->saveString(string.data()/*, string.size()*/);
	}

	char* Stream::saveString(const char* string)
	{
		return this->saveString(string, strlen(string));
	}

	char* Stream::saveString(const char* string, std::size_t len)
	{
		auto ret = this->length();

		if (string)
		{
			this->save(string, len);
		}

		this->saveNull();

		return this->data() + ret;
	}

	char* Stream::saveText(const std::string& string)
	{
		return this->save(string.data(), string.length());
	}

	char* Stream::saveByte(unsigned char byte, std::size_t count)
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

	bool Stream::hasBlock()
	{
		return !this->streamStack.empty();
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

#ifdef WRITE_LOGS
		const auto* data = String::VA("%*s%u\n", this->structLevel, "", size);
		if (stream == Game::XFILE_BLOCK_RUNTIME) data = String::VA("%*s(%u)\n", this->structLevel, "", size);
		IO::WriteFile("userraw/logs/zb_writes.log", data, true);
#endif
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
		return const_cast<char*>(this->buffer_.data());
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
		std::string buffer;
		this->toBuffer(buffer);
		return buffer;
	}

	void Stream::enterCriticalSection()
	{
		++this->criticalSectionState;
	}

	void Stream::leaveCriticalSection()
	{
		--this->criticalSectionState;
	}

	bool Stream::isCriticalSection() const
	{
		if (this->criticalSectionState < 0)
		{
			MessageBoxA(nullptr, "CriticalSectionState in stream has been overrun!", "ERROR", MB_ICONERROR);
			__debugbreak();
		}

		return (this->criticalSectionState != 0);
	}

#ifdef WRITE_LOGS
	void Stream::enterStruct(const char* structName)
	{
		if (this->structLevel >= 0)
		{
			IO::WriteFile("userraw/logs/zb_writes.log", String::VA("%*s%s\n", this->structLevel++, "", structName), true);
		}
	}

	void Stream::leaveStruct()
	{
		if (--this->structLevel < 0)
		{
			Components::Logger::Print("Stream::leaveStruct underflow! All following writes will not be logged!\n");
			return;
		}

		IO::WriteFile("userraw/logs/zb_writes.log", String::VA("%*s-----\n", this->structLevel, ""), true);
	}
#endif
}
