#include <STDInclude.hpp>

namespace Assets
{
	void IRawFile::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_AssetStruct(Game::RawFile, 16);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::RawFile* asset = header.rawfile;
		Game::RawFile* dest = (Game::RawFile*)buffer->At();
		buffer->Save(asset, sizeof(Game::RawFile));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		if (asset->compressedData)
		{
			if (asset->sizeCompressed)
			{
				buffer->SaveString(asset->compressedData, asset->sizeCompressed);
			}
			else
			{
				buffer->SaveString(asset->compressedData, asset->sizeUnCompressed);
			}

			dest->compressedData = (char*)-1;
		}

		buffer->PopBlock();
	}
}
