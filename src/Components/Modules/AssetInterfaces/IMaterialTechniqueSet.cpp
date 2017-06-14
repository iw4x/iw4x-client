#include "STDInclude.hpp"

namespace Assets
{
	void IMaterialTechniqueSet::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::MaterialTechniqueSet* asset = header.techniqueSet;

		for (int i = 0; i < ARRAYSIZE(Game::MaterialTechniqueSet::techniques); ++i)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (!technique) continue;

			for (short j = 0; j < technique->passCount; ++j)
			{
				Game::MaterialPass* pass = &technique->passArray[j];

				if (pass->vertexDecl)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl);
				}

				if (pass->vertexShader)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader);
				}

				if (pass->pixelShader)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader);
				}
			}
		}
	}

	void IMaterialTechniqueSet::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialTechniqueSet, 204);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MaterialTechniqueSet* asset = header.techniqueSet;
		Game::MaterialTechniqueSet* dest = buffer->dest<Game::MaterialTechniqueSet>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		// Save_MaterialTechniquePtrArray
		static_assert(ARRAYSIZE(Game::MaterialTechniqueSet::techniques) == 48, "Techniques array invalid!");

		for (int i = 0; i < ARRAYSIZE(Game::MaterialTechniqueSet::techniques); ++i)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (technique)
			{
				if (builder->hasPointer(technique))
				{
					dest->techniques[i] = builder->getPointer(technique);
				}
				else
				{
					// Size-check is obsolete, as the structure is dynamic
					buffer->align(Utils::Stream::ALIGN_4);
					builder->storePointer(technique);

					Game::MaterialTechnique* destTechnique = buffer->dest<Game::MaterialTechnique>();
					buffer->save(technique, 8);

					// Save_MaterialPassArray
					Game::MaterialPass* destPasses = buffer->dest<Game::MaterialPass>();
					buffer->saveArray(technique->passArray, technique->passCount);

					for (short j = 0; j < technique->passCount; ++j)
					{
						AssertSize(Game::MaterialPass, 20);

						Game::MaterialPass* destPass = &destPasses[j];
						Game::MaterialPass* pass = &technique->passArray[j];

						if (pass->vertexDecl)
						{
							destPass->vertexDecl = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl).vertexDecl;
						}

						if (pass->vertexShader)
						{
							destPass->vertexShader = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader).vertexShader;
						}

						if (pass->pixelShader)
						{
							destPass->pixelShader = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader).pixelShader;
						}

						if (pass->args)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							Game::MaterialShaderArgument* destArgs = buffer->dest<Game::MaterialShaderArgument>();
							buffer->saveArray(pass->args, pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount);

							for (int k = 0; k < pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount; ++k)
							{
								Game::MaterialShaderArgument* arg = &pass->args[k];
								Game::MaterialShaderArgument* destArg = &destArgs[k];

								if (arg->type == 1 || arg->type == 7)
								{
									if (builder->hasPointer(arg->u.literalConst))
									{
										destArg->u.literalConst = builder->getPointer(arg->u.literalConst);
									}
									else
									{
										buffer->align(Utils::Stream::ALIGN_4);
										builder->storePointer(arg->u.literalConst);

										buffer->saveArray(arg->u.literalConst, 4);
										Utils::Stream::ClearPointer(&destArg->u.literalConst);
									}
								}
							}

							Utils::Stream::ClearPointer(&destPass->args);
						}
					}

					if (technique->name)
					{
						buffer->saveString(technique->name);
						Utils::Stream::ClearPointer(&destTechnique->name);
					}

					Utils::Stream::ClearPointer(&dest->techniques[i]);
				}
			}
		}

		buffer->popBlock();
	}
}
