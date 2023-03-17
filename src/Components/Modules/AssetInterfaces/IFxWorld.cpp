#include <STDInclude.hpp>
#include "IFxWorld.hpp"

namespace Assets
{
	void IFxWorld::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::FxWorld, 116);

		Utils::Stream* buffer = builder->getBuffer();
		SaveLogEnter("FxWorld");

		Game::FxWorld* asset = header.fxWorld;
		Game::FxWorld* dest = buffer->dest<Game::FxWorld>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		// saveFxGlassSystem
		{
			AssertSize(Game::FxGlassSystem, 112);
			if (asset->glassSys.defs)
			{
				AssertSize(Game::FxGlassDef, 36);

				buffer->align(Utils::Stream::ALIGN_4);
				Game::FxGlassDef* glassDefTable = buffer->dest<Game::FxGlassDef>();
				buffer->saveArray(asset->glassSys.defs, asset->glassSys.defCount);

				for (unsigned int i = 0; i < asset->glassSys.defCount; ++i)
				{
					Game::FxGlassDef* glassDef = &asset->glassSys.defs[i];
					Game::FxGlassDef* destGlassDef = &glassDefTable[i];

					if (glassDef->physPreset)
					{
						destGlassDef->physPreset = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, glassDef->physPreset).physPreset;
					}

					if (glassDef->material)
					{
						destGlassDef->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, glassDef->material).material;
					}

					if (glassDef->materialShattered)
					{
						destGlassDef->materialShattered = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, glassDef->materialShattered).material;
					}
				}

				Utils::Stream::ClearPointer(&dest->glassSys.defs);
			}

			buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

			if (asset->glassSys.piecePlaces)
			{
				AssertSize(Game::FxGlassPiecePlace, 32);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.piecePlaces, asset->glassSys.pieceLimit);
				Utils::Stream::ClearPointer(&dest->glassSys.piecePlaces);
			}

			if (asset->glassSys.pieceStates)
			{
				AssertSize(Game::FxGlassPieceState, 32);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.pieceStates, asset->glassSys.pieceLimit);
				Utils::Stream::ClearPointer(&dest->glassSys.pieceStates);
			}

			if (asset->glassSys.pieceDynamics)
			{
				AssertSize(Game::FxGlassPieceDynamics, 36);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.pieceDynamics, asset->glassSys.pieceLimit);
				Utils::Stream::ClearPointer(&dest->glassSys.pieceDynamics);
			}

			if (asset->glassSys.geoData)
			{
				AssertSize(Game::FxGlassGeometryData, 4);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.geoData, asset->glassSys.geoDataLimit);
				Utils::Stream::ClearPointer(&dest->glassSys.geoData);
			}

			if (asset->glassSys.isInUse)
			{
				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.isInUse, asset->glassSys.pieceWordCount);
				Utils::Stream::ClearPointer(&dest->glassSys.isInUse);
			}

			if (asset->glassSys.cellBits)
			{
				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.cellBits, asset->glassSys.pieceWordCount * asset->glassSys.cellCount);
				Utils::Stream::ClearPointer(&dest->glassSys.cellBits);
			}

			if (asset->glassSys.visData)
			{
				buffer->align(Utils::Stream::ALIGN_16);
				buffer->save(asset->glassSys.visData, 1, (asset->glassSys.pieceLimit + 15) & 0xFFFFFFF0);
				Utils::Stream::ClearPointer(&dest->glassSys.visData);
			}

			if (asset->glassSys.linkOrg)
			{
				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.linkOrg, asset->glassSys.pieceLimit);
				Utils::Stream::ClearPointer(&dest->glassSys.linkOrg);
			}

			if (asset->glassSys.halfThickness)
			{
				buffer->align(Utils::Stream::ALIGN_16);
				buffer->save(asset->glassSys.halfThickness, 1, ((4 * asset->glassSys.pieceLimit) + 12) & 0xFFFFFFF0);
				Utils::Stream::ClearPointer(&dest->glassSys.halfThickness);
			}

			buffer->popBlock();

			if (asset->glassSys.lightingHandles)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(asset->glassSys.lightingHandles, asset->glassSys.initPieceCount);
				Utils::Stream::ClearPointer(&dest->glassSys.lightingHandles);
			}

			if (asset->glassSys.initPieceStates)
			{
				AssertSize(Game::FxGlassInitPieceState, 52);

				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.initPieceStates, asset->glassSys.initPieceCount);
				Utils::Stream::ClearPointer(&dest->glassSys.initPieceStates);
			}

			if (asset->glassSys.initGeoData)
			{
				buffer->align(Utils::Stream::ALIGN_4);
				buffer->saveArray(asset->glassSys.initGeoData, asset->glassSys.initGeoDataCount);
				Utils::Stream::ClearPointer(&dest->glassSys.initGeoData);
			}
		}

		SaveLogExit();
		buffer->popBlock();
	}

	void IFxWorld::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::FxWorld* asset = header.fxWorld;

		if (asset->glassSys.defs)
		{
			for (unsigned int i = 0; i < asset->glassSys.defCount; ++i)
			{
				if (asset->glassSys.defs[i].physPreset)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->glassSys.defs[i].physPreset);
				}

				if (asset->glassSys.defs[i].material)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->glassSys.defs[i].material);
				}

				if (asset->glassSys.defs[i].materialShattered)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->glassSys.defs[i].materialShattered);
				}
			}
		}
	}
	
	void IFxWorld::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->fxWorld) loadFromDisk(header, name, builder);
		if (!header->fxWorld) generate(header, name, builder);

		assert(header->fxWorld);
	}

	void IFxWorld::loadFromDisk(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		header->fxWorld = builder->getIW4OfApi()->read<Game::FxWorld>(Game::XAssetType::ASSET_TYPE_FXWORLD, _name);
	}

	void IFxWorld::generate(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Game::FxWorld* map = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_FXWORLD, name.data()).fxWorld;
		if (map) return;

		// Generate
		map = builder->getAllocator()->allocate<Game::FxWorld>();
		map->name = builder->getAllocator()->duplicateString(name);

		// No glass for you!
		ZeroMemory(&map->glassSys, sizeof(map->glassSys));

		map->glassSys.firstFreePiece = 0xFFFF; // That's how rust has it (no glass)

		header->fxWorld = map;
	}
}
