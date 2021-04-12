#include "STDInclude.hpp"

namespace Assets
{
	void ILoadedSound::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File soundFile(Utils::String::VA("loaded_sound/%s", name.data()));
		if (!soundFile.exists())
		{
			header->loadSnd = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).loadSnd;
			return;
		}

		Game::LoadedSound* sound = builder->getAllocator()->allocate<Game::LoadedSound>();
		if (!sound)
		{
			Components::Logger::Print("Error allocating memory for sound structure!\n");
			return;
		}

		Game::LoadedSound* reference = nullptr;
		if (!reference) reference = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_LOADED_SOUND, "weapons/c4_detpack/c4_drop_dirt1.wav").loadSnd;

		std::memcpy(sound, reference, sizeof(Game::LoadedSound));
		sound->sound.data = nullptr;

		Utils::Stream::Reader reader(builder->getAllocator(), soundFile.getBuffer());

		unsigned int chunkIDBuffer = reader.read<unsigned int>();
		if (chunkIDBuffer != 0x46464952) // RIFF
		{
			Components::Logger::Error(0, "Reading sound '%s' failed, header is invalid!", name.data());
			return;
		}

		unsigned int chunkSize = reader.read<unsigned int>();

		unsigned int format = reader.read<unsigned int>();
		if (format != 0x45564157) // WAVE
		{
			Components::Logger::Error(0, "Reading sound '%s' failed, header is invalid!", name.data());
			return;
		}

		while (!sound->sound.data && !reader.end())
		{
			chunkIDBuffer = reader.read<unsigned int>();
			chunkSize = reader.read<unsigned int>();

			switch (chunkIDBuffer)
			{
			case 0x20746D66: // fmt
				if (chunkSize >= 16)
				{
					sound->sound.info.format = reader.read<short>();
					if (sound->sound.info.format != 1 && sound->sound.info.format != 17)
					{
						Components::Logger::Error(0, "Reading sound '%s' failed, invalid format!", name.data());
						return;
					}

					sound->sound.info.channels = reader.read<short>();
					sound->sound.info.rate = reader.read<int>();
					sound->sound.info.samples = reader.read<int>();
					sound->sound.info.block_size = reader.read<short>();
					sound->sound.info.bits = reader.read<short>();

					// skip any extra parameters
					if (chunkSize > 16)
					{
						reader.seekRelative(chunkSize - 16);
					}
				}
				break;

			case 0x61746164: // data
				sound->sound.info.data_len = chunkSize;
				sound->sound.data = reader.readArray<char>(chunkSize);
				break;

			default:
				if (chunkSize > 0)
				{
					reader.seekRelative(chunkSize);
				}
				break;
			}
		}

		if (!sound->sound.info.data_ptr)
		{
			Components::Logger::Error(0, "Reading sound '%s' failed, invalid format!", name.data());
			return;
		}

		sound->name = builder->getAllocator()->duplicateString(name.c_str());
		header->loadSnd = sound;
	}

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

			if (asset->sound.data)
			{
				if (builder->hasPointer(asset->sound.data))
				{
					dest->sound.data = builder->getPointer(asset->sound.data);
				}
				else
				{
					builder->storePointer(asset->sound.data);

					buffer->saveArray(asset->sound.data, asset->sound.info.data_len);
					Utils::Stream::ClearPointer(&dest->sound.data);
				}
			}

			buffer->popBlock();
		}

		buffer->popBlock();
	}
}
