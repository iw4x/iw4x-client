#include <STDInclude.hpp>
#include "IMaterialTechniqueSet.hpp"

#include <Utils/Json.hpp>

#define IW4X_TECHSET_VERSION 1

namespace Assets
{
	void IMaterialTechniqueSet::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadFromDisk(header, name, builder); // Check if we need to import a new one into the game
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
	}

	void IMaterialTechniqueSet::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->techniqueSet = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).techniqueSet;
	}

	void IMaterialTechniqueSet::loadTechniqueFromDisk(Game::MaterialTechnique** tech, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialPass, 20);

		Components::FileSystem::File techFile(std::format("techniques/{}.iw4x.json", name));
		if (!techFile.exists())
		{
			*tech = nullptr;

			Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Missing technique '{}'\n", name);
			return;
		}

		nlohmann::json technique;

		try
		{
			technique = nlohmann::json::parse(techFile.getBuffer());
		}
		catch (std::exception& e)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, file is messed up! {}", name, e.what());
		}

		int version = technique["version"].get<int>();

		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL,
				"Reading technique '{}' failed, expected version is {}, but it was {}!", name, IW4X_TECHSET_VERSION, version);
		}

		unsigned short flags = static_cast<unsigned short>(Utils::Json::ReadFlags(technique["flags"].get<std::string>(), sizeof(short)));

		if (technique["passArray"].is_array())
		{
			nlohmann::json::array_t passArray = technique["passArray"];

			Game::MaterialTechnique* asset = (Game::MaterialTechnique*)builder->getAllocator()->allocateArray<unsigned char>(sizeof(Game::MaterialTechnique) + (sizeof(Game::MaterialPass) * (passArray.size() - 1)));

			asset->name = builder->getAllocator()->duplicateString(name);
			asset->flags = flags;
			asset->passCount = static_cast<unsigned short>(passArray.size());

			Game::MaterialPass* passes = builder->getAllocator()->allocateArray<Game::MaterialPass>(asset->passCount);
			std::memcpy(asset->passArray, passes, sizeof(Game::MaterialPass) * asset->passCount);

			for (unsigned short i = 0; i < asset->passCount; i++)
			{
				Game::MaterialPass* pass = &asset->passArray[i];
				auto jsonPass = passArray[i];

				if (jsonPass["vertexDeclaration"].is_string())
				{
					auto declName = jsonPass["vertexDeclaration"].get<std::string>();
					pass->vertexDecl = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_VERTEXDECL, declName, builder).vertexDecl;
				}

				if (jsonPass["vertexShader"].is_string())
				{
					auto vsName = jsonPass["vertexShader"].get<std::string>();
					pass->vertexShader = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, vsName, builder).vertexShader;
				}

				if (jsonPass["pixelShader"].is_string())
				{
					auto psName = jsonPass["pixelShader"].get<std::string>();
					pass->pixelShader = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_PIXELSHADER, psName, builder).pixelShader;
				}

				pass->perPrimArgCount = jsonPass["perPrimArgCount"].get<char>();
				pass->perObjArgCount = jsonPass["perObjArgCount"].get<char>();
				pass->stableArgCount = jsonPass["stableArgCount"].get<char>();
				pass->customSamplerFlags = jsonPass["customSamplerFlags"].get<char>();


				if (jsonPass["arguments"].is_array())
				{
					nlohmann::json::array_t jsonAguments = jsonPass["arguments"];

					pass->args = builder->getAllocator()->allocateArray<Game::MaterialShaderArgument>(jsonAguments.size());

					for (size_t j = 0; j < jsonAguments.size(); j++)
					{
						auto jsonArgument = jsonAguments[j];
						Game::MaterialShaderArgument* argument = &pass->args[j];

						argument->type = jsonArgument["type"].get<Game::MaterialShaderArgumentType>();
						argument->dest = jsonArgument["dest"].get<unsigned short>();

						if (argument->type == Game::MaterialShaderArgumentType::MTL_ARG_LITERAL_VERTEX_CONST ||
							argument->type == Game::MaterialShaderArgumentType::MTL_ARG_LITERAL_PIXEL_CONST)
						{
							argument->u.literalConst = builder->getAllocator()->allocateArray<float>(4);

							auto literals = jsonArgument["literals"].get<std::vector<float>>();
							std::copy(literals.begin(), literals.end(), argument->u.literalConst);
						}
						else if (argument->type == Game::MaterialShaderArgumentType::MTL_ARG_CODE_VERTEX_CONST ||
							argument->type == Game::MaterialShaderArgumentType::MTL_ARG_CODE_PIXEL_CONST)
						{
							if (jsonArgument["codeConst"].is_object())
							{
								auto codeConst = jsonArgument["codeConst"];

								argument->u.codeConst.index = codeConst["index"].get<unsigned short>();
								argument->u.codeConst.firstRow = codeConst["firstRow"].get<unsigned char>();
								argument->u.codeConst.rowCount = codeConst["rowCount"].get<unsigned char>();
							}
						}
						else if (argument->type == Game::MaterialShaderArgumentType::MTL_ARG_MATERIAL_PIXEL_SAMPLER ||
							argument->type == Game::MaterialShaderArgumentType::MTL_ARG_MATERIAL_VERTEX_CONST ||
							argument->type == Game::MaterialShaderArgumentType::MTL_ARG_MATERIAL_PIXEL_CONST)
						{
							argument->u.nameHash = jsonArgument["nameHash"].get<unsigned int>();
						}
						else if (argument->type == Game::MaterialShaderArgumentType::MTL_ARG_CODE_PIXEL_SAMPLER)
						{
							argument->u.codeSampler = jsonArgument["codeSampler"].get<unsigned int>();
						}
					}
				}
			}

			*tech = asset;
		}
	}

	void IMaterialTechniqueSet::loadFromDisk(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File tsFile(std::format("techsets/{}.iw4x.json", name));
		if (!tsFile.exists()) return;

		nlohmann::json techset;

		try
		{
			techset = nlohmann::json::parse(tsFile.getBuffer());
		}
		catch (std::exception& e)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, file is messed up! {}", name, e.what());
		}

		auto version = techset["version"].get<int>();
		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, expected version is {}, but it was {}!",
				name, IW4X_TECHSET_VERSION, version);
		}

		Game::MaterialTechniqueSet* asset = builder->getAllocator()->allocate<Game::MaterialTechniqueSet>();

		if (asset == nullptr)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading techset '{}' failed, allocation failed!", name);
			return;
		}

		if (techset["name"].is_string())
		{
			asset->name = builder->getAllocator()->duplicateString(techset["name"].get<std::string>());
		}

		asset->hasBeenUploaded = techset["hasBeenUploaded"].get<bool>();
		asset->worldVertFormat = techset["worldVertFormat"].get<char>();


		if (techset["remappedTechniqueSet"].is_string())
		{
			auto remapped = techset["remappedTechniqueSet"].get<std::string>();

			if (remapped != asset->name)
			{
				builder->loadAssetByName(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, remapped, false);
			}
		}

		if (techset["techniques"].is_object())
		{
			for (int i = 0; i < Game::TECHNIQUE_COUNT; i++)
			{
				auto technique = techset["techniques"].at(std::to_string(i));

				if (technique.is_string())
				{
					this->loadTechniqueFromDisk(&asset->techniques[i], technique.get<std::string>(), builder);
				}
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
