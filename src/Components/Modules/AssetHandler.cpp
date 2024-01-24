#include <STDInclude.hpp>
#include "FastFiles.hpp"
#include "Weapon.hpp"

#include "AssetInterfaces/IFont_s.hpp"
#include "AssetInterfaces/IWeapon.hpp"
#include "AssetInterfaces/IXModel.hpp"
#include "AssetInterfaces/IFxWorld.hpp"
#include "AssetInterfaces/IMapEnts.hpp"
#include "AssetInterfaces/IRawFile.hpp"
#include "AssetInterfaces/IComWorld.hpp"
#include "AssetInterfaces/IGfxImage.hpp"
#include "AssetInterfaces/IGfxWorld.hpp"
#include "AssetInterfaces/IMaterial.hpp"
#include "AssetInterfaces/ISndCurve.hpp"
#include "AssetInterfaces/IMenuList.hpp"
#include "AssetInterfaces/IclipMap_t.hpp"
#include "AssetInterfaces/ImenuDef_t.hpp"
#include "AssetInterfaces/ITracerDef.hpp"
#include "AssetInterfaces/IPhysPreset.hpp"
#include "AssetInterfaces/IXAnimParts.hpp"
#include "AssetInterfaces/IFxEffectDef.hpp"
#include "AssetInterfaces/IGameWorldMp.hpp"
#include "AssetInterfaces/IGameWorldSp.hpp"
#include "AssetInterfaces/IGfxLightDef.hpp"
#include "AssetInterfaces/ILoadedSound.hpp"
#include "AssetInterfaces/IPhysCollmap.hpp"
#include "AssetInterfaces/IStringTable.hpp"
#include "AssetInterfaces/IXModelSurfs.hpp"
#include "AssetInterfaces/ILocalizeEntry.hpp"
#include "AssetInterfaces/Isnd_alias_list_t.hpp"
#include "AssetInterfaces/IMaterialPixelShader.hpp"
#include "AssetInterfaces/IMaterialTechniqueSet.hpp"
#include "AssetInterfaces/IMaterialVertexShader.hpp"
#include "AssetInterfaces/IStructuredDataDefSet.hpp"
#include "AssetInterfaces/IMaterialVertexDeclaration.hpp"

namespace Components
{
	thread_local int AssetHandler::BypassState = 0;
	bool AssetHandler::ShouldSearchTempAssets = false;
	std::map<Game::XAssetType, AssetHandler::IAsset*> AssetHandler::AssetInterfaces;
	std::map<Game::XAssetType, Utils::Slot<AssetHandler::Callback>> AssetHandler::TypeCallbacks;
	Utils::Signal<AssetHandler::RestrictCallback> AssetHandler::RestrictSignal;

	std::map<void*, void*> AssetHandler::Relocations;

	std::vector<std::pair<Game::XAssetType, std::string>> AssetHandler::EmptyAssets;

	std::map<std::string, Game::XAssetHeader> AssetHandler::TemporaryAssets[Game::ASSET_TYPE_COUNT];

	void AssetHandler::RegisterInterface(IAsset* iAsset)
	{
		if (!iAsset) return;
		if (iAsset->getType() == Game::XAssetType::ASSET_TYPE_INVALID)
		{
			delete iAsset;
			return;
		}

		if (AssetHandler::AssetInterfaces.contains(iAsset->getType()))
		{
			Logger::Print("Duplicate asset interface: {}\n", Game::DB_GetXAssetTypeName(iAsset->getType()));
			delete AssetHandler::AssetInterfaces[iAsset->getType()];
		}
		else
		{
			Logger::Print("Asset interface registered: {}\n", Game::DB_GetXAssetTypeName(iAsset->getType()));
		}

		AssetHandler::AssetInterfaces[iAsset->getType()] = iAsset;
	}

	void AssetHandler::ClearTemporaryAssets()
	{
		for (int i = 0; i < Game::ASSET_TYPE_COUNT; ++i)
		{
			AssetHandler::TemporaryAssets[i].clear();
		}
	}

	void AssetHandler::StoreTemporaryAsset(Game::XAssetType type, Game::XAssetHeader asset)
	{
		AssetHandler::TemporaryAssets[type][Game::DB_GetXAssetNameHandlers[type](&asset)] = asset;
	}

	Game::XAssetHeader AssetHandler::FindAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { nullptr };

		if (filename)
		{
			// Allow call DB_FindXAssetHeader within the hook
			AssetHandler::SetBypassState(true);

			if (AssetHandler::TypeCallbacks.contains(type))
			{
				header = AssetHandler::TypeCallbacks[type](type, filename);
			}

			// Disallow calling DB_FindXAssetHeader ;)
			AssetHandler::SetBypassState(false);
		}

		return header;
	}

	Game::XAssetHeader AssetHandler::FindTemporaryAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { nullptr };
		if (type >= Game::ASSET_TYPE_COUNT) return header;

		auto tempPool = &AssetHandler::TemporaryAssets[type];
		auto entry = tempPool->find(filename);
		if (entry != tempPool->end())
		{
			header = { entry->second };
		}

		return header;
	}    

	int AssetHandler::HasThreadBypass()
	{
		return AssetHandler::BypassState > 0;
	}

	void AssetHandler::SetBypassState(bool value)
	{
		AssetHandler::BypassState += (value ? 1 : -1);
		if (AssetHandler::BypassState < 0)
		{
			AssetHandler::BypassState = 0;
			Logger::Error(Game::ERR_FATAL, "Bypass state is below 0!");
		}
	}

	void AssetHandler::ResetBypassState()
	{
		if (AssetHandler::HasThreadBypass())
		{
			// Maybe just decrement it?
			AssetHandler::BypassState = 0;
		}
	}

	__declspec(naked) void AssetHandler::FindAssetStub()
	{
		__asm
		{
			push ecx
			push ebx
			push ebp
			push esi
			push edi

			push eax
			pushad

			// Check if custom handler should be bypassed
			call AssetHandler::HasThreadBypass

			mov [esp + 20h], eax
			popad
			pop eax

			test al, al
			jnz checkTempAssets

			mov ecx, [esp + 18h] // Asset type
			mov ebx, [esp + 1Ch] // Filename

			push eax
			pushad

			push ebx
			push ecx

			call AssetHandler::FindAsset

			add esp, 8h

			mov [esp + 20h], eax
			popad
			pop eax

			test eax, eax
			jnz finishFound

		checkTempAssets:
			mov al, AssetHandler::ShouldSearchTempAssets // check to see if enabled
			test eax, eax
			jz finishOriginal

			mov ecx, [esp + 18h] // Asset type
			mov ebx, [esp + 1Ch] // Filename

			push ebx
			push ecx

			call AssetHandler::FindTemporaryAsset

			add esp, 8h

			test eax, eax
			jnz finishFound

		finishOriginal:
			// Asset not found using custom handlers or in temp assets or bypasses were enabled
			// redirect to DB_FindXAssetHeader
			mov ebx, ds:6D7190h // InterlockedDecrement
			mov eax, 40793Bh
			jmp eax

		finishFound:
			pop edi
			pop esi
			pop ebp
			pop ebx
			pop ecx
			retn
		}
	}
#pragma optimize( "", off )
	void AssetHandler::ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, const std::string& name)
	{
#ifdef DEBUG
		if (type == Game::XAssetType::ASSET_TYPE_IMAGE && name[0] != ',')
		{
			const auto image = asset.image;
			const auto cat = static_cast<Game::ImageCategory>(image->category);
			if (cat == Game::ImageCategory::IMG_CATEGORY_UNKNOWN)
			{
				Logger::Warning(Game::CON_CHANNEL_GFX, "Image {} has wrong category IMG_CATEGORY_UNKNOWN, this is an IMPORTANT ISSUE that should be fixed!\n", name);
			}
		}
#endif

		if (name == "body_urban_civ_female_a"s)
			//if (name == "mp_body_tf141_lmg"s)
		{
			class QuatInt16
			{
			public:
				static uint16_t ToInt16(const float quat)
				{
					return static_cast<uint16_t>(quat * INT16_MAX);
				}

				static float ToFloat(const uint16_t quat)
				{
					return static_cast<float>(quat) / static_cast<float>(INT16_MAX);
				}
			};

			struct BoneEnsemble
			{
				uint16_t name;
				Game::DObjAnimMat mat;
				Game::XBoneInfo info;
				int16_t quat[4];
				float trans[3];

				BoneEnsemble() {};

				BoneEnsemble(Game::XModel* model, int index)
				{
					name = model->boneNames[index];
					mat = model->baseMat[index];
					info = model->boneInfo[index];

					std::memcpy(quat, &model->quats[(index - 1) * 4], 4 * sizeof(uint16_t));
					std::memcpy(trans, &model->trans[(index - 1) * 3], 3 * sizeof(float));
				}
			};

			const auto equals = [](const BoneEnsemble& a, const BoneEnsemble& b)
				{
					if (a.name == b.name)
					{
						if (b.mat.transWeight != a.mat.transWeight)
						{
							return false;
						}

						for (size_t i = 0; i < 4; i++)
						{
							if (b.mat.quat[i] != a.mat.quat[i])
							{
								return false;
							}

							if (b.quat[i] != a.quat[i])
							{
								return false;
							}
						}

						for (size_t i = 0; i < 3; i++)
						{
							if (b.mat.trans[i] != a.mat.trans[i])
							{
								return false;
							}

							if (b.trans[i] != a.trans[i])
							{
								return false;
							}

							if (b.info.bounds.halfSize[i] != a.info.bounds.halfSize[i])
							{
								return false;
							}

							if (b.info.bounds.midPoint[i] != a.info.bounds.midPoint[i])
							{
								return false;
							}
						}

						if (b.info.radiusSquared != a.info.radiusSquared)
						{
							return false;
						}

						return true;
					}

					return false;
				};

			const auto model = asset.model;

			static std::vector<std::string> names{};

			for (auto i = 0; i < model->numBones; i++)
			{
				const auto bone = model->boneNames[i];
				const auto name = Game::SL_ConvertToString(bone);
				names.push_back(name);
			}

			const auto getIndexOfBone = [&](std::string name)
				{
					for (uint8_t i = 0; i < model->numBones; i++)
					{
						const auto bone = model->boneNames[i];
						const auto boneName = Game::SL_ConvertToString(bone);
						if (name == boneName)
						{
							return i;
						}
					}

					return static_cast<uint8_t>(UCHAR_MAX);
				};

			const auto getParentIndexOfBone = [&](uint8_t index)
				{
					const auto parentIndex = index - model->parentList[index - model->numRootBones];
					return parentIndex;
				};

			const auto setParentIndexOfBone = [&](uint8_t boneIndex, uint8_t parentIndex)
				{
					if (boneIndex == SCHAR_MAX)
					{
						return;
					}

					model->parentList[boneIndex - model->numRootBones] = boneIndex - parentIndex;
				};

			const auto getParentOfBone = [&](int index)
				{
					const auto parentIndex = getParentIndexOfBone(index);
					const auto boneName = Game::SL_ConvertToString(model->boneNames[parentIndex]);
					return boneName;
				};

			const auto insertBone = [&](const std::string& boneName, uint8_t parentIndex)
				{
					uint8_t newBoneCount = model->numBones + 1;
					uint8_t newBoneCountMinusRoot = newBoneCount - model->numRootBones;

					int atPosition = model->numBones;

					// Find best position to insert it
					for (int index = model->numRootBones; index < model->numBones; index++)
					{
						int parent = getParentIndexOfBone(index);
						if (parent >= parentIndex)
						{
							atPosition = index;
							break;
						}
					}

					const auto newBoneIndex = atPosition;
					const auto newBoneIndexMinusRoot = atPosition - model->numRootBones;

					// Reallocate
					const auto newBoneNames = (uint16_t*)Game::Z_Malloc(sizeof(uint16_t) * newBoneCount);
					const auto newMats = (Game::DObjAnimMat*)Game::Z_Malloc(sizeof(Game::DObjAnimMat) * newBoneCount);
					const auto newBoneInfo = (Game::XBoneInfo*)Game::Z_Malloc(sizeof(Game::XBoneInfo) * newBoneCount);
					const auto newQuats = (int16_t*)Game::Z_Malloc(sizeof(uint16_t) * 4 * newBoneCountMinusRoot);
					const auto newTrans = (float*)Game::Z_Malloc(sizeof(float) * 3 * newBoneCountMinusRoot);
					const auto newParentList = reinterpret_cast<unsigned char*>(Game::Z_Malloc(sizeof(uint8_t) * newBoneCountMinusRoot));

					int lengthOfFirstPart = atPosition;
					int lengthOfSecondPart = model->numBones - atPosition;

					int lengthOfFirstPartM1 = atPosition - model->numRootBones;
					int lengthOfSecondPartM1 = model->numBones - model->numRootBones - (atPosition - model->numRootBones);

					int atPositionM1 = atPosition - model->numRootBones;

					// should be equal to model->numBones
					int total = lengthOfFirstPart + lengthOfSecondPart;
					assert(total = model->numBones);

					// should be equal to model->numBones - model->numRootBones
					int totalM1 = lengthOfFirstPartM1 + lengthOfSecondPartM1;
					assert(totalM1 == model->numBones - model->numRootBones);

					// Copy before
					if (lengthOfFirstPart > 0)
					{
						std::memcpy(newBoneNames, model->boneNames, sizeof(uint16_t) * lengthOfFirstPart);
						std::memcpy(newMats, model->baseMat, sizeof(Game::DObjAnimMat) * lengthOfFirstPart);
						std::memcpy(newBoneInfo, model->boneInfo, sizeof(Game::XBoneInfo) * lengthOfFirstPart);
						std::memcpy(newQuats, model->quats, sizeof(uint16_t) * 4 * lengthOfFirstPartM1);
						std::memcpy(newTrans, model->trans, sizeof(float) * 3 * lengthOfFirstPartM1);
					}

					// Insert new bone
					{
						unsigned int name = Game::SL_GetString(boneName.data(), 0);
						Game::XBoneInfo boneInfo{};

						Game::DObjAnimMat mat{};

						mat = model->baseMat[parentIndex];
						boneInfo = model->boneInfo[parentIndex];

						uint16_t quat[4]{};
						std::memcpy(quat, &model->quats[(parentIndex - model->numRootBones) * sizeof(uint16_t)], ARRAYSIZE(quat) * sizeof(uint16_t));

						float trans[3]{};
						std::memcpy(trans, &model->trans[(parentIndex - model->numRootBones) * sizeof(float)], ARRAYSIZE(trans) * sizeof(float));

						//mat.quat[3] = 1.0f;

						//mat.trans[0] = -3.4;
						//mat.trans[1] = -6;
						//mat.trans[2] = 37;

						//mat.transWeight = 2.0f;

						uint8_t part = 5; // Unused ?

						newMats[newBoneIndex] = mat;
						newBoneInfo[newBoneIndex] = boneInfo;
						newBoneNames[newBoneIndex] = name;

						std::memcpy(&newQuats[newBoneIndexMinusRoot * 4], quat, ARRAYSIZE(quat) * sizeof(uint16_t));
						std::memcpy(&newTrans[newBoneIndexMinusRoot * 3], trans, ARRAYSIZE(trans) * sizeof(float));
					}

					// Copy after
					if (lengthOfSecondPart > 0)
					{
						std::memcpy(&newBoneNames[atPosition + 1], &model->boneNames[atPosition], sizeof(uint16_t) * lengthOfSecondPart);
						std::memcpy(&newMats[atPosition + 1], &model->baseMat[atPosition], sizeof(Game::DObjAnimMat) * lengthOfSecondPart);
						std::memcpy(&newBoneInfo[atPosition + 1], &model->boneInfo[atPosition], sizeof(Game::XBoneInfo) * lengthOfSecondPart);
						std::memcpy(&newQuats[(atPositionM1 + 1) * 4], &model->quats[atPositionM1 * 4], sizeof(uint16_t) * 4 * lengthOfSecondPartM1);
						std::memcpy(&newTrans[(atPositionM1 + 1) * 3], &model->trans[atPositionM1 * 3], sizeof(float) * 3 * lengthOfSecondPartM1);
					}

					// Assign reallocated
					model->baseMat = newMats;
					model->boneInfo = newBoneInfo;
					model->boneNames = newBoneNames;
					model->quats = newQuats;
					model->trans = newTrans;
					model->parentList = newParentList;

					model->numBones++;

					// Update vertex weights
					for (int i = 0; i < model->numLods; i++)
					{
						const auto lod = &model->lodInfo[i];
						size_t weightOffset = 0u;

						for (int surfIndex = 0; surfIndex < lod->modelSurfs->numsurfs; surfIndex++)
						{
							auto vertsBlendOffset = 0u;

							const auto surface = &lod->modelSurfs->surfs[surfIndex];

							{
								const auto fixVertexBlendIndex = [&](unsigned int offset) {
									int index = static_cast<int>(surface->vertInfo.vertsBlend[offset] / sizeof(Game::DObjSkelMat));
									if (index >= atPosition)
									{
										if (index < 0 || index >= model->numBones - 1)
										{
											assert(false);
										}

										index++;

										surface->vertInfo.vertsBlend[offset] = index * sizeof(Game::DObjSkelMat);
									}
								};

								// Fix bone offsets
								if (surface->vertList)
								{
									for (auto vertListIndex = 0u; vertListIndex < surface->vertListCount; vertListIndex++)
									{
										const auto vertList = &surface->vertList[vertListIndex];
										auto index = vertList->boneOffset / sizeof(Game::DObjSkelMat);
										if (index < 0 || index >= model->numBones - 1)
										{
											assert(false);
										}

										if (index >= atPosition)
										{
											index++;
											vertList->boneOffset = index * sizeof(Game::DObjSkelMat);
										}
									}
								}

								// 1 bone weight
								for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[0]; vertIndex++)
								{
									fixVertexBlendIndex(vertsBlendOffset + 0);

									vertsBlendOffset += 1;
								}

								// 2 bone weights
								for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[1]; vertIndex++)
								{
									fixVertexBlendIndex(vertsBlendOffset + 0);
									fixVertexBlendIndex(vertsBlendOffset + 1);

									vertsBlendOffset += 3;
								}

								// 3 bone weights
								for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[2]; vertIndex++)
								{
									fixVertexBlendIndex(vertsBlendOffset + 0);
									fixVertexBlendIndex(vertsBlendOffset + 1);
									fixVertexBlendIndex(vertsBlendOffset + 3);


									vertsBlendOffset += 5;
								}

								// 4 bone weights
								for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[3]; vertIndex++)
								{
									fixVertexBlendIndex(vertsBlendOffset + 0);
									fixVertexBlendIndex(vertsBlendOffset + 1);
									fixVertexBlendIndex(vertsBlendOffset + 3);
									fixVertexBlendIndex(vertsBlendOffset + 5);

									vertsBlendOffset += 7;
								}
							}
						}
					}

					// TODO free memory of original lists
					printf("");

					return atPosition; // Bone index of added bone
				};

			auto indexOfSpine = getIndexOfBone("j_spinelower");
			if (indexOfSpine < UCHAR_MAX)
			{
				const auto nameOfParent = getParentOfBone(indexOfSpine);
				const auto stabilizer = getIndexOfBone("torso_stabilizer");
				const auto otherIndex = stabilizer - model->numRootBones;

				{
					const auto root = getIndexOfBone("j_mainroot");
					if (root < UCHAR_MAX) {

						//
						std::map<std::string, std::string> parentsBefore{};
						std::map<std::string, BoneEnsemble> bonesBefore{};
						for (int i = model->numRootBones; i < model->numBones; i++)
						{
							parentsBefore[Game::SL_ConvertToString(model->boneNames[i])] = getParentOfBone(i);
							bonesBefore[Game::SL_ConvertToString(model->boneNames[i])] = BoneEnsemble(model, i);
						}
						//

#if 0
						// Add pelvis
						auto indexOfPelvis = getIndexOfBone("pelvis");
						if (indexOfPelvis == UCHAR_MAX)
						{
							indexOfPelvis = insertBone("pelvis", root);

							parentsBefore["pelvis"] = "j_mainroot";
							setParentIndexOfBone(indexOfPelvis, getIndexOfBone("j_mainroot"));

							assert(root == getIndexOfBone("j_mainroot"));

							parentsBefore["j_hip_le"] = "pelvis";
							setParentIndexOfBone(getIndexOfBone("j_hip_le"), getIndexOfBone("pelvis"));

							parentsBefore["j_hip_ri"] = "pelvis";
							setParentIndexOfBone(getIndexOfBone("j_hip_ri"), getIndexOfBone("pelvis"));

							parentsBefore["tag_stowed_hip_rear"] = "pelvis";
							setParentIndexOfBone(getIndexOfBone("tag_stowed_hip_rear"), getIndexOfBone("pelvis"));

						}

						uint8_t torsoStabilizer = insertBone("torso_stabilizer", indexOfPelvis);
						assert(indexOfPelvis == getIndexOfBone("pelvis"));

#else
						const auto parentIndex = 30;
						uint8_t torsoStabilizer = insertBone("torso_stabilizer", parentIndex);
						//setParentIndexOfBone(torsoStabilizer, parentIndex);
#endif

#if DEBUG
						const auto newRoot = getIndexOfBone("j_mainroot");
						assert(root == newRoot);
#endif
						//parentsBefore["j_spinelower"] = "torso_stabilizer";
						//setParentIndexOfBone(getIndexOfBone("j_spinelower"), torsoStabilizer);

						//parentsBefore["torso_stabilizer"] = "pelvis";
						//setParentIndexOfBone(torsoStabilizer, indexOfPelvis);
						
						//
						std::map<std::string, std::string> parentsAfter{};
						std::map<std::string, BoneEnsemble> bonesAfter{};
						for (int i = model->numRootBones; i < model->numBones; i++)
						{
							parentsAfter[Game::SL_ConvertToString(model->boneNames[i])] = getParentOfBone(i);
							bonesAfter[Game::SL_ConvertToString(model->boneNames[i])] = BoneEnsemble(model, i);
						}
						//

						for (const auto& kv : parentsBefore)
						{
							// Fix parents
							const auto key = kv.first;
							const auto beforeVal = kv.second;
							const auto afterVal = parentsAfter[kv.first];
							if (beforeVal != afterVal)
							{
								const auto parentIndex = getIndexOfBone(beforeVal);
								const auto index = getIndexOfBone(key);
								setParentIndexOfBone(index, parentIndex);
								parentsAfter[Game::SL_ConvertToString(model->boneNames[index])] = getParentOfBone(index);
							}
						}


						// check
						for (const auto& kv : parentsBefore)
						{
							const auto key = kv.first;
							const auto beforeVal = kv.second;
							
							const auto afterVal = parentsAfter[key];

							if (beforeVal != afterVal)
							{
								printf("");
							}
						}
						//

						//
						for (const auto& kv : bonesBefore)
						{
							const auto key = kv.first;
							const auto beforeVal = kv.second;
							const auto afterVal = bonesAfter[kv.first];
							if (equals(beforeVal, afterVal))
							{
								// good
							}
							else
							{
								printf("");
							}
						}
						//
 						printf("");
					}
				}
				printf("");
			}

			printf("");
		}

		if (type == Game::ASSET_TYPE_MATERIAL && (name == "gfx_distortion_knife_trail" || name == "gfx_distortion_heat_far" || name == "gfx_distortion_ring_light" || name == "gfx_distortion_heat") && asset.material->info.sortKey >= 43)
		{
			if (Zones::Version() >= VERSION_ALPHA2)
			{
				asset.material->info.sortKey = 44;
			}
			else
			{
				asset.material->info.sortKey = 43;
			}
		}

		if (type == Game::ASSET_TYPE_MATERIAL && (name == "wc/codo_ui_viewer_black_decal3" || name == "wc/codo_ui_viewer_black_decal2" || name == "wc/hint_arrows01" || name == "wc/hint_arrows02"))
		{
			asset.material->info.sortKey = 0xE;
		}

		if (type == Game::ASSET_TYPE_VEHICLE && Zones::Version() >= VERSION_ALPHA2)
		{
			asset.vehDef->turretWeapon = nullptr;
		}

		// Fix shader const stuff
		if (type == Game::ASSET_TYPE_TECHNIQUE_SET && Zones::Version() >= 359 && Zones::Version() < 448)
		{
			for (int i = 0; i < 48; ++i)
			{
				if (asset.techniqueSet->techniques[i])
				{
					for (int j = 0; j < asset.techniqueSet->techniques[i]->passCount; ++j)
					{
						Game::MaterialPass* pass = &asset.techniqueSet->techniques[i]->passArray[j];

						for (int k = 0; k < (pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount); ++k)
						{
							if (pass->args[k].type == Game::MaterialShaderArgumentType::MTL_ARG_LITERAL_PIXEL_CONST)
							{
								if (pass->args[k].u.codeConst.index == -28132)
								{
									pass->args[k].u.codeConst.index = 2644;
								}
							}
						}
					}
				}
			}
		}

		if (type == Game::ASSET_TYPE_GFXWORLD && Zones::Version() >= 316)
		{
			asset.gfxWorld->sortKeyEffectDecal = 39;
			asset.gfxWorld->sortKeyEffectAuto = 48;
			asset.gfxWorld->sortKeyDistortion = 43;
		}
	}
#pragma optimize( "", on )

	bool AssetHandler::IsAssetEligible(Game::XAssetType type, Game::XAssetHeader *asset)
	{
		const char* name = Game::DB_GetXAssetNameHandlers[type](asset);
		if (!name) return false;

		for (auto i = AssetHandler::EmptyAssets.begin(); i != AssetHandler::EmptyAssets.end();)
		{
			if (i->first == type && i->second == name)
			{
				i = AssetHandler::EmptyAssets.erase(i);
			}
			else
			{
				++i;
			}
		}

		if (Flags::HasFlag("entries"))
		{
			OutputDebugStringA(Utils::String::VA("%s: %d: %s\n", FastFiles::Current().data(), type, name));
		}

		bool restrict = false;
		AssetHandler::RestrictSignal(type, *asset, name, &restrict);

		if (!restrict)
		{
			AssetHandler::ModifyAsset(type, *asset, name);
		}

		// If no slot restricts the loading, we can load the asset
		return (!restrict);
	}

	__declspec(naked) void AssetHandler::AddAssetStub()
	{
		__asm
		{
			push eax
			pushad

			push [esp + 2Ch]
			push [esp + 2Ch]
			call AssetHandler::IsAssetEligible
			add esp, 8h

			mov [esp + 20h], eax
			popad
			pop eax

			test al, al
			jz doNotLoad

			mov eax, [esp + 8h]
			sub esp, 14h
			mov ecx, 5BB657h
			jmp ecx

		doNotLoad:
			mov eax, [esp + 8h]
			retn
		}
	}

	void AssetHandler::OnFind(Game::XAssetType type, Utils::Slot<AssetHandler::Callback> callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	std::function<void()> AssetHandler::OnLoad(Utils::Slot<AssetHandler::RestrictCallback> callback)
	{
		AssetHandler::RestrictSignal.connect(callback);

		return [callback](){
			AssetHandler::RestrictSignal.disconnect(callback);
		};
	}

	void AssetHandler::ClearRelocations()
	{
		AssetHandler::Relocations.clear();
	}

	void AssetHandler::Relocate(void* start, void* to, DWORD size)
	{
		for (DWORD i = 0; i < size; i += 4)
		{
			AssetHandler::Relocations[reinterpret_cast<char*>(start) + i] = reinterpret_cast<char*>(to) + i;
		}
	}

	void AssetHandler::OffsetToAlias(Utils::Stream::Offset* offset)
	{
		void* pointer = (*Game::g_streamBlocks)[offset->getUnpackedBlock()].data + offset->getUnpackedOffset();

		if (AssetHandler::Relocations.contains(pointer))
		{
			pointer = AssetHandler::Relocations[pointer];
		}

		offset->pointer = *static_cast<void**>(pointer);
	}

	void AssetHandler::ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.contains(asset.type))
		{
			AssetHandler::AssetInterfaces[asset.type]->save(asset.header, builder);
		}
		else
		{
			Logger::Error(Game::ERR_FATAL, "No interface for type '{}'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	void AssetHandler::ZoneMark(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.contains(asset.type))
		{
			AssetHandler::AssetInterfaces[asset.type]->mark(asset.header, builder);
		}
		else
		{
			Logger::Error(Game::ERR_FATAL, "No interface for type '{}'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	Game::XAssetHeader AssetHandler::FindAssetForZone(Game::XAssetType type, const std::string& filename, ZoneBuilder::Zone* builder, bool isSubAsset)
	{
		ZoneBuilder::Zone::AssetRecursionMarker _(builder);

		Game::XAssetHeader header = { nullptr };
		if (type >= Game::ASSET_TYPE_COUNT) return header;

		auto tempPool = &AssetHandler::TemporaryAssets[type];
		auto entry = tempPool->find(filename);
		if (entry != tempPool->end())
		{
			return { entry->second };
		}

		if (AssetHandler::AssetInterfaces.contains(type))
		{
			AssetHandler::AssetInterfaces[type]->load(&header, filename, builder);

			if (header.data)
			{
				Components::AssetHandler::StoreTemporaryAsset(type, header);
			}
		}

		if (!header.data && isSubAsset)
		{
			header = ZoneBuilder::GetEmptyAssetIfCommon(type, filename, builder);
		}

		if (!header.data)
		{
			header = Game::DB_FindXAssetHeader(type, filename.data());
			if (header.data) Components::AssetHandler::StoreTemporaryAsset(type, header); // Might increase efficiency...
		}

		return header;
	}

	Game::XAssetHeader AssetHandler::FindOriginalAsset(Game::XAssetType type, const char* filename)
	{
		AssetHandler::SetBypassState(true);
		Game::XAssetHeader header = Game::DB_FindXAssetHeader(type, filename);
		AssetHandler::SetBypassState(false);

		return header;
	}

	void AssetHandler::StoreEmptyAsset(Game::XAssetType type, const char* name)
	{
		AssetHandler::EmptyAssets.push_back({ type, name });
	}

	__declspec(naked) void AssetHandler::StoreEmptyAssetStub()
	{
		__asm
		{
			pushad
			push ebx
			push eax

			call AssetHandler::StoreEmptyAsset

			pop eax
			pop ebx
			popad

			push 5BB290h
			retn
		}
	}

	void AssetHandler::MissingAssetError(int severity, const char* format, const char* type, const char* name)
	{
		if (Dedicated::IsEnabled() && Game::DB_GetXAssetNameType(type) == Game::ASSET_TYPE_TECHNIQUE_SET) return;
		Utils::Hook::Call<void(int, const char*, const char*, const char*)>(0x4F8C70)(severity, format, type, name); // Print error
	}

	void AssetHandler::reallocateEntryPool()
	{
		AssertSize(Game::XAssetEntry, 16);

		size_t size = (ZoneBuilder::IsEnabled() ? 1183968 : 789312);
		Game::XAssetEntry* entryPool = Utils::Memory::GetAllocator()->allocateArray<Game::XAssetEntry>(size);

		// Apply new size
		Utils::Hook::Set<DWORD>(0x5BAEB0, size);

		// Apply new pool
		DWORD patches[] =
		{
			0x48E6F4,
			0x4C67E4,
			0x4C8584,
			0x5BAEA8,
			0x5BB0C4,
			0x5BB0F5,
			0x5BB1D4,
			0x5BB235,
			0x5BB278,
			0x5BB34C,
			0x5BB484,
			0x5BB570,
			0x5BB6B7,
			0x5BB844,
			0x5BB98D,
			0x5BBA66,
			0x5BBB8D,
			0x5BBCB1,
			0x5BBD9B,
			0x5BBE4C,
			0x5BBF14,
			0x5BBF54,
			0x5BBFB8
		};

		for (int i = 0; i < ARRAYSIZE(patches); ++i)
		{
			Utils::Hook::Set<Game::XAssetEntry*>(patches[i], entryPool);
		}

		Utils::Hook::Set<Game::XAssetEntry*>(0x5BAE91, entryPool + 1);
		Utils::Hook::Set<Game::XAssetEntry*>(0x5BAEA2, entryPool + 1);
	}

	void AssetHandler::ExposeTemporaryAssets(bool expose)
	{
		AssetHandler::ShouldSearchTempAssets = expose;
	}

	AssetHandler::AssetHandler()
	{
		this->reallocateEntryPool();

		Dvar::Register<bool>("r_noVoid", false, Game::DVAR_ARCHIVE, "Disable void model (red fx)");

		AssetHandler::ClearTemporaryAssets();

		// DB_FindXAssetHeader
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).install()->quick();

		// DB_ConvertOffsetToAlias
		Utils::Hook(0x4FDFA0, AssetHandler::OffsetToAlias, HOOK_JUMP).install()->quick();

		// DB_AddXAsset
		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).install()->quick();

		// Store empty assets
		Utils::Hook(0x5BB6EC, AssetHandler::StoreEmptyAssetStub, HOOK_CALL).install()->quick();

		// Intercept missing asset messages
		if (!ZoneBuilder::IsEnabled()) Utils::Hook(0x5BB3F2, AssetHandler::MissingAssetError, HOOK_CALL).install()->quick();

		// Log missing empty assets
		Scheduler::Loop([]
		{
			if (FastFiles::Ready() && !AssetHandler::EmptyAssets.empty())
			{
				for (auto& asset : AssetHandler::EmptyAssets)
				{
					Logger::Warning(Game::CON_CHANNEL_FILES, "Could not load {} \"{}\".\n", Game::DB_GetXAssetTypeName(asset.first), asset.second);
				}

				AssetHandler::EmptyAssets.clear();
			}
		}, Scheduler::Pipeline::MAIN);

		AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool*)
		{
			if (Dvar::Var("r_noVoid").get<bool>() && type == Game::ASSET_TYPE_XMODEL && name == "void")
			{
				asset.model->numLods = 0;
			}
		});

		Game::ReallocateAssetPool(Game::ASSET_TYPE_GAMEWORLD_SP, 1);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_IMAGE, ZoneBuilder::IsEnabled() ? 14336 * 2 : 7168);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_LOADED_SOUND, 2700 * 2);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_FX, 1200 * 2);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_LOCALIZE_ENTRY, 14000);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_XANIMPARTS, 8192 * 2);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_XMODEL, 5125 * 2);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_PHYSPRESET, 128);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_PIXELSHADER, ZoneBuilder::IsEnabled() ? 0x4000 : 10000);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_VERTEXSHADER, ZoneBuilder::IsEnabled() ? 0x2000 : 3072);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_MATERIAL, 8192 * 2);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_VERTEXDECL, ZoneBuilder::IsEnabled() ? 0x400 : 196);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_WEAPON, WEAPON_LIMIT);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_STRINGTABLE, 800);
		Game::ReallocateAssetPool(Game::ASSET_TYPE_IMPACT_FX, 8);

		// Register asset interfaces
		if (ZoneBuilder::IsEnabled())
		{
			Game::ReallocateAssetPool(Game::ASSET_TYPE_MAP_ENTS, 10);
			Game::ReallocateAssetPool(Game::ASSET_TYPE_XMODEL_SURFS, 8192 * 2);
			Game::ReallocateAssetPool(Game::ASSET_TYPE_TECHNIQUE_SET, 0x2000);
			Game::ReallocateAssetPool(Game::ASSET_TYPE_FONT, 32);
			Game::ReallocateAssetPool(Game::ASSET_TYPE_RAWFILE, 2048);
			Game::ReallocateAssetPool(Game::ASSET_TYPE_LEADERBOARD, 500);

			AssetHandler::RegisterInterface(new Assets::IFont_s());
			AssetHandler::RegisterInterface(new Assets::IWeapon());
			AssetHandler::RegisterInterface(new Assets::IXModel());
			AssetHandler::RegisterInterface(new Assets::IFxWorld());
			AssetHandler::RegisterInterface(new Assets::IMapEnts());
			AssetHandler::RegisterInterface(new Assets::IRawFile());
			AssetHandler::RegisterInterface(new Assets::IComWorld());
			AssetHandler::RegisterInterface(new Assets::IGfxImage());
			AssetHandler::RegisterInterface(new Assets::IGfxWorld());
			AssetHandler::RegisterInterface(new Assets::ISndCurve());
			AssetHandler::RegisterInterface(new Assets::IMaterial());
			AssetHandler::RegisterInterface(new Assets::IMenuList());
			AssetHandler::RegisterInterface(new Assets::IclipMap_t());
			AssetHandler::RegisterInterface(new Assets::ImenuDef_t());
			AssetHandler::RegisterInterface(new Assets::ITracerDef());
			AssetHandler::RegisterInterface(new Assets::IPhysPreset());
			AssetHandler::RegisterInterface(new Assets::IXAnimParts());
			AssetHandler::RegisterInterface(new Assets::IFxEffectDef());
			AssetHandler::RegisterInterface(new Assets::IGameWorldMp());
			AssetHandler::RegisterInterface(new Assets::IGameWorldSp());
			AssetHandler::RegisterInterface(new Assets::IGfxLightDef());
			AssetHandler::RegisterInterface(new Assets::ILoadedSound());
			AssetHandler::RegisterInterface(new Assets::IPhysCollmap());
			AssetHandler::RegisterInterface(new Assets::IStringTable());
			AssetHandler::RegisterInterface(new Assets::IXModelSurfs());
			AssetHandler::RegisterInterface(new Assets::ILocalizeEntry());
			AssetHandler::RegisterInterface(new Assets::Isnd_alias_list_t());
			AssetHandler::RegisterInterface(new Assets::IMaterialPixelShader());
			AssetHandler::RegisterInterface(new Assets::IMaterialTechniqueSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexShader());
			AssetHandler::RegisterInterface(new Assets::IStructuredDataDefSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexDeclaration());
		}
	}

	AssetHandler::~AssetHandler()
	{
		AssetHandler::ClearTemporaryAssets();

		for (auto i = AssetHandler::AssetInterfaces.begin(); i != AssetHandler::AssetInterfaces.end(); ++i)
		{
			delete i->second;
		}

		AssetHandler::Relocations.clear();
		AssetHandler::AssetInterfaces.clear();
		AssetHandler::RestrictSignal.clear();
		AssetHandler::TypeCallbacks.clear();
	}
}
