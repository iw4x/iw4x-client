#include <STDInclude.hpp>
#include "IMaterial.hpp"

#include <Utils/Json.hpp>

#define IW4X_MAT_BIN_VERSION "1"
#define IW4X_MAT_JSON_VERSION 1

namespace Assets
{
	const std::unordered_map<std::string, std::string> techSetCorrespondance =
	{
		{"effect", "effect_blend"},
		{"effect", "effect_blend"},
		{"effect_nofog", "effect_blend_nofog"},
		{"effect_zfeather", "effect_zfeather_blend"},
		{"effect_zfeather_falloff", "effect_zfeather_falloff_add"},
		{"effect_zfeather_nofog", "effect_zfeather_add_nofog"},

		{"wc_unlit_add", "wc_unlit_add_lin"},
		{"wc_unlit_distfalloff", "wc_unlit_distfalloff_replace"},
		{"wc_unlit_multiply", "wc_unlit_multiply_lin"},
		{"wc_unlit_falloff_add", "wc_unlit_falloff_add_lin"},
		{"wc_unlit", "wc_unlit_replace_lin"},
		{"wc_unlit_alphatest", "wc_unlit_blend_lin"},
		{"wc_unlit_blend", "wc_unlit_blend_lin"},
		{"wc_unlit_replace", "wc_unlit_replace_lin"},
		{"wc_unlit_nofog", "wc_unlit_replace_lin_nofog_nocast" },

		{"mc_unlit_replace", "mc_unlit_replace_lin"},
		{"mc_unlit_nofog", "mc_unlit_blend_nofog_ua"},
		{"mc_unlit", "mc_unlit_replace_lin_nocast"},
		{"mc_unlit_alphatest", "mc_unlit_blend_lin"},
		{"mc_effect_nofog", "mc_effect_blend_nofog"},
		{"mc_effect_falloff_add_nofog", "mc_effect_falloff_add_nofog_eyeoffset"},
	};

	void IMaterial::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadJson(header, name, builder);   // Check if we want to load a material from disk
		if (!header->data) this->loadBinary(header, name, builder); // Check if we want to load a material from disk (binary format)
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one

		assert(header->data);
	}


	void IMaterial::loadJson(Game::XAssetHeader* header, const std::string& name, [[maybe_unused]] Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File materialInfo(std::format("materials/{}.iw4x.json", name));

		if (!materialInfo.exists()) return;

		Game::Material* asset = builder->getAllocator()->allocate<Game::Material>();


		nlohmann::json materialJson;
		try
		{
			materialJson = nlohmann::json::parse(materialInfo.getBuffer());
		}
		catch (const std::exception& e)
		{
			Components::Logger::Print("Invalid material json for {} (broken json {})\n", name, e.what());
		}

		if (!materialJson.is_object())
		{
			Components::Logger::Print("Invalid material json for {} (Is it zonebuilder format?)\n", name);
			return;
		}

		if (materialJson["version"].get<int>() != IW4X_MAT_JSON_VERSION)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid material json version for {}, expected {} and got {}\n", name, IW4X_MAT_JSON_VERSION, materialJson["version"].get<std::string>());
			return;
		}

		try
		{
			asset->info.name = builder->getAllocator()->duplicateString(materialJson["name"].get<std::string>());
			asset->info.gameFlags = static_cast<char>(Utils::Json::ReadFlags(materialJson["gameFlags"].get<std::string>(), sizeof(char)));

			asset->info.sortKey = materialJson["sortKey"].get<char>();
			// * We do techset later * //
			asset->info.textureAtlasRowCount = materialJson["textureAtlasRowCount"].get<unsigned char>();
			asset->info.textureAtlasColumnCount = materialJson["textureAtlasColumnCount"].get<unsigned char>();
			asset->info.surfaceTypeBits = static_cast<unsigned int>(Utils::Json::ReadFlags(materialJson["surfaceTypeBits"].get<std::string>(), sizeof(int)));
			asset->info.hashIndex = materialJson["hashIndex"].get<unsigned short>();
			asset->cameraRegion = materialJson["cameraRegion"].get<char>();
		}
		catch (const nlohmann::json::exception& e)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid material json for {} (broken json {})\n", name, e.what());
			return;
		}

		if (materialJson["gfxDrawSurface"].is_object())
		{
			asset->info.drawSurf.fields.customIndex = materialJson["gfxDrawSurface"]["customIndex"].get<long long>();
			asset->info.drawSurf.fields.hasGfxEntIndex = materialJson["gfxDrawSurface"]["hasGfxEntIndex"].get<long long>();
			asset->info.drawSurf.fields.materialSortedIndex = materialJson["gfxDrawSurface"]["materialSortedIndex"].get<long long>();
			asset->info.drawSurf.fields.objectId = materialJson["gfxDrawSurface"]["objectId"].get<long long>();
			asset->info.drawSurf.fields.prepass = materialJson["gfxDrawSurface"]["prepass"].get<long long>();
			asset->info.drawSurf.fields.primarySortKey = materialJson["gfxDrawSurface"]["primarySortKey"].get<long long>();
			asset->info.drawSurf.fields.reflectionProbeIndex = materialJson["gfxDrawSurface"]["reflectionProbeIndex"].get<long long>();
			asset->info.drawSurf.fields.sceneLightIndex = materialJson["gfxDrawSurface"]["sceneLightIndex"].get<long long>();
			asset->info.drawSurf.fields.surfType = materialJson["gfxDrawSurface"]["surfType"].get<long long>();
			asset->info.drawSurf.fields.unused = materialJson["gfxDrawSurface"]["unused"].get<long long>();
			asset->info.drawSurf.fields.useHeroLighting = materialJson["gfxDrawSurface"]["useHeroLighting"].get<long long>();
		}

		asset->stateFlags = static_cast<char>(Utils::Json::ReadFlags(materialJson["stateFlags"].get<std::string>(), sizeof(char)));

		if (materialJson["textureTable"].is_array())
		{
			nlohmann::json::array_t textureTable = materialJson["textureTable"];
			asset->textureCount = static_cast<unsigned char>(textureTable.size());
			asset->textureTable = builder->getAllocator()->allocateArray<Game::MaterialTextureDef>(asset->textureCount);

			for (size_t i = 0; i < textureTable.size(); i++)
			{
				auto& textureJson = textureTable[i];
				if (textureJson.is_object())
				{
					Game::MaterialTextureDef* textureDef = &asset->textureTable[i];
					textureDef->semantic = textureJson["semantic"].get<Game::TextureSemantic>();
					textureDef->samplerState = textureJson["samplerState"].get<char>();
					textureDef->nameStart = textureJson["nameStart"].get<char>();
					textureDef->nameEnd = textureJson["nameEnd"].get<char>();
					textureDef->nameHash = textureJson["nameHash"].get<unsigned int>();

					if (textureDef->semantic == Game::TextureSemantic::TS_WATER_MAP)
					{
						Game::water_t* water = builder->getAllocator()->allocate<Game::water_t>();

						if (textureJson["water"].is_object())
						{
							auto& waterJson = textureJson["water"];

							if (waterJson["image"].is_string())
							{
								auto imageName = waterJson["image"].get<std::string>();

								water->image = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_IMAGE, imageName.data(), builder).image;
							}

							water->amplitude = waterJson["amplitude"].get<float>();
							water->M = waterJson["M"].get<int>();
							water->N = waterJson["N"].get<int>();
							water->Lx = waterJson["Lx"].get<float>();
							water->Lz = waterJson["Lz"].get<float>();
							water->gravity = waterJson["gravity"].get<float>();
							water->windvel = waterJson["windvel"].get<float>();

							auto winddir = waterJson["winddir"].get<std::vector<float>>();
							if (winddir.size() == 2)
							{
								std::copy(winddir.begin(), winddir.end(), water->winddir);
							}

							auto codeConstant = waterJson["codeConstant"].get<std::vector<float>>();

							if (codeConstant.size() == 4)
							{
								std::copy(codeConstant.begin(), codeConstant.end(), water->codeConstant);
							}

							/// H0
							[[maybe_unused]] auto idealSize = water->M * water->N * sizeof(Game::complex_s);
							auto h064 = waterJson["H0"].get<std::string>();
							auto predictedSize = static_cast<unsigned long>(std::ceilf((h064.size() / 4.f) * 3.f));
							assert(predictedSize >= idealSize);

							auto h0 = reinterpret_cast<Game::complex_s*>(builder->getAllocator()->allocate(predictedSize));

							[[maybe_unused]] auto h0Result = base64_decode(
								h064.data(),
								h064.size(),
								reinterpret_cast<unsigned char*>(h0),
								&predictedSize
							);

							assert(h0Result == CRYPT_OK);
							water->H0 = h0;

							/// WTerm
							auto wTerm64 = waterJson["wTerm"].get<std::string>();
							auto predictedWTermSize = static_cast<unsigned long>(std::ceilf((wTerm64.size() / 4.f) * 3.f));

							auto wTerm = reinterpret_cast<float*>(builder->getAllocator()->allocate(predictedWTermSize));

							[[maybe_unused]] auto wTermResult = base64_decode(
								wTerm64.data(),
								wTerm64.size(),
								reinterpret_cast<unsigned char*>(wTerm),
								&predictedWTermSize
							);

							assert(wTermResult == CRYPT_OK);
							water->wTerm = wTerm;
						}

						textureDef->u.water = water;
					}
					else
					{
						textureDef->u.image = nullptr;
						if (textureJson["image"].is_string())
						{
							textureDef->u.image = Components::AssetHandler::FindAssetForZone(
								Game::ASSET_TYPE_IMAGE,
								textureJson["image"].get<std::string>(),
								builder
							).image;

							assert(textureDef->u.image);
						}
						else
						{
							AssertUnreachable;
						}
					}
				}
			}
		}

		// Statebits
		if (materialJson["stateBitsEntry"].is_array())
		{
			nlohmann::json::array_t stateBitsEntry = materialJson["stateBitsEntry"];

			for (size_t i = 0; i < std::min(stateBitsEntry.size(), 48u); i++)
			{
				asset->stateBitsEntry[i] = stateBitsEntry[i].get<char>();
			}
		}

		if (materialJson["stateBitsTable"].is_array())
		{
			nlohmann::json::array_t array = materialJson["stateBitsTable"];
			asset->stateBitsCount = static_cast<unsigned char>(array.size());

			asset->stateBitsTable = builder->getAllocator()->allocateArray<Game::GfxStateBits>(array.size());

			size_t statebitTableIndex = 0;
			for (auto& jsonStateBitEntry : array)
			{
				auto stateBit = &asset->stateBitsTable[statebitTableIndex++];

				unsigned int loadbits0 = 0;
				unsigned int loadbits1 = 0;

#define READ_INT_LB_FROM_JSON(x) unsigned int x = jsonStateBitEntry[#x].get<unsigned int>()
#define READ_BOOL_LB_FROM_JSON(x) bool x = jsonStateBitEntry[#x].get<bool>()

				READ_INT_LB_FROM_JSON(srcBlendRgb);
				READ_INT_LB_FROM_JSON(dstBlendRgb);
				READ_INT_LB_FROM_JSON(blendOpRgb);
				READ_INT_LB_FROM_JSON(srcBlendAlpha);
				READ_INT_LB_FROM_JSON(dstBlendAlpha);
				READ_INT_LB_FROM_JSON(blendOpAlpha);
				READ_INT_LB_FROM_JSON(depthTest);
				READ_INT_LB_FROM_JSON(polygonOffset);

				const auto alphaTest = jsonStateBitEntry["alphaTest"].get<std::string>();
				const auto cullFace = jsonStateBitEntry["cullFace"].get<std::string>();

				READ_BOOL_LB_FROM_JSON(colorWriteRgb);
				READ_BOOL_LB_FROM_JSON(colorWriteAlpha);
				READ_BOOL_LB_FROM_JSON(polymodeLine);

				READ_BOOL_LB_FROM_JSON(gammaWrite);
				READ_BOOL_LB_FROM_JSON(depthWrite);
				READ_BOOL_LB_FROM_JSON(stencilFrontEnabled);
				READ_BOOL_LB_FROM_JSON(stencilBackEnabled);

				READ_INT_LB_FROM_JSON(stencilFrontPass);
				READ_INT_LB_FROM_JSON(stencilFrontFail);
				READ_INT_LB_FROM_JSON(stencilFrontZFail);
				READ_INT_LB_FROM_JSON(stencilFrontFunc);
				READ_INT_LB_FROM_JSON(stencilBackPass);
				READ_INT_LB_FROM_JSON(stencilBackFail);
				READ_INT_LB_FROM_JSON(stencilBackZFail);
				READ_INT_LB_FROM_JSON(stencilBackFunc);

				loadbits0 |= srcBlendRgb << Game::GFXS0_SRCBLEND_RGB_SHIFT;
				loadbits0 |= dstBlendRgb << Game::GFXS0_DSTBLEND_RGB_SHIFT;
				loadbits0 |= blendOpRgb << Game::GFXS0_BLENDOP_RGB_SHIFT;
				loadbits0 |= srcBlendAlpha << Game::GFXS0_SRCBLEND_ALPHA_SHIFT;
				loadbits0 |= dstBlendAlpha << Game::GFXS0_DSTBLEND_ALPHA_SHIFT;
				loadbits0 |= blendOpAlpha << Game::GFXS0_BLENDOP_ALPHA_SHIFT;

				if (depthTest == -1)
				{
					loadbits1 |= Game::GFXS1_DEPTHTEST_DISABLE;
				}
				else
				{
					loadbits1 |= depthTest << Game::GFXS1_DEPTHTEST_SHIFT;
				}

				loadbits1 |= polygonOffset << Game::GFXS1_POLYGON_OFFSET_SHIFT;

				if (alphaTest == "disable")
				{
					loadbits0 |= Game::GFXS0_ATEST_DISABLE;
				}
				else if (alphaTest == ">0")
				{
					loadbits0 |= Game::GFXS0_ATEST_GT_0;
				}
				else if (alphaTest == "<128")
				{
					loadbits0 |= Game::GFXS0_ATEST_LT_128;
				}
				else if (alphaTest == ">=128")
				{
					loadbits0 |= Game::GFXS0_ATEST_GE_128;
				}
				else
				{
					Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid alphatest loadbit0 '{}' in material {}\n", alphaTest, name);
					return;
				}

				if (cullFace == "none")
				{
					loadbits0 |= Game::GFXS0_CULL_NONE;
				}
				else if (cullFace == "back")
				{
					loadbits0 |= Game::GFXS0_CULL_BACK;
				}
				else if (cullFace == "front")
				{
					loadbits0 |= Game::GFXS0_CULL_FRONT;
				}
				else
				{
					Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid cullFace loadbit0 '{}' in material {}\n", cullFace, name);
					return;
				}

				if (gammaWrite)
				{
					loadbits0 |= Game::GFXS0_GAMMAWRITE;
				}

				if (colorWriteAlpha)
				{
					loadbits0 |= Game::GFXS0_COLORWRITE_ALPHA;
				}

				if (colorWriteRgb)
				{
					loadbits0 |= Game::GFXS0_COLORWRITE_RGB;
				}

				if (polymodeLine)
				{
					loadbits0 |= Game::GFXS0_POLYMODE_LINE;
				}

				if (depthWrite)
				{
					loadbits1 |= Game::GFXS1_DEPTHWRITE;
				}
				if (stencilFrontEnabled)
				{
					loadbits1 |= Game::GFXS1_STENCIL_FRONT_ENABLE;
				}
				if (stencilBackEnabled)
				{
					loadbits1 |= Game::GFXS1_STENCIL_BACK_ENABLE;
				}

				loadbits1 |= stencilFrontPass << Game::GFXS1_STENCIL_FRONT_PASS_SHIFT;
				loadbits1 |= stencilFrontFail << Game::GFXS1_STENCIL_FRONT_FAIL_SHIFT;
				loadbits1 |= stencilFrontZFail << Game::GFXS1_STENCIL_FRONT_ZFAIL_SHIFT;
				loadbits1 |= stencilFrontFunc << Game::GFXS1_STENCIL_FRONT_FUNC_SHIFT;
				loadbits1 |= stencilBackPass << Game::GFXS1_STENCIL_BACK_PASS_SHIFT;
				loadbits1 |= stencilBackFail << Game::GFXS1_STENCIL_BACK_FAIL_SHIFT;
				loadbits1 |= stencilBackZFail << Game::GFXS1_STENCIL_BACK_ZFAIL_SHIFT;
				loadbits1 |= stencilBackFunc << Game::GFXS1_STENCIL_BACK_FUNC_SHIFT;

				stateBit->loadBits[0] = loadbits0;
				stateBit->loadBits[1] = loadbits1;
			}

			// Constant table
			if (materialJson["constantTable"].is_array())
			{

				nlohmann::json::array_t constants = materialJson["constantTable"];
				asset->constantCount = static_cast<char>(constants.size());
				auto table = builder->getAllocator()->allocateArray<Game::MaterialConstantDef>(asset->constantCount);

				for (size_t constantIndex = 0; constantIndex < asset->constantCount; constantIndex++)
				{
					auto& constant = constants[constantIndex];
					auto entry = &table[constantIndex];

					auto litVec = constant["literal"].get<std::vector<float>>();
					std::copy(litVec.begin(), litVec.end(), entry->literal);

					auto constantName = constant["name"].get<std::string>();
					std::copy(constantName.begin(), constantName.end(), entry->name);

					entry->nameHash = constant["nameHash"].get<unsigned int>();
				}

				asset->constantTable = table;
			}

			if (materialJson["techniqueSet"].is_string())
			{
				const std::string techsetName = materialJson["techniqueSet"].get<std::string>();
				asset->techniqueSet = findWorkingTechset(techsetName, asset, builder);

				if (asset->techniqueSet == nullptr)
				{
					assert(false);
				}
			}

			header->material = asset;
		}
	}

	Game::MaterialTechniqueSet* IMaterial::findWorkingTechset(std::string techsetName, [[maybe_unused]] Game::Material* material, Components::ZoneBuilder::Zone* builder) const
	{
		Game::MaterialTechniqueSet* techset;

		// Pass 1: Identical techset (1:1)
		techset = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, techsetName.data(), builder).techniqueSet;
		if (techset != nullptr)
		{
			return techset;
		}

		// We do no more cause we use CoD4 techset and they should always be present
		// If one day we want to go back to mw2 fallback we can add extra steps here!

		return nullptr;
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

		Components::FileSystem::File materialFile(std::format("materials/{}.iw4xMaterial", name));
		if (!materialFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), materialFile.getBuffer());

		char* magic = reader.readArray<char>(7);
		if (std::memcmp(magic, "IW4xMat", 7) != 0)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading material '{}' failed, header is invalid!", name);
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_MAT_BIN_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading material '{}' failed, expected version is {}, but it was {}!", name, IW4X_MAT_BIN_VERSION, version);
		}

		auto* asset = reader.readObject<Game::Material>();

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

				if (textureDef->semantic == Game::TextureSemantic::TS_WATER_MAP)
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
						std::memcpy(asset->stateBitsEntry, header.material->stateBitsEntry, ARRAYSIZE(asset->stateBitsEntry));
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
			if (const auto itr = techSetCorrespondance.find(techName); itr != techSetCorrespondance.end())
			{
				auto& iw4TechSetName = itr->second;
				auto* iw4TechSet = Game::DB_FindXAssetEntry(Game::ASSET_TYPE_TECHNIQUE_SET, iw4TechSetName.data());

				if (iw4TechSet)
				{
					Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [asset, iw4TechSet](Game::XAssetEntry* entry)
						{
							if (!replacementFound)
							{
								Game::XAssetHeader header = entry->asset.header;
								Components::Logger::Print("Material {} with techset {} has been mapped to {} (last chance!), taking the sort key of material {}\n",
									asset->info.name, asset->techniqueSet->name, header.material->techniqueSet->name, header.material->info.name);

								// Yeah this has a tendency to fuck up a LOT of transparent materials
								if (header.material->techniqueSet == iw4TechSet->asset.header.techniqueSet && std::string(header.material->info.name).find("icon") != std::string::npos)
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
					if (asset->textureTable[i].semantic == Game::TextureSemantic::TS_WATER_MAP)
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

				auto* destTextureTable = buffer->dest<Game::MaterialTextureDef>();
				buffer->saveArray(asset->textureTable, asset->textureCount);

				for (std::uint8_t i = 0; i < asset->textureCount; ++i)
				{
					auto* destTextureDef = &destTextureTable[i];
					auto* textureDef = &asset->textureTable[i];

					if (textureDef->semantic == Game::TextureSemantic::TS_WATER_MAP)
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
