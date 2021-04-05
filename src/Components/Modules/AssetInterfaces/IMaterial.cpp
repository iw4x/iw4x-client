#include "STDInclude.hpp"

#define IW4X_MAT_VERSION "1"

namespace Assets
{
	void IMaterial::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadJson(header, name, builder);   // Check if we want to override materials
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
	}

	void IMaterial::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		static const char* techsetSuffix[] =
		{
			"_lin",
			"_add_lin",
			"_replace",
			"_eyeoffset",

			"_blend",
			"_blend_nofog",
			"_add",
			"_nofog",
			"_nocast",

			"_add_lin_nofog",
		};

		std::map<std::string, std::string> techSetCorrespondance = {
			{"effect_zfeather_outdoor", "effect_zfeather_blend"},
			{"effect", "effect_blend"},
			{"effect_nofog", "effect_blend_nofog"}
		};

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

		if (asset->info.name)
		{
			asset->info.name = reader.readCString();
		}

		if (asset->techniqueSet)
		{
			std::string techset = reader.readString();
			if (!techset.empty() && techset.front() == ',') techset.erase(techset.begin());
			asset->techniqueSet = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, techset.data(), builder).techniqueSet;

			if (!asset->techniqueSet)
			{
				// Workaround for effect techsets having _nofog suffix
				std::string suffix;
				if (Utils::String::StartsWith(techset, "effect_") && Utils::String::EndsWith(techset, "_nofog"))
				{
					suffix = "_nofog";
					Utils::String::Replace(techset, suffix, "");
				}

				for (int i = 0; i < ARRAYSIZE(techsetSuffix); ++i)
				{
					Game::MaterialTechniqueSet* techsetPtr = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, (techset + techsetSuffix[i] + suffix).data(), builder).techniqueSet;

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
				Components::Logger::Error("Missing techset: '%s' not found", techset.data());
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
					if (textureDef->u.water)
					{
						Game::water_t* water = reader.readObject<Game::water_t>();
						textureDef->u.water = water;

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
				else if (textureDef->u.image)
				{
					textureDef->u.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, reader.readString().data(), builder).image;
				}
			}
		}

		if (asset->constantTable)
		{
			asset->constantTable = reader.readArray<Game::MaterialConstantDef>(asset->constantCount);
		}

		if (asset->stateBitsTable)
		{
			asset->stateBitsTable = reader.readArray<Game::GfxStateBits>(asset->stateBitsCount);
		}

		header->material = asset;

		static thread_local bool replacementFound;
		replacementFound = false;

		// Find correct sortkey by comparing techsets
		Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [asset](Game::XAssetEntry* entry)
		{
			if (!replacementFound)
			{
				Game::XAssetHeader header = entry->asset.header;

				const char* name = asset->techniqueSet->name;
				if (name[0] == ',') ++name;

				if (std::string(name) == header.material->techniqueSet->name)
				{
					asset->info.sortKey = header.material->info.sortKey;

					// This is temp, as nobody has time to fix materials
					asset->stateBitsCount = header.material->stateBitsCount;
					asset->stateBitsTable = header.material->stateBitsTable;
					std::memcpy(asset->stateBitsEntry, header.material->stateBitsEntry, 48);
					asset->constantCount = header.material->constantCount;
					asset->constantTable = header.material->constantTable;

					replacementFound = true;
				}
			}
		}, false, false);

		if (!replacementFound)
		{
			auto techsetMatches = [](Game::Material* m1, Game::Material* m2)
			{
				Game::MaterialTechniqueSet* t1 = m1->techniqueSet;
				Game::MaterialTechniqueSet* t2 = m2->techniqueSet;
				if (!t1 || !t2) return false;
				if (t1->remappedTechniqueSet && t2->remappedTechniqueSet && std::string(t1->remappedTechniqueSet->name) == t2->remappedTechniqueSet->name) return true;

				for (int i = 0; i < ARRAYSIZE(t1->techniques); ++i)
				{
					if (!t1->techniques[i] && !t2->techniques[i]) continue;;
					if (!t1->techniques[i] || !t2->techniques[i]) return false;

					if (t1->techniques[i]->flags != t2->techniques[i]->flags) return false;
				}

				return true;
			};


			Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [asset, techsetMatches](Game::XAssetEntry* entry)
			{
				if (!replacementFound)
				{
					Game::XAssetHeader header = entry->asset.header;

					if (techsetMatches(header.material, asset))
					{
						Components::Logger::Print("Material %s with techset %s has been mapped to %s\n", asset->info.name, asset->techniqueSet->name, header.material->techniqueSet->name);
						asset->info.sortKey = header.material->info.sortKey;
						replacementFound = true;
					}
				}
			}, false, false);
		}

		if (!replacementFound && asset->techniqueSet)
		{
			Components::Logger::Print("No replacement found for material %s with techset %s\n", asset->info.name, asset->techniqueSet->name);
			std::string techName = asset->techniqueSet->name;
			if (techSetCorrespondance.find(techName) != techSetCorrespondance.end()) {
				auto iw4TechSetName = techSetCorrespondance[techName];
				Game::XAssetEntry* entry = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, iw4TechSetName.data());

				if (entry) {
					asset->techniqueSet = entry->asset.header.techniqueSet;

					Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [asset](Game::XAssetEntry* entry)
						{
							if (!replacementFound)
							{
								Game::XAssetHeader header = entry->asset.header;

								if (header.material->techniqueSet == asset->techniqueSet)
								{
									Components::Logger::Print("Material %s with techset %s has been mapped (last chance!) to %s\n", asset->info.name, asset->techniqueSet->name, header.material->techniqueSet->name);
									asset->info.sortKey = header.material->info.sortKey;
									replacementFound = true;
								}
							}
						}, false, false);

					if (!replacementFound) {
						Components::Logger::Print("Could not find any loaded material with techset %s, so I cannot set the sortkey for material %s\n", asset->techniqueSet->name,  asset->info.name);
					}
				}
				else {
					Components::Logger::Print("Could not find any loaded techset with iw4 name %s for iw3 techset %s\n", iw4TechSetName.data(), asset->techniqueSet->name);
				}
			}
			else {
				Components::Logger::Print("Could not match iw3 techset %s with any of the techsets I know! This is a critical error, the map will not be playable.\n", asset->techniqueSet->name);
			}

		}

		if (!reader.end())
		{
			Components::Logger::Error("Material data left!");
		}

		/*char baseIndex = 0;
		for (char i = 0; i < asset->stateBitsCount; ++i)
		{
			auto stateBits = asset->stateBitsTable[i];
			if (stateBits.loadBits[0] == 0x18128812 &&
				stateBits.loadBits[1] == 0xD) // Seems to be like a default stateBit causing a 'generic' initialization
			{
				baseIndex = i;
				break;
			}
		}

		for (int i = 0; i < 48; ++i)
		{
			if (!asset->techniqueSet->techniques[i] && asset->stateBitsEntry[i] != -1)
			{
				asset->stateBitsEntry[i] = -1;
			}

			if (asset->techniqueSet->techniques[i] && asset->stateBitsEntry[i] == -1)
			{
				asset->stateBitsEntry[i] = baseIndex;
			}
		}*/
	}

	void IMaterial::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->material = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).material;
	}

	void IMaterial::loadJson(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
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
		material->info.name = builder->getAllocator()->duplicateString(name);

		material->info.textureAtlasRowCount = 1;
		material->info.textureAtlasColumnCount = 1;

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
					material->info.textureAtlasColumnCount = static_cast<char>(animCoordX.number_value()) & 0xFF;
				}

				if (animCoordY.is_number())
				{
					material->info.textureAtlasRowCount = static_cast<char>(animCoordY.number_value()) & 0xFF;
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

			for (auto& texture : textures.array_items())
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
				textureDef.samplerState = -30;
				textureDef.nameEnd = map.string_value().back();
				textureDef.nameStart = map.string_value().front();
				textureDef.nameHash = Game::R_HashString(map.string_value().data());

				textureDef.u.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, image.string_value(), builder).image;

				if (replaceTexture)
				{
					bool applied = false;

					for (char i = 0; i < baseMaterial->textureCount; ++i)
					{
						if (material->textureTable[i].nameHash == textureDef.nameHash)
						{
							applied = true;
							material->textureTable[i].u.image = textureDef.u.image;
							break;
						}
					}

					if (!applied)
					{
						Components::Logger::Error(0, "Unable to find texture for map '%s' in %s!", map.string_value().data(), baseMaterial->info.name);
					}
				}
				else
				{
					textureList.push_back(textureDef);
				}
			}

			if (!replaceTexture)
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
				if (asset->textureTable[i].u.image)
				{
					if (asset->textureTable[i].semantic == SEMANTIC_WATER_MAP)
					{
						if (asset->textureTable[i].u.water->image)
						{
							builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].u.water->image);
						}
					}
					else
					{
						builder->loadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].u.image);
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

		if (asset->info.name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->info.name));
			Utils::Stream::ClearPointer(&dest->info.name);
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
						Game::water_t* water = textureDef->u.water;

						if (water)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							buffer->save(water);
							Utils::Stream::ClearPointer(&destTextureDef->u.water);

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
					else if (textureDef->u.image)
					{
						destTextureDef->u.image = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_IMAGE, textureDef->u.image).image;
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

		if (asset->stateBitsTable)
		{
			if (builder->hasPointer(asset->stateBitsTable))
			{
				dest->stateBitsTable = builder->getPointer(asset->stateBitsTable);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->stateBitsTable);

				buffer->save(asset->stateBitsTable, 8, asset->stateBitsCount);
				Utils::Stream::ClearPointer(&dest->stateBitsTable);
			}
		}

		buffer->popBlock();
	}
}
