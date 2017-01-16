#include <STDInclude.hpp>

namespace Assets
{
	void ILoadedSound::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::LoadedSound, 44);

		Utils::Stream* buffer = builder->getBuffer();
		Game::LoadedSound* asset = header.loadSnd;
		Game::LoadedSound* dest = buffer->dest<Game::LoadedSound>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		{
			buffer->pushBlock(Game::XFILE_BLOCK_TEMP);

			if (asset->mssSound.data)
			{
				if (builder->hasPointer(asset->mssSound.data))
				{
					dest->mssSound.data = builder->getPointer(asset->mssSound.data);
				}
				else
				{
					builder->storePointer(asset->mssSound.data);

					buffer->saveArray(asset->mssSound.data, asset->mssSound.size);
					Utils::Stream::ClearPointer(&dest->mssSound.data);
				}
			}

			buffer->popBlock();
		}

		buffer->popBlock();
	}
}
