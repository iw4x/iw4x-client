#include <STDInclude.hpp>
#include "ILoadedSound.hpp"

namespace Assets
{
	void ILoadedSound::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->loadSnd = builder->getIW4OfApi()->read<Game::LoadedSound>(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, name);
	}

	void ILoadedSound::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::LoadedSound, 44);

		auto* buffer = builder->getBuffer();
		auto* asset = header.loadSnd;
		auto* dest = buffer->dest<Game::LoadedSound>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		{
			buffer->pushBlock(Game::XFILE_BLOCK_TEMP);

			if (asset->sound.data)
			{
				buffer->saveArray(asset->sound.data, asset->sound.info.data_len);
				Utils::Stream::ClearPointer(&dest->sound.data);
			}

			buffer->popBlock();
		}

		buffer->popBlock();
	}
}
