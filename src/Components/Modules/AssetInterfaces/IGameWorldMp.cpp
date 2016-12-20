#include <STDInclude.hpp>

namespace Assets
{
	void IGameWorldMp::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GameWorldMp, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GameWorldMp* asset = header.gameMapMP;
		Game::GameWorldMp* dest = buffer->dest<Game::GameWorldMp>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->data)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			// Save_G_GlassData
			{
				AssertSize(Game::G_GlassData, 128);

				Game::G_GlassData* destGlass = buffer->dest<Game::G_GlassData>();
				buffer->save(asset->data);

				if (asset->data->glassPieces)
				{
					AssertSize(Game::G_GlassPiece, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(asset->data->glassPieces, asset->data->pieceCount);
					Utils::Stream::ClearPointer(&destGlass->glassPieces);
				}

				if (asset->data->glassNames)
				{
					AssertSize(Game::G_GlassName, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					Game::G_GlassName* destGlassNames = buffer->dest<Game::G_GlassName>();
					buffer->saveArray(asset->data->glassNames, asset->data->glassNameCount);

					for (unsigned int i = 0; i < asset->data->glassNameCount; ++i)
					{
						Game::G_GlassName* destGlassName = &destGlassNames[i];
						Game::G_GlassName* glassName = &asset->data->glassNames[i];

						if (glassName->nameStr)
						{
							buffer->saveString(glassName->nameStr);
							Utils::Stream::ClearPointer(&destGlassName->nameStr);
						}

						if (glassName->pieceIndices)
						{
							buffer->align(Utils::Stream::ALIGN_2);
							buffer->saveArray(glassName->pieceIndices, glassName->pieceCount);
							Utils::Stream::ClearPointer(&destGlassName->pieceIndices);
						}
					}

					Utils::Stream::ClearPointer(&destGlass->glassNames);
				}
			}

			Utils::Stream::ClearPointer(&dest->data);
		}

		buffer->popBlock();
	}
}
