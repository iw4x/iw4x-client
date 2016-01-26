#include <STDInclude.hpp>

namespace Assets
{
	void IMaterial::Load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File materialInfo(Utils::VA("materials/%s.json", name.data()));

		if (!materialInfo.Exists()) return;

		std::string errors;
		json11::Json infoData = json11::Json::parse(materialInfo.GetBuffer(), errors);

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

		Game::Material* material = builder->GetAllocator()->AllocateArray<Game::Material>();

		if (!material)
		{
			Components::Logger::Error("Failed to allocate material structure!");
			return;
		}

		// Copy base material to our structure
		memcpy(material, baseMaterial, sizeof(Game::Material));
		material->name = builder->GetAllocator()->DuplicateString(name);

		material->textureAtlasRowCount = 0;
		material->textureAtlasColumnCount = 0;

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
				if(!map.is_string() || !image.is_string()) continue;

				Game::MaterialTextureDef textureDef;

				textureDef.semantic = 0; // No water image
				textureDef.sampleState = -30;
				textureDef.nameEnd = map.string_value().data()[map.string_value().size() - 1];
				textureDef.nameStart = map.string_value().data()[0];
				textureDef.nameHash = Game::R_HashString(map.string_value().data());

				textureDef.info.image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, image.string_value(), builder).image;

				textureList.push_back(textureDef);
			}

			if (textureList.size())
			{
				Game::MaterialTextureDef* textureTable = builder->GetAllocator()->AllocateArray<Game::MaterialTextureDef>(textureList.size());

				if (!textureTable)
				{
					Components::Logger::Error("Failed to allocate texture table!");
					return;
				}

				memcpy(textureTable, textureList.data(), sizeof(Game::MaterialTextureDef) * textureList.size());

				material->textureTable = textureTable;
			}
			else
			{
				material->textureTable = 0;
			}

			material->textureCount = static_cast<char>(textureList.size()) & 0xFF;
		}

		header->material = material;
	}

	void IMaterial::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::Material* asset = header.material;

		if (asset->techniqueSet)
		{
			builder->LoadAsset(Game::XAssetType::ASSET_TYPE_TECHSET, asset->techniqueSet->name);
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
							builder->LoadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].info.water->image->name);
						}
					}
					else
					{
						builder->LoadAsset(Game::XAssetType::ASSET_TYPE_IMAGE, asset->textureTable[i].info.image->name);
					}
				}
			}
		}
	}

	void IMaterial::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::Material, 96);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::Material* asset = header.material;
		Game::Material* dest = buffer->Dest<Game::Material>();
		buffer->Save(asset, sizeof(Game::Material));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->techniqueSet)
		{
			dest->techniqueSet = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_TECHSET, asset->techniqueSet->name).materialTechset;
		}

		if (asset->textureTable)
		{
			Assert_Size(Game::MaterialTextureDef, 12);

			// Pointer/Offset insertion is untested, but it worked in T6, so I think it's fine
			if (builder->HasPointer(asset->textureTable))
			{
				dest->textureTable = builder->GetPointer(asset->textureTable);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_4);
				builder->StorePointer(asset->textureTable);

				Game::MaterialTextureDef* destTextureTable = buffer->Dest<Game::MaterialTextureDef>();
				buffer->SaveArray(asset->textureTable, asset->textureCount);

				for (char i = 0; i < asset->textureCount; ++i)
				{
					Game::MaterialTextureDef* destTextureDef = &destTextureTable[i];
					Game::MaterialTextureDef* textureDef = &asset->textureTable[i];

					if (textureDef->semantic == SEMANTIC_WATER_MAP)
					{
						Assert_Size(Game::water_t, 68);

						Game::water_t* destWater = buffer->Dest<Game::water_t>();
						Game::water_t* water = textureDef->info.water;

						if (water)
						{
							buffer->Align(Utils::Stream::ALIGN_4);
							buffer->Save(water, sizeof(Game::water_t));
							destTextureDef->info.water = reinterpret_cast<Game::water_t*>(-1);

							// Save_water_t
							if (water->H0X)
							{
								buffer->Align(Utils::Stream::ALIGN_4);
								buffer->Save(water->H0X, 8, water->M * water->N);
								destWater->H0X = reinterpret_cast<float*>(-1);
							}

							if (water->H0Y)
							{
								buffer->Align(Utils::Stream::ALIGN_4);
								buffer->Save(water->H0Y, 4, water->M * water->N);
								destWater->H0Y = reinterpret_cast<float*>(-1);
							}

							if (water->image)
							{
								destWater->image = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, water->image->name).image;
							}
						}
					}
					else if (textureDef->info.image)
					{
						destTextureDef->info.image = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_IMAGE, textureDef->info.image->name).image;
					}
				}

				dest->textureTable = reinterpret_cast<Game::MaterialTextureDef*>(-1);
			}
		}

		if (asset->constantTable)
		{
			Assert_Size(Game::MaterialConstantDef, 32);

			if (builder->HasPointer(asset->constantTable))
			{
				dest->constantTable = builder->GetPointer(asset->constantTable);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_16);
				builder->StorePointer(asset->constantTable);

				buffer->SaveArray(asset->constantTable, asset->constantCount);
				dest->constantTable = reinterpret_cast<Game::MaterialConstantDef*>(-1);
			}
		}

		if (asset->stateBitTable)
		{
			if (builder->HasPointer(asset->stateBitTable))
			{
				dest->stateBitTable = builder->GetPointer(asset->stateBitTable);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_4);
				builder->StorePointer(asset->stateBitTable);

				buffer->Save(asset->stateBitTable, 8, asset->stateBitsCount);
				dest->stateBitTable = reinterpret_cast<void*>(-1);
			}
		}

		buffer->PopBlock();
	}
}
