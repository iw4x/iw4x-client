#include "STDInclude.hpp"

#define IW4X_MAT_VERSION "1"

namespace Assets
{
	void IMaterial::load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadJson(header, name, builder);   // Check if we want to override materials
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
	}

	void IMaterial::loadBinary(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File materialFile(Utils::String::VA("materials/%s.iw4xMaterial", name.data()));
		if (!materialFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), materialFile.getBuffer());

		char* magic = reader.readArray<char>(7);
		if (std::memcmp(magic, "IW4xMat", 7))
		{
			Components::Logger::Error(0, "Reading material '%s' failed, header is invalid!", name.data());
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_MAT_VERSION)
		{
			Components::Logger::Error("Reading material '%s' failed, expected version is %d, but it was %d!", name.data(), atoi(IW4X_MAT_VERSION), atoi(version.data()));
		}

		Game::Material* asset = reader.readObject<Game::Material>();

		if (asset->name)
		{
			asset->name = reader.readCString();
		}

		if (asset->techniqueSet)
		{
			std::string techset = reader.readString();
			if (!techset.empty() && techset.front() == ',') techset.erase(techset.begin());
			asset->techniqueSet = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, techset.data(), builder).techniqueSet;

			const char* techsetSuffix[] =
			{
				"_lin",
				"_add_lin",
				"_replace",
				"_eyeoffset",
			};

			if (!asset->techniqueSet)
			{
				for (int i = 0; i < ARRAYSIZE(techsetSuffix); ++i)
				{
					Game::MaterialTechniqueSet* techsetPtr = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, (techset + techsetSuffix[i]).data(), builder).techniqueSet;

					if (techsetPtr)
					{
						asset->techniqueSet = techsetPtr;

						if (asset->techniqueSet->name[0] == ',') continue; // Try to find a better one
						Components::Logger::Print("Techset '%s' has been mapped to '%s'\n", techset.data(), asset->techniqueSet->name);
						break;
					}
				}
			}

			if (!asset->techniqueSet)
			{
				Components::Logger::Error("Techset '%s' not found!", techset.data());
			}
		}

		if (asset->textureTable)
		{
			asset->textureTable = reader.readArray<Game::MaterialTextureDef>(asset->textureCount);

			for (char i = 0; i < asset->textureCount; ++i)
			{
				Game::MaterialTextureDef* textureDef = &asset->textureTable[i];

				if (textureDef->semantic == SEMANTIC_WATER_MAP)
				{
					if (textureDef->info.water)
					{
						Game::water_t* water = reader.readObject<Game::water_t>();
						textureDef->info.water = water;

						// Save_water_t
						if (water->H0)
						{
							water->H0 = reader.readArray<Game::complex_s>(water->M * water->N);
						}

						if (water->wTerm)
						{
							water->wTerm = reader.readArray<float>(water->M * water->N);
						}

						if (water->image)
						{
							water->image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, reader.readString().data(), builder).image;
						}
					}
				}
				else if (textureDef->info.image)
				{
					textureDef->info.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, reader.readString().data(), builder).image;
				}
			}
		}

		if (asset->constantTable)
		{
			asset->constantTable = reader.readArray<Game::MaterialConstantDef>(asset->constantCount);
		}

		if (asset->stateBitTable)
		{
			asset->stateBitTable = reader.readArray<Game::GfxStateBits>(asset->stateBitsCount);
		}

		header->material = asset;

		static thread_local bool replacementFound;
		replacementFound = false;

		// Find correct sortkey by comparing techsets
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_MATERIAL, [](Game::XAssetHeader header, void* data)
		{
			if (!replacementFound)
			{
				Game::Material* material = reinterpret_cast<Game::Material*>(data);
				const char* name = material->techniqueSet->name;
				if (name[0] == ',') ++name;

				if (std::string(name) == header.material->techniqueSet->name)
				{
					material->sortKey = header.material->sortKey;

					// This is temp, as nobody has time to fix materials
					material->stateBitsCount = header.material->stateBitsCount;
					material->stateBitTable = header.material->stateBitTable;
					std::memcpy(material->stateBitsEntry, header.material->stateBitsEntry, 48);
					material->constantCount = header.material->constantCount;
					material->constantTable = header.material->constantTable;

					replacementFound = true;
				}
			}
		}, asset, false);

		if(!replacementFound)
		{
			Components::Logger::Print("No replacement found for material %s with techset %s\n", asset->name, asset->techniqueSet->name);
		}

		if (!reader.end())
		{
			Components::Logger::Error("Material data left!");
		}
	}

	void IMaterial::loadNative(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->material = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).material;
	}

	void IMaterial::loadJson(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File materialInfo(Utils::String::VA("materials/%s.json", name.data()));

		if (!materialInfo.exists()) return;

		std::string errors;
		json11::Json infoData = json11::Json::parse(materialInfo.getBuffer(), errors);

		if (!infoData.is_object())
		{
			Components::Logger::Error("Failed to load material information for %s!", name.data());
			return;
		}

		auto base = infoData["base"];

		if (!base.is_string())
		{
			Components::Logger::Error("No valid material base provided for %s!", name.data());
			return;
		}

		Game::Material* baseMaterial = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, base.string_value().data()).material;

		if (!baseMaterial) // TODO: Maybe check if default asset? Maybe not? You could still want to use the default one as base!?
		{
			Components::Logger::Error("Basematerial '%s' not found for %s!", base.string_value().data(), name.data());
			return;
		}

		Game::Material* material = builder->getAllocator()->allocate<Game::Material>();

		if (!material)
		{
			Components::Logger::Error("Failed to allocate material structure!");
			return;
		}

		// Copy base material to our structure
		std::memcpy(material, baseMaterial, sizeof(Game::Material));
		material->name = builder->getAllocator()->duplicateString(name);

		material->textureAtlasRowCount = 1;
		material->textureAtlasColumnCount = 1;

		// Load animation frames
		auto anims = infoData["anims"];
		if (anims.is_array())
		{
			auto animCoords = anims.array_items();

			if (animCoords.size() >= 2)
			{
				auto animCoordX = animCoords[0];
				auto animCoordY = animCoords[1];

				if (animCoordX.is_number())
				{
					material->textureAtlasColumnCount = static_cast<char>(animCoordX.number_value()) & 0xFF;
				}

				if (animCoordY.is_number())
				{
					material->textureAtlasRowCount = static_cast<char>(animCoordY.number_value()) & 0xFF;
				}
			}
		}

		// Model surface textures are special, they need a special order and whatnot
		bool replaceTexture = Utils::String::StartsWith(name, "mc/");
		if (replaceTexture)
		{
			Game::MaterialTextureDef* textureTable = builder->getAllocator()->allocateArray<Game::MaterialTextureDef>(baseMaterial->textureCount);
			std::memcpy(textureTable, baseMaterial->textureTable, sizeof(Game::MaterialTextureDef) * baseMaterial->textureCount);
			material->textureTable = textureTable;
			material->textureCount = baseMaterial->textureCount;
		}

		// Load referenced textures
		auto textures = infoData["textures"];
		if (textures.is_array())
		{
			std::vector<Game::MaterialTextureDef> textureList;

			for (auto texture : textures.array_items())
			{
				if (!texture.is_array()) continue;
				if (textureList.size() >= 0xFF) break;

				auto textureInfo = texture.array_items();
				if (textureInfo.size() < 2) continue;

				auto map = textureInfo[0];
				auto image = textureInfo[1];
				if (!map.is_string() || !image.is_string()) continue;

				Game::MaterialTextureDef textureDef;

				textureDef.semantic = 0; // No water image
				textureDef.sampleState = -30;
				textureDef.nameEnd = map.string_value().back();
				textureDef.nameStart = map.string_value().front();
				textureDef.nameHash = Game::R_HashString(map.string_value().data());

				textureDef.info.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, image.string_value(), builder).image;

				if (replaceTexture)
				{
					bool applied = false;

					for (char i = 0; i < baseMaterial->textureCount; ++i)
					{
						if (material->textureTable[i].nameHash == textureDef.nameHash)
						{
							applied = true;
							material->textureTable[i].info.image = textureDef.info.image;
							break;
						}
					}

					if (!applied)
					{
						Components::Logger::Error(0, "Unable to find texture for map '%s' in %s!", map.string_value().data(), baseMaterial->name);
					}
				}
				else
				{
					textureList.push_back(textureDef);
				}
			}

			if(!replaceTexture)
			{
				if (!textureList.empty())
				{
					Game::MaterialTextureDef* textureTable = builder->getAllocator()->allocateArray<Game::MaterialTextureDef>(textureList.size());

					if (!textureTable)
					{
						Components::Logger::Error("Failed to allocate texture table!");
						return;
					}

					std::memcpy(textureTable, textureList.data(), sizeof(Game::MaterialTextureDef) * textureList.size());

					material->textureTable = textureTable;
				}
				else
				{
					material->textureTable = nullptr;
				}

				material->textureCount = static_cast<char>(textureList.size()) & 0xFF;
			}
		}

		header->material = material;
	}

	void IMaterial::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::Material* asset = header.material;

		if (asset->techniqueSet)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, asset->techniqueSet);
		}

		if (asset->textureTable)
		{
			for (char i = 0; i < asset->textureCount; ++i)
			{
				if (asset->textureTable[i].info.image)
				{
					if (asset->textureTable[i].semantic == SEMANTIC_WATER_MAP)
					{
						if (asset->textureTable[i].info.water->image)
						{
							builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].info.water->image);
						}
					}
					else
					{
						builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].info.image);
					}
				}
			}
		}
	}

	void IMaterial::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::Material, 96);

		Utils::Stream* buffer = builder->getBuffer();
		Game::Material* asset = header.material;
		Game::Material* dest = buffer->dest<Game::Material>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->techniqueSet)
		{
			dest->techniqueSet = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, asset->techniqueSet).techniqueSet;
		}

		if (asset->textureTable)
		{
			AssertSize(Game::MaterialTextureDef, 12);

			// Pointer/Offset insertion is untested, but it worked in T6, so I think it's fine
			if (builder->hasPointer(asset->textureTable))
			{
				dest->textureTable = builder->getPointer(asset->textureTable);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->textureTable);

				Game::MaterialTextureDef* destTextureTable = buffer->dest<Game::MaterialTextureDef>();
				buffer->saveArray(asset->textureTable, asset->textureCount);

				for (char i = 0; i < asset->textureCount; ++i)
				{
					Game::MaterialTextureDef* destTextureDef = &destTextureTable[i];
					Game::MaterialTextureDef* textureDef = &asset->textureTable[i];

					if (textureDef->semantic == SEMANTIC_WATER_MAP)
					{
						AssertSize(Game::water_t, 68);

						Game::water_t* destWater = buffer->dest<Game::water_t>();
						Game::water_t* water = textureDef->info.water;

						if (water)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							buffer->save(water);
							Utils::Stream::ClearPointer(&destTextureDef->info.water);

							// Save_water_t
							if (water->H0)
							{
								buffer->align(Utils::Stream::ALIGN_4);
								buffer->save(water->H0, 8, water->M * water->N);
								Utils::Stream::ClearPointer(&destWater->H0);
							}

							if (water->wTerm)
							{
								buffer->align(Utils::Stream::ALIGN_4);
								buffer->save(water->wTerm, 4, water->M * water->N);
								Utils::Stream::ClearPointer(&destWater->wTerm);
							}

							if (water->image)
							{
								destWater->image = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, water->image).image;
							}
						}
					}
					else if (textureDef->info.image)
					{
						destTextureDef->info.image = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, textureDef->info.image).image;
					}
				}

				Utils::Stream::ClearPointer(&dest->textureTable);
			}
		}

		if (asset->constantTable)
		{
			AssertSize(Game::MaterialConstantDef, 32);

			if (builder->hasPointer(asset->constantTable))
			{
				dest->constantTable = builder->getPointer(asset->constantTable);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_16);
				builder->storePointer(asset->constantTable);

				buffer->saveArray(asset->constantTable, asset->constantCount);
				Utils::Stream::ClearPointer(&dest->constantTable);
			}
		}

		if (asset->stateBitTable)
		{
			if (builder->hasPointer(asset->stateBitTable))
			{
				dest->stateBitTable = builder->getPointer(asset->stateBitTable);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->stateBitTable);

				buffer->save(asset->stateBitTable, 8, asset->stateBitsCount);
				Utils::Stream::ClearPointer(&dest->stateBitTable);
			}
		}

		buffer->popBlock();
	}
}
