#include <STDInclude.hpp>

namespace Assets
{
	void IRawFile::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File rawFile(name);

		if (rawFile.exists())
		{
			Game::RawFile* asset = builder->getAllocator()->allocate<Game::RawFile>();

			if (asset)
			{
				//std::string data = Utils::Compression::ZLib::Compress(rawFile.getBuffer());

				asset->name = builder->getAllocator()->duplicateString(name);
				asset->compressedData = builder->getAllocator()->duplicateString(rawFile.getBuffer());
				asset->sizeCompressed = 0;//data.size();
				asset->sizeUnCompressed = rawFile.getBuffer().size();

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

		if (asset->compressedData)
		{
			if (asset->sizeCompressed)
			{
				buffer->save(asset->compressedData, asset->sizeCompressed);
			}
			else
			{
				buffer->save(asset->compressedData, asset->sizeUnCompressed + 1);
			}

			Utils::Stream::ClearPointer(&dest->compressedData);
		}

		buffer->popBlock();
	}
}
