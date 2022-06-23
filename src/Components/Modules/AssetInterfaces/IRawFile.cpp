#include <STDInclude.hpp>
#include "IRawFile.hpp"

namespace Assets
{
	void IRawFile::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File rawFile(name);

		if (rawFile.exists())
		{
			Game::RawFile* asset = builder->getAllocator()->allocate<Game::RawFile>();

			if (asset)
			{
				//std::string data = Utils::Compression::ZLib::Compress(rawFile.getBuffer());

				asset->name = builder->getAllocator()->duplicateString(name);
				asset->buffer = builder->getAllocator()->duplicateString(rawFile.getBuffer());
				asset->compressedLen = 0;//data.size();
				asset->len = rawFile.getBuffer().size();

				header->rawfile = asset;
			}
		}
	}

	void IRawFile::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::RawFile, 16);

		Utils::Stream* buffer = builder->getBuffer();
		Game::RawFile* asset = header.rawfile;
		Game::RawFile* dest = buffer->dest<Game::RawFile>();
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
