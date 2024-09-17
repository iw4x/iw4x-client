#include <STDInclude.hpp>
#include "IRawFile.hpp"

#include <Utils/Compression.hpp>

namespace Assets
{
	void IRawFile::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->rawfile = builder->getIW4OfApi()->read<Game::RawFile>(Game::XAssetType::ASSET_TYPE_RAWFILE, name);
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
