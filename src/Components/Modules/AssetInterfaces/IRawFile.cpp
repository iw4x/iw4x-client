#include <STDInclude.hpp>
#include "IRawFile.hpp"

#include <Utils/Compression.hpp>

namespace Assets
{
	void IRawFile::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File rawFile(name);

		if (!rawFile.exists())
		{
			return;
		}

		auto* asset = builder->getAllocator()->allocate<Game::RawFile>();
		if (!asset)
		{
			return;
		}

		asset->name = builder->getAllocator()->duplicateString(name);
		asset->len = static_cast<int>(rawFile.getBuffer().size());

		const auto compressedData = Utils::Compression::ZLib::Compress(rawFile.getBuffer());
		// Only save the compressed buffer if we gained space
		if (compressedData.size() < rawFile.getBuffer().size())
		{
			asset->buffer = builder->getAllocator()->allocateArray<char>(compressedData.size());
			std::memcpy(const_cast<char*>(asset->buffer), compressedData.data(), compressedData.size());
			asset->compressedLen = static_cast<int>(compressedData.size());
		}
		else
		{
			asset->buffer = builder->getAllocator()->allocateArray<char>(rawFile.getBuffer().size() + 1);
			std::memcpy(const_cast<char*>(asset->buffer), rawFile.getBuffer().data(), rawFile.getBuffer().size());
			asset->compressedLen = 0;
		}

		header->rawfile = asset;
	}

	void IRawFile::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::RawFile, 16);

		auto* buffer = builder->getBuffer();
		auto* asset = header.rawfile;
		auto* dest = buffer->dest<Game::RawFile>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->buffer)
		{
			if (asset->compressedLen)
			{
				buffer->save(asset->buffer, asset->compressedLen);
			}
			else
			{
				buffer->save(asset->buffer, asset->len + 1);
			}

			Utils::Stream::ClearPointer(&dest->buffer);
		}

		buffer->popBlock();
	}
}
