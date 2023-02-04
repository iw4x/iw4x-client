#include <STDInclude.hpp>

#include <json.hpp>

#include "IFxWorld.hpp"

#define IW4X_FXWORLD_VERSION 1

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
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File fxWorldFile(std::format("fxworld/{}.iw4x.json", name));
		if (!fxWorldFile.exists())
		{
			return;
		}

		nlohmann::json fxWorldJson;
		try
		{
			fxWorldJson = nlohmann::json::parse(fxWorldFile.getBuffer());
		}
		catch (const std::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid JSON for gameworld {}! Error message: {}", name, e.what());
			return;
		}

		if (!fxWorldJson.is_object())
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid FXWORLD JSON for {}\n", name);
			return;
		}

		auto version = fxWorldJson["version"].is_number() ? fxWorldJson["version"].get<int>() : 0;
		if (version != IW4X_FXWORLD_VERSION)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid FXWORLD json version for {}, expected {} and got {}\n", name, IW4X_FXWORLD_VERSION, version);
			return;
		}

		auto map = builder->getAllocator()->allocate<Game::FxWorld>();
		map->name = builder->getAllocator()->duplicateString(_name);

		try
		{
			auto glassSys = &map->glassSys;
			auto glassSysJson = fxWorldJson["glassSys"];

			glassSys->time = glassSysJson["time"].get<int>();
			glassSys->prevTime = glassSysJson["prevTime"].get<int>();
			glassSys->defCount = glassSysJson["defCount"].get<unsigned int>();
			glassSys->pieceLimit = glassSysJson["pieceLimit"].get<unsigned int>();
			glassSys->pieceWordCount = glassSysJson["pieceWordCount"].get<unsigned int>();
			glassSys->initPieceCount = glassSysJson["initPieceCount"].get<unsigned int>();
			glassSys->cellCount = glassSysJson["cellCount"].get<unsigned int>();
			glassSys->activePieceCount = glassSysJson["activePieceCount"].get<unsigned int>();
			glassSys->firstFreePiece = glassSysJson["firstFreePiece"].get<unsigned int>();
			glassSys->geoDataLimit = glassSysJson["geoDataLimit"].get<unsigned int>();
			glassSys->geoDataCount = glassSysJson["geoDataCount"].get<unsigned int>();
			glassSys->initGeoDataCount = glassSysJson["initGeoDataCount"].get<unsigned int>();

			auto i = 0;
			glassSys->defs = builder->getAllocator()->allocateArray<Game::FxGlassDef>(glassSys->defCount);
			for (auto member : glassSysJson["defs"])
			{
				auto def = &glassSys->defs[i];

				def->halfThickness = member["halfThickness"].get<float>();

				auto xy = 0;
				for (auto x = 0; x < 2; x++)
				{
					for (auto y = 0; y < 2; y++)
					{
						def->texVecs[x][y] = member["texVecs"][xy].get<float>();
						xy++;
					}
				}

				def->color.packed = member["color"].get<int>();

				auto matShateredName = member["materialShattered"].get<std::string>();
				auto matName = member["material"].get<std::string>();
				auto physPresetName = member["physPreset"].get<std::string>();
				def->material = Components::AssetHandler::FindAssetForZone(Game::ASSET_TYPE_MATERIAL, matName, builder).material;
				def->materialShattered = Components::AssetHandler::FindAssetForZone(Game::ASSET_TYPE_MATERIAL, matShateredName, builder).material;
				def->physPreset = Components::AssetHandler::FindAssetForZone(Game::ASSET_TYPE_PHYSPRESET, physPresetName, builder ).physPreset;

				assert(def->material);
				assert(def->materialShattered);
				assert(def->physPreset);
				++i;
			}

			i = 0;
			glassSys->initPieceStates = builder->getAllocator()->allocateArray<Game::FxGlassInitPieceState>(glassSys->initPieceCount);
			for (auto member : glassSysJson["initPieceStates"])
			{
				auto initial = &glassSys->initPieceStates[i];

				for (int j = 0; j < ARRAYSIZE(initial->frame.quat); j++)
				{
					initial->frame.quat[j] = member["frame"]["quat"][j].get<float>();
				}

				for (int j = 0; j < ARRAYSIZE(initial->frame.origin); j++)
				{
					initial->frame.origin[j] = member["frame"]["origin"][j].get<float>();
				}

				initial->radius = member["radius"].get<float>();
				initial->texCoordOrigin[0] = member["texCoordOrigin"][0].get<float>();
				initial->texCoordOrigin[1] = member["texCoordOrigin"][1].get<float>();
				initial->supportMask = member["supportMask"].get<int>();
				initial->areaX2 = member["areaX2"].get<float>();
				initial->defIndex = member["defIndex"].get<char>();
				initial->vertCount = member["vertCount"].get<char>();
				initial->fanDataCount = member["fanDataCount"].get<char>();
				++i;
			}

			i = 0;
			glassSys->initGeoData = builder->getAllocator()->allocateArray<Game::FxGlassGeometryData>(glassSys->initGeoDataCount);
			for (auto member : glassSysJson["initGeoData"])
			{
				auto data = &glassSys->initGeoData[i];
				data->anonymous[0] = member[0];
				data->anonymous[1] = member[1];
				++i;
			}
		}
		catch (const nlohmann::json::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Malformed FXWORLD JSON for {}! Error message: {}\n", name, e.what());
			return;
		}

		map->glassSys.piecePlaces = builder->getAllocator()->allocateArray<Game::FxGlassPiecePlace>(map->glassSys.pieceLimit);
		map->glassSys.pieceStates = builder->getAllocator()->allocateArray<Game::FxGlassPieceState>(map->glassSys.pieceLimit);
		map->glassSys.pieceDynamics = builder->getAllocator()->allocateArray<Game::FxGlassPieceDynamics>(map->glassSys.pieceLimit);
		map->glassSys.geoData = builder->getAllocator()->allocateArray<Game::FxGlassGeometryData>(map->glassSys.geoDataLimit);
		map->glassSys.isInUse = builder->getAllocator()->allocateArray<unsigned int>(map->glassSys.pieceWordCount);
		map->glassSys.cellBits = builder->getAllocator()->allocateArray<unsigned int>(map->glassSys.pieceWordCount * map->glassSys.cellCount);
		map->glassSys.visData = builder->getAllocator()->allocateArray<char>((map->glassSys.pieceLimit + 15) & 0xFFFFFFF0); // ugh
		map->glassSys.linkOrg = reinterpret_cast<float(*)[3]>(builder->getAllocator()->allocateArray<float>(map->glassSys.pieceLimit));
		map->glassSys.halfThickness = builder->getAllocator()->allocateArray<float>(map->glassSys.pieceLimit * 3);
		map->glassSys.lightingHandles = builder->getAllocator()->allocateArray<unsigned short>(map->glassSys.initPieceCount);

		header->fxWorld = map;
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
