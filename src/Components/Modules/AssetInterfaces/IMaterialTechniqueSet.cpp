#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialTechniqueSet::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::MaterialTechniqueSet* asset = header.materialTechset;

		for (int i = 0; i < ARRAYSIZE(Game::MaterialTechniqueSet::techniques); ++i)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (!technique) continue;

			for (short j = 0; j < technique->numPasses; ++j)
			{
				Game::MaterialPass* pass = &technique->passes[j];

				if (pass->vertexDecl)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl->name);
				}

				if (pass->vertexShader)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader->name);
				}

				if (pass->pixelShader)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader->name);
				}
			}
		}
	}

	void IMaterialTechniqueSet::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::MaterialTechniqueSet, 204);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialTechniqueSet* asset = header.materialTechset;
		Game::MaterialTechniqueSet* dest = buffer->Dest<Game::MaterialTechniqueSet>();
		buffer->Save(asset);

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		// Save_MaterialTechniquePtrArray
		static_assert(ARRAYSIZE(Game::MaterialTechniqueSet::techniques) == 48, "Techniques array invalid!");

		for (int i = 0; i < ARRAYSIZE(Game::MaterialTechniqueSet::techniques); ++i)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (technique)
			{
				if (builder->HasPointer(technique))
				{
					dest->techniques[i] = builder->GetPointer(technique);
				}
				else
				{
					// Size-check is obsolete, as the structure is dynamic
					buffer->Align(Utils::Stream::ALIGN_4);
					builder->StorePointer(technique);

					Game::MaterialTechnique* destTechnique = buffer->Dest<Game::MaterialTechnique>();
					buffer->Save(technique, 8);

					// Save_MaterialPassArray
					Game::MaterialPass* destPasses = buffer->Dest<Game::MaterialPass>();
					buffer->SaveArray(technique->passes, technique->numPasses);

					for (short j = 0; j < technique->numPasses; ++j)
					{
						Assert_Size(Game::MaterialPass, 20);

						Game::MaterialPass* destPass = &destPasses[j];
						Game::MaterialPass* pass = &technique->passes[j];

						if (pass->vertexDecl)
						{
							destPass->vertexDecl = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl->name).vertexDecl;
						}

						if (pass->vertexShader)
						{
							destPass->vertexShader = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader->name).vertexShader;
						}

						if (pass->pixelShader)
						{
							destPass->pixelShader = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader->name).pixelShader;
						}

						if (pass->argumentDef)
						{
							buffer->Align(Utils::Stream::ALIGN_4);
							buffer->SaveArray(pass->argumentDef, pass->argCount1 + pass->argCount2 + pass->argCount3);
							Utils::Stream::ClearPointer(&destPass->argumentDef);
						}
					}

					if (technique->name)
					{
						buffer->SaveString(technique->name);
						Utils::Stream::ClearPointer(&destTechnique->name);
					}

					Utils::Stream::ClearPointer(&dest->techniques[i]);
				}
			}
		}

		buffer->PopBlock();
	}
}
