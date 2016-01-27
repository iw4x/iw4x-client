#include <STDInclude.hpp>

namespace Assets
{
	void IRawFile::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File rawFile(name);

		if (rawFile.Exists())
		{
			Game::RawFile* asset = builder->GetAllocator()->AllocateArray<Game::RawFile>();

			std::string data = Utils::Compression::ZLib::Compress(rawFile.GetBuffer());

			asset->name = builder->GetAllocator()->DuplicateString(name);
			asset->compressedData = builder->GetAllocator()->DuplicateString(data);
			asset->sizeCompressed = data.size() + 1;
			asset->sizeUnCompressed = 0;

			header->rawfile = asset;
		}
	}

	void IRawFile::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::RawFile, 16);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::RawFile* asset = header.rawfile;
		Game::RawFile* dest = buffer->Dest<Game::RawFile>();
		buffer->Save(asset, sizeof(Game::RawFile));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->compressedData)
		{
			if (asset->sizeCompressed)
			{
				buffer->Save(asset->compressedData, asset->sizeCompressed);
			}
			else
			{
				buffer->Save(asset->compressedData, asset->sizeUnCompressed + 1);
			}

			dest->compressedData = reinterpret_cast<char*>(-1);
		}

		buffer->PopBlock();
	}
}
