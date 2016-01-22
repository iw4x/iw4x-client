#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialTechniqueSet::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::MaterialTechniqueSet* asset = header.materialTechset;

		for (int i = 0; i < ARR_SIZE(Game::MaterialTechniqueSet::techniques); i++)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (!technique) continue;

			for (short i = 0; i < technique->numPasses; i++)
			{
				Game::MaterialPass* pass = &technique->passes[i];

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
		Assert_AssetStruct(Game::MaterialTechniqueSet, 204);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialTechniqueSet* asset = header.materialTechset;
		Game::MaterialTechniqueSet* dest = (Game::MaterialTechniqueSet*)buffer->At();
		buffer->Save(asset, sizeof(Game::MaterialTechniqueSet));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		// Save_MaterialTechniquePtrArray
		static_assert(ARR_SIZE(Game::MaterialTechniqueSet::techniques) == 48, "Techniques array invalid!");

		for (int i = 0; i < ARR_SIZE(Game::MaterialTechniqueSet::techniques); i++)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (technique)
			{
				// Size-check is obsolete, as the structure is dynamic
				buffer->Align(Utils::Stream::ALIGN_4);

				Game::MaterialTechnique* destTechnique = (Game::MaterialTechnique*)buffer->At();
				buffer->Save(technique, 8);
				dest->techniques[i] = (Game::MaterialTechnique*) - 1;

				// Save_MaterialPassArray
				for (short i = 0; i < technique->numPasses; i++)
				{
					Assert_AssetStruct(Game::MaterialPass, 20);

					Game::MaterialPass* destPass = (Game::MaterialPass*)buffer->At();
					Game::MaterialPass* pass = &technique->passes[i];
					buffer->Save(pass, sizeof(Game::MaterialPass));

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
						destPass->argumentDef = (Game::ShaderArgumentDef*)-1;
					}
				}

				// We absolutely have to write something here!
				if (technique->name)
				{
					buffer->SaveString(technique->name);
				}
				else
				{
					buffer->SaveString("");
				}

				destTechnique->name = (char*)-1;
			}
		}

		buffer->PopBlock();
	}
}
