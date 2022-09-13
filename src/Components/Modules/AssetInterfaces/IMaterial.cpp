#include <STDInclude.hpp>
#include "IMaterial.hpp"

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
			{"effect", "effect_blend"},
			{"effect", "effect_blend"},
			{"effect_nofog", "effect_blend_nofog"},
			{"effect_zfeather", "effect_zfeather_blend"},

			{"wc_unlit_add", "wc_unlit_add_lin"},
			{"wc_unlit_distfalloff", "wc_unlit_distfalloff_replace"},
			{"wc_unlit_multiply", "wc_unlit_multiply_lin"},
			{"wc_unlit_falloff_add", "wc_unlit_falloff_add_lin_ua"},
			{"wc_unlit", "wc_unlit_replace_lin"},
			{"wc_unlit_alphatest", "wc_unlit_blend_lin"},
			{"wc_unlit_blend", "wc_unlit_blend_lin_ua"},
			{"wc_unlit_replace", "wc_unlit_replace_lin"},
			{"wc_unlit_nofog", "wc_unlit_replace_lin_nofog_nocast" },

			{"mc_unlit_replace", "mc_unlit_replace_lin"},
			{"mc_unlit_nofog", "mc_unlit_blend_nofog_ua"},
			{"mc_unlit", "mc_unlit_replace_lin_nocast"},
			{"mc_unlit_alphatest", "mc_unlit_blend_lin"}
			/*,
			{"", ""},
			{"", ""},
			{"", ""},
			{"", ""},
			{"", ""},*/
		};

		Components::FileSystem::File materialFile(Utils::String::VA("materials/%s.iw4xMaterial", name.data()));
		if (!materialFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), materialFile.getBuffer());

		char* magic = reader.readArray<char>(7);
		if (std::memcmp(magic, "IW4xMat", 7))
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading material '{}' failed, header is invalid!", name);
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_MAT_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading material '{}' failed, expected version is {}, but it was {}!", name, IW4X_MAT_VERSION, version);
		}

		Game::Material* asset = reader.readObject<Game::Material>();

		if (asset->info.name)
		{
			asset->info.name = reader.readCString();
		}

		if (asset->techniqueSet)
		{
			std::string techsetName = reader.readString();
			if (!techsetName.empty() && techsetName.front() == ',') techsetName.erase(techsetName.begin());
			asset->techniqueSet = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, techsetName.data(), builder).techniqueSet;

			if (!asset->techniqueSet)
			{
				// Workaround for effect techsets having _nofog suffix
				std::string suffix;
				if (Utils::String::StartsWith(techsetName, "effect_") && Utils::String::EndsWith(techsetName, "_nofog"))
				{
					suffix = "_nofog";
					Utils::String::Replace(techsetName, suffix, "");
				}

				for (int i = 0; i < ARRAYSIZE(techsetSuffix); ++i)
				{
					Game::MaterialTechniqueSet* techsetPtr = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, (techsetName + techsetSuffix[i] + suffix).data(), builder).techniqueSet;

					if (techsetPtr)
					{
						asset->techniqueSet = techsetPtr;

						if (asset->techniqueSet->name[0] == ',') continue; // Try to find a better one
						Components::Logger::Print("Techset '{}' has been mapped to '{}'\n", techsetName, asset->techniqueSet->name);
						break;
					}
				}
			}
			else
			{
				Components::Logger::Print("Techset {} exists with the same name in iw4, and was mapped 1:1 with {}\n", techsetName, asset->techniqueSet->name);
			}

			if (!asset->techniqueSet)
			{
				Components::Logger::Error(Game::ERR_FATAL, "Missing techset: '{}' not found", techsetName);
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
					Components::Logger::Print("For {}, copied constants & statebits from {}\n", asset->info.name, header.material->info.name);
					replacementFound = true;
				}
			}
		}, false);

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

					// Apparently, this is really not that important
					//if (t1->techniques[i]->flags != t2->techniques[i]->flags) return false; 
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
						Components::Logger::Print("Material {} with techset {} has been mapped to {}\n", asset->info.name, asset->techniqueSet->name, header.material->techniqueSet->name);
						asset->info.sortKey = header.material->info.sortKey;
						replacementFound = true;
					}
				}
			}, false);
		}

		if (!replacementFound && asset->techniqueSet)
		{
			Components::Logger::Print("No replacement found for material {} with techset {}\n", asset->info.name, asset->techniqueSet->name);
			std::string techName = asset->techniqueSet->name;
			if (techSetCorrespondance.contains(techName))
			{
				auto iw4TechSetName = techSetCorrespondance[techName];
				Game::XAssetEntry* iw4TechSet = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, iw4TechSetName.data());

				if (iw4TechSet) 
				{
					Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [asset, iw4TechSet](Game::XAssetEntry* entry)
					{
						if (!replacementFound)
						{
							Game::XAssetHeader header = entry->asset.header;

							if (header.material->techniqueSet == iw4TechSet->asset.header.techniqueSet 
								&& std::string(header.material->info.name).find("icon") == std::string::npos) // Yeah this has a tendency to fuck up a LOT of transparent materials
							{
								Components::Logger::Print("Material {} with techset {} has been mapped to {} (last chance!), taking the sort key of material {}\n",
									asset->info.name, asset->techniqueSet->name, header.material->techniqueSet->name, header.material->info.name);

								asset->info.sortKey = header.material->info.sortKey;
								asset->techniqueSet = iw4TechSet->asset.header.techniqueSet;

								// this is terrible!
								asset->stateBitsCount = header.material->stateBitsCount;
								asset->stateBitsTable = header.material->stateBitsTable;
								std::memcpy(asset->stateBitsEntry, header.material->stateBitsEntry, 48);
								asset->constantCount = header.material->constantCount;
								asset->constantTable = header.material->constantTable;

								replacementFound = true;
							}
						}
					}, false);

					if (!replacementFound) 
					{
						Components::Logger::Print("Could not find any loaded material with techset {} (in replacement of {}), so I cannot set the sortkey for material {}\n", iw4TechSetName, asset->techniqueSet->name, asset->info.name);
					}
				}
				else 
				{
					Components::Logger::Print("Could not find any loaded techset with iw4 name {} for iw3 techset {}\n", iw4TechSetName, asset->techniqueSet->name);
				}
			}
			else 
			{
				Components::Logger::Print("Could not match iw3 techset {} with any of the techsets I know! This is a critical error, there's a good chance the map will not be playable.\n", techName);
			}
		}

		if (!reader.end())
		{
			Components::Logger::Error(Game::ERR_FATAL, "Material data left!");
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

	void IMaterial::loadJson(Game::XAssetHeader* header, const std::string& name, [[maybe_unused]] Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File materialInfo(Utils::String::VA("materials/%s.json", name.data()));

		if (!materialInfo.exists()) return;

		header->material = nullptr;
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
