#include <STDInclude.hpp>
#include "IMaterialTechniqueSet.hpp"

#define IW4X_TECHSET_VERSION "0"

namespace Assets
{
	void IMaterialTechniqueSet::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
	}

	void IMaterialTechniqueSet::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->techniqueSet = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).techniqueSet;
	}

	void IMaterialTechniqueSet::loadBinaryTechnique(Game::MaterialTechnique** tech, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialPass, 20);

		Components::FileSystem::File techFile(Utils::String::VA("techniques/%s.iw4xTech", name.data()));
		if (!techFile.exists()) {
			*tech = nullptr;
			Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Missing technique '{}'\n", name);
			return;
		}

		Utils::Stream::Reader reader(builder->getAllocator(), techFile.getBuffer());

		char* magic = reader.readArray<char>(8);
		if (std::memcmp(magic, "IW4xTECH", 8))
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading technique '{}' failed, header is invalid!", name);
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL,
				"Reading technique '{}' failed, expected version is {}, but it was {}!", name, IW4X_TECHSET_VERSION, version.data());
		}

		unsigned short flags = reader.read<unsigned short>();
		unsigned short passCount = reader.read<unsigned short>();

		Game::MaterialTechnique* asset = (Game::MaterialTechnique*)builder->getAllocator()->allocateArray<unsigned char>(sizeof(Game::MaterialTechnique) + (sizeof(Game::MaterialPass) * (passCount - 1)));

		asset->name = builder->getAllocator()->duplicateString(name);
		asset->flags = flags;
		asset->passCount = passCount;

		Game::MaterialPass* passes = reader.readArray<Game::MaterialPass>(passCount);
		std::memcpy(asset->passArray, passes, sizeof(Game::MaterialPass) * passCount);

		for (unsigned short i = 0; i < asset->passCount; i++)
		{ 
			Game::MaterialPass* pass = &asset->passArray[i];

			if (pass->vertexDecl)
			{
				const char* declName = reader.readCString();
				pass->vertexDecl = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_VERTEXDECL, declName, builder).vertexDecl;
			}

			if (pass->vertexShader)
			{
				const char* vsName = reader.readCString();
				pass->vertexShader = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, vsName, builder).vertexShader;

			}

			if (pass->pixelShader)
			{
				const char* psName = reader.readCString();
				pass->pixelShader = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PIXELSHADER, psName, builder).pixelShader;
			}

			pass->args = reader.readArray<Game::MaterialShaderArgument>(pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount);

			for (int j = 0; j < pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount; j++)
			{
				if (pass->args[j].type == 1 || pass->args[j].type == 7)
				{
					pass->args[j].u.literalConst = reader.readArray<float>(4);
				}

				if (pass->args[j].type == 3 || pass->args[j].type == 5)
				{
					pass->args[j].u.codeConst.index = *reader.readObject<unsigned short>();
					pass->args[j].u.codeConst.firstRow = *reader.readObject<unsigned char>();
					pass->args[j].u.codeConst.rowCount = *reader.readObject<unsigned char>();
				}
			}
		}

		*tech = asset;
	}

	void IMaterialTechniqueSet::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File tsFile(Utils::String::VA("techsets/%s.iw4xTS", name.data()));
		if (!tsFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), tsFile.getBuffer());

		char* magic = reader.readArray<char>(8);
		if (std::memcmp(magic, "IW4xTSET", 8))
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, header is invalid!", name);
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, expected version is {}, but it was {}!",
				name, IW4X_TECHSET_VERSION, version);
		}

		Game::MaterialTechniqueSet* asset = reader.readObject<Game::MaterialTechniqueSet>();

		if (asset->name)
		{
			asset->name = reader.readCString();
		}

		for (int i = 0; i < 48; i++)
		{
			if (asset->techniques[i])
			{
				const char* techName = reader.readCString();
				this->loadBinaryTechnique(&asset->techniques[i], techName, builder);
			}
		}


		header->techniqueSet = asset;
	}

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
