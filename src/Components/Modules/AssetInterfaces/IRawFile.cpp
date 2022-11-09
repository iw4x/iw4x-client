#include <STDInclude.hpp>
#include "IRawFile.hpp"

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

		const auto data = Utils::Compression::ZLib::Compress(rawFile.getBuffer());

		asset->name = builder->getAllocator()->duplicateString(name);

		if (data.size() < rawFile.getBuffer().size())
		{
			asset->buffer = builder->getAllocator()->duplicateString(data);
			asset->compressedLen = static_cast<int>(data.size());
		}
		else
		{
			asset->buffer = builder->getAllocator()->duplicateString(rawFile.getBuffer());
			asset->compressedLen = 0;
		}

		asset->len = static_cast<int>(rawFile.getBuffer().size());

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
