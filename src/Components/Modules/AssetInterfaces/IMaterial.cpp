#include <STDInclude.hpp>

namespace Assets
{
	void IMaterial::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::Material* asset = header.material;

		if (asset->techniqueSet)
		{
			builder->LoadAsset(Game::XAssetType::ASSET_TYPE_TECHSET, asset->techniqueSet->name);
		}

		if (asset->textureTable)
		{
			for (char i = 0; i < asset->textureCount; i++)
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
		Assert_AssetStruct(Game::Material, 96);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::Material* asset = header.material;
		Game::Material* dest = (Game::Material*)buffer->At();
		buffer->Save(asset, sizeof(Game::Material));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		if (asset->techniqueSet)
		{
			dest->techniqueSet = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_TECHSET, asset->techniqueSet->name).materialTechset;
		}

		if (asset->textureTable)
		{
			Assert_AssetStruct(Game::MaterialTextureDef, 12);

			// Pointer/Offset insertion is untested, but it worked in T6, so I think it's fine
			if (builder->HasPointer(asset->textureTable))
			{
				dest->textureTable = builder->GetPointer(asset->textureTable);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_4);
				builder->StorePointer(asset->textureTable);

				Game::MaterialTextureDef* destTextureTable = (Game::MaterialTextureDef*)buffer->At();
				buffer->SaveArray(asset->textureTable, asset->textureCount);

				for (char i = 0; i < asset->textureCount; i++)
				{
					Game::MaterialTextureDef* destTextureDef = &destTextureTable[i];
					Game::MaterialTextureDef* textureDef = &asset->textureTable[i];

					if (textureDef->semantic == SEMANTIC_WATER_MAP)
					{
						Assert_AssetStruct(Game::water_t, 68);

						Game::water_t* destWater = (Game::water_t*)buffer->At();
						Game::water_t* water = textureDef->info.water;

						if (water)
						{
							buffer->Align(Utils::Stream::ALIGN_4);
							buffer->Save(water, sizeof(Game::water_t));
							destTextureDef->info.water = (Game::water_t *) - 1;

							// Save_water_t
							if (water->H0X)
							{
								buffer->Align(Utils::Stream::ALIGN_4);
								buffer->Save(water->H0X, 8, water->M * water->N);
								destWater->H0X = (float *)-1;
							}

							if (water->H0Y)
							{
								buffer->Align(Utils::Stream::ALIGN_4);
								buffer->Save(water->H0Y, 4, water->M * water->N);
								destWater->H0Y = (float *)-1;
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

				dest->textureTable = (Game::MaterialTextureDef*) - 1;
			}
		}

		if (asset->constantTable)
		{
			Assert_AssetStruct(Game::MaterialConstantDef, 32);

			if (builder->HasPointer(asset->constantTable))
			{
				dest->constantTable = builder->GetPointer(asset->constantTable);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_16);
				builder->StorePointer(asset->constantTable);

				buffer->SaveArray(asset->constantTable, asset->constantCount);
				dest->constantTable = (Game::MaterialConstantDef *) - 1;
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
				dest->stateBitTable = (void *)-1;
			}
		}

		buffer->PopBlock();
	}
}
