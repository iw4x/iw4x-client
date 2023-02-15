#include <STDInclude.hpp>
#include "IGameWorldMp.hpp"

#define IW4X_GAMEWORLD_VERSION 1

namespace Assets
{
	void IGameWorldMp::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GameWorldMp, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GameWorldMp* asset = header.gameWorldMp;
		Game::GameWorldMp* dest = buffer->dest<Game::GameWorldMp>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->g_glassData)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			// Save_G_GlassData
			{
				AssertSize(Game::G_GlassData, 128);

				Game::G_GlassData* destGlass = buffer->dest<Game::G_GlassData>();
				buffer->save(asset->g_glassData);

				if (asset->g_glassData->glassPieces)
				{
					AssertSize(Game::G_GlassPiece, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(asset->g_glassData->glassPieces, asset->g_glassData->pieceCount);
					Utils::Stream::ClearPointer(&destGlass->glassPieces);
				}

				if (asset->g_glassData->glassNames)
				{
					AssertSize(Game::G_GlassName, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					Game::G_GlassName* destGlassNames = buffer->dest<Game::G_GlassName>();
					buffer->saveArray(asset->g_glassData->glassNames, asset->g_glassData->glassNameCount);

					for (unsigned int i = 0; i < asset->g_glassData->glassNameCount; ++i)
					{
						Game::G_GlassName* destGlassName = &destGlassNames[i];
						Game::G_GlassName* glassName = &asset->g_glassData->glassNames[i];

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

			Utils::Stream::ClearPointer(&dest->g_glassData);
		}

		buffer->popBlock();
	}

	void IGameWorldMp::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->gameWorldMp = builder->getIW4OfApi()->read<Game::GameWorldMp>(Game::XAssetType::ASSET_TYPE_GAMEWORLD_MP, _name);
	}
}
