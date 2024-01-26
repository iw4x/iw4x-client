#include <STDInclude.hpp>

#include <stdint.h>

#include "IXModel.hpp"

namespace Assets
{

	void IXModel::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->model = builder->getIW4OfApi()->read<Game::XModel>(Game::XAssetType::ASSET_TYPE_XMODEL, name);

		if (header->model)
		{
			// ???
			if (header->model->physCollmap)
			{
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, { header->model->physCollmap });
			}

			if (header->model->physPreset)
			{
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, { header->model->physPreset });
			}

			for (size_t i = 0; i < header->model->numLods; i++)
			{
				const auto& info = header->model->lodInfo[i];
				Components::AssetHandler::StoreTemporaryAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, { info.modelSurfs });
			}
		}
	}

	void IXModel::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::XModel* asset = header.model;

		if (asset->boneNames)
		{
			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->addScriptString(asset->boneNames[i]);
			}
		}

		if (asset->materialHandles)
		{
			for (unsigned char i = 0; i < asset->numsurfs; ++i)
			{
				if (asset->materialHandles[i])
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materialHandles[i]);
				}
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (asset->lodInfo[i].modelSurfs)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, asset->lodInfo[i].modelSurfs);
			}
		}

		if (asset->physPreset)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset);
		}

		if (asset->physCollmap)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->physCollmap);
		}
	}

	void IXModel::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XModel, 304);

		Utils::Stream* buffer = builder->getBuffer();
		Game::XModel* asset = header.model;
		Game::XModel* dest = buffer->dest<Game::XModel>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->boneNames)
		{
			buffer->align(Utils::Stream::ALIGN_2);

			unsigned short* destBoneNames = buffer->dest<unsigned short>();
			buffer->saveArray(asset->boneNames, asset->numBones);

			for (char i = 0; i < asset->numBones; ++i)
			{
				builder->mapScriptString(destBoneNames[i]);
			}

			Utils::Stream::ClearPointer(&dest->boneNames);
		}

		if (asset->parentList)
		{
			if (builder->hasPointer(asset->parentList))
			{
				dest->parentList = builder->getPointer(asset->parentList);
			}
			else
			{
				builder->storePointer(asset->parentList);
				buffer->save(asset->parentList, asset->numBones - asset->numRootBones);
				Utils::Stream::ClearPointer(&dest->parentList);
			}
		}

		if (asset->quats)
		{
			if (builder->hasPointer(asset->quats))
			{
				dest->quats = builder->getPointer(asset->quats);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_2);
				builder->storePointer(asset->quats);
				buffer->saveArray(asset->quats, (asset->numBones - asset->numRootBones) * 4);
				Utils::Stream::ClearPointer(&dest->quats);
			}
		}

		if (asset->trans)
		{
			if (builder->hasPointer(asset->trans))
			{
				dest->trans = builder->getPointer(asset->trans);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->trans);
				buffer->saveArray(asset->trans, (asset->numBones - asset->numRootBones) * 3);
				Utils::Stream::ClearPointer(&dest->trans);
			}
		}

		if (asset->partClassification)
		{
			if (builder->hasPointer(asset->partClassification))
			{
				dest->partClassification = builder->getPointer(asset->partClassification);
			}
			else
			{
				builder->storePointer(asset->partClassification);
				buffer->save(asset->partClassification, asset->numBones);
				Utils::Stream::ClearPointer(&dest->partClassification);
			}
		}

		if (asset->baseMat)
		{
			AssertSize(Game::DObjAnimMat, 32);
			if (builder->hasPointer(asset->baseMat))
			{
				dest->baseMat = builder->getPointer(asset->baseMat);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);
				builder->storePointer(asset->baseMat);
				buffer->saveArray(asset->baseMat, asset->numBones);
				Utils::Stream::ClearPointer(&dest->baseMat);
			}

		}

		if (asset->materialHandles)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::Material** destMaterials = buffer->dest<Game::Material*>();
			buffer->saveArray(asset->materialHandles, asset->numsurfs);

			for (unsigned char i = 0; i < asset->numsurfs; ++i)
			{
				if (asset->materialHandles[i])
				{
					destMaterials[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->materialHandles[i]).material;
				}
			}

			Utils::Stream::ClearPointer(&dest->materialHandles);
		}

		// Save_XModelLodInfoArray
		{
			AssertSize(Game::XModelLodInfo, 44);

			for (int i = 0; i < 4; ++i)
			{
				if (asset->lodInfo[i].modelSurfs)
				{
					dest->lodInfo[i].modelSurfs = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, asset->lodInfo[i].modelSurfs).modelSurfs;
				}
			}
		}

		// Save_XModelCollSurfArray
		if (asset->collSurfs)
		{
			AssertSize(Game::XModelCollSurf_s, 44);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XModelCollSurf_s* destColSurfs = buffer->dest<Game::XModelCollSurf_s>();
			buffer->saveArray(asset->collSurfs, asset->numCollSurfs);

			for (int i = 0; i < asset->numCollSurfs; ++i)
			{
				Game::XModelCollSurf_s* destCollSurf = &destColSurfs[i];
				Game::XModelCollSurf_s* collSurf = &asset->collSurfs[i];

				if (collSurf->collTris)
				{
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->save(collSurf->collTris, 48, collSurf->numCollTris);
					Utils::Stream::ClearPointer(&destCollSurf->collTris);
				}
			}

			Utils::Stream::ClearPointer(&dest->collSurfs);
		}

		if (asset->boneInfo)
		{
			AssertSize(Game::XBoneInfo, 28);

			buffer->align(Utils::Stream::ALIGN_4);

			buffer->saveArray(asset->boneInfo, asset->numBones);
			Utils::Stream::ClearPointer(&dest->boneInfo);
		}

		if (asset->physPreset)
		{
			dest->physPreset = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSPRESET, asset->physPreset).physPreset;
		}

		if (asset->physCollmap)
		{
			dest->physCollmap = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->physCollmap).physCollmap;
		}

		buffer->popBlock();
	}


	uint8_t IXModel::GetIndexOfBone(const Game::XModel* model, std::string name)
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

	uint8_t IXModel::GetParentIndexOfBone(const Game::XModel* model, uint8_t index)
	{
		const auto parentIndex = index - model->parentList[index - model->numRootBones];
		return static_cast<uint8_t>(parentIndex);
	};

	void IXModel::SetParentIndexOfBone(Game::XModel* model, uint8_t boneIndex, uint8_t parentIndex)
	{
		if (boneIndex == SCHAR_MAX)
		{
			return;
		}

		model->parentList[boneIndex - model->numRootBones] = boneIndex - parentIndex;
	};

	std::string IXModel::GetParentOfBone(Game::XModel* model, uint8_t index)
	{
		const auto parentIndex = GetParentIndexOfBone(model, index);
		const auto boneName = Game::SL_ConvertToString(model->boneNames[parentIndex]);
		return boneName;
	};

	uint8_t IXModel::GetHighestAffectingBoneIndex(const Game::XModelLodInfo* lod)
	{
		uint8_t highestBoneIndex = 0;
		constexpr auto LENGTH = 6;

		{
			for (auto surfIndex = 0; surfIndex < lod->numsurfs; surfIndex++)
			{
				const auto surface = &lod->surfs[surfIndex];

				auto vertsBlendOffset = 0;

				int rebuiltPartBits[6]{};
				std::unordered_set<uint8_t> affectingBones{};

				const auto registerBoneAffectingSurface = [&](unsigned int offset) {
					uint8_t index = static_cast<uint8_t>(surface->vertInfo.vertsBlend[offset] / sizeof(Game::DObjSkelMat));
					highestBoneIndex = std::max(highestBoneIndex, index);
				};


				// 1 bone weight
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[0]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);

					vertsBlendOffset += 1;
				}

				// 2 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[1]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);

					vertsBlendOffset += 3;
				}

				// 3 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[2]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);
					registerBoneAffectingSurface(vertsBlendOffset + 3);

					vertsBlendOffset += 5;
				}

				// 4 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[3]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);
					registerBoneAffectingSurface(vertsBlendOffset + 3);
					registerBoneAffectingSurface(vertsBlendOffset + 5);

					vertsBlendOffset += 7;
				}

				for (auto vertListIndex = 0; vertListIndex < surface->vertListCount; vertListIndex++)
				{
					highestBoneIndex = std::max(highestBoneIndex, static_cast<uint8_t>(surface->vertList[vertListIndex].boneOffset / sizeof(Game::DObjSkelMat)));
				}
			}
		}

		return highestBoneIndex;
	};

	void IXModel::RebuildPartBits(Game::XModel* model)
	{
		constexpr auto LENGTH = 6;

		for (auto i = 0; i < model->numLods; i++)
		{
			const auto lod = &model->lodInfo[i];
			int lodPartBits[6]{};

			for (auto surfIndex = 0; surfIndex < lod->numsurfs; surfIndex++)
			{
				const auto surface = &lod->surfs[surfIndex];

				auto vertsBlendOffset = 0;

				int rebuiltPartBits[6]{};
				std::unordered_set<uint8_t> affectingBones{};

				const auto registerBoneAffectingSurface = [&](unsigned int offset) {
					uint8_t index = static_cast<uint8_t>(surface->vertInfo.vertsBlend[offset] / sizeof(Game::DObjSkelMat));

					assert(index >= 0);
					assert(index < model->numBones);

					affectingBones.emplace(index);
				};


				// 1 bone weight
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[0]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);

					vertsBlendOffset += 1;
				}

				// 2 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[1]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);

					vertsBlendOffset += 3;
				}

				// 3 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[2]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);
					registerBoneAffectingSurface(vertsBlendOffset + 3);

					vertsBlendOffset += 5;
				}

				// 4 bone weights
				for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[3]; vertIndex++)
				{
					registerBoneAffectingSurface(vertsBlendOffset + 0);
					registerBoneAffectingSurface(vertsBlendOffset + 1);
					registerBoneAffectingSurface(vertsBlendOffset + 3);
					registerBoneAffectingSurface(vertsBlendOffset + 5);

					vertsBlendOffset += 7;
				}

				for (auto vertListIndex = 0; vertListIndex < surface->vertListCount; vertListIndex++)
				{
					affectingBones.emplace(surface->vertList[vertListIndex].boneOffset / sizeof(Game::DObjSkelMat));
				}

				// Actually rebuilding
				for (const auto& boneIndex : affectingBones)
				{
					const auto bitPosition = 31 - boneIndex % 32;
					const auto groupIndex = boneIndex / 32;

					assert(groupIndex < 6);
					assert(groupIndex >= 0);

					rebuiltPartBits[groupIndex] |= 1 << bitPosition;
					lodPartBits[groupIndex] |= 1 << bitPosition;
				}

				std::memcpy(surface->partBits, rebuiltPartBits, 6 * sizeof(int32_t));
			}

			std::memcpy(lod->partBits, lodPartBits, 6 * sizeof(int32_t));
			std::memcpy(lod->modelSurfs->partBits, lodPartBits, 6 * sizeof(int32_t));

			// here's a little lesson in trickery:
			// We set the 192nd part bit to TRUE because it has no consequences
			//	but allows us to find out whether that surf was already converted in the past or not
			lod->partBits[LENGTH - 1] |= 0x1;
			lod->modelSurfs->partBits[LENGTH - 1] |= 0x1;
		}
	};


	uint8_t IXModel::InsertBone(Game::XModel* model, const std::string& boneName, const std::string& parentName, Utils::Memory::Allocator& allocator)
	{
		assert(GetIndexOfBone(model, boneName) == UCHAR_MAX);

		constexpr auto MAX_BONES = 192;

		assert(model->numBones < MAX_BONES);

		// Start with backing up parent links that we will have to restore
		// We'll restore them at the end
		std::map<std::string, std::string> parentsToRestore{};
		for (int i = model->numRootBones; i < model->numBones; i++)
		{
			parentsToRestore[Game::SL_ConvertToString(model->boneNames[i])] = GetParentOfBone(model, i);
		}

		const uint8_t newBoneCount = model->numBones + 1;
		const uint8_t newBoneCountMinusRoot = newBoneCount - model->numRootBones;

		const auto parentIndex = GetIndexOfBone(model, parentName);

		assert(parentIndex != UCHAR_MAX);

		const uint8_t atPosition = parentIndex + 1;

		const uint8_t newBoneIndex = atPosition;
		const uint8_t newBoneIndexMinusRoot = atPosition - model->numRootBones;

		Components::Logger::Print("Inserting bone {} at position {} (between {} and {})\n", boneName, atPosition, Game::SL_ConvertToString(model->boneNames[atPosition-1]), Game::SL_ConvertToString(model->boneNames[atPosition+1]));

		// Reallocate
		const auto newBoneNames = allocator.allocateArray<uint16_t>(newBoneCount);
		const auto newMats = allocator.allocateArray<Game::DObjAnimMat>(newBoneCount);
		const auto newBoneInfo = allocator.allocateArray<Game::XBoneInfo>(newBoneCount);
		const auto newPartsClassification = allocator.allocateArray<uint8_t>(newBoneCount);
		const auto newQuats = allocator.allocateArray<int16_t>(4 * newBoneCountMinusRoot);
		const auto newTrans = allocator.allocateArray<float>(3 * newBoneCountMinusRoot);
		const auto newParentList = allocator.allocateArray<uint8_t>(newBoneCountMinusRoot);

		const uint8_t lengthOfFirstPart = atPosition;
		const uint8_t lengthOfSecondPart = model->numBones - atPosition;

		const uint8_t lengthOfFirstPartM1 = atPosition - model->numRootBones;
		const uint8_t lengthOfSecondPartM1 = model->numBones - model->numRootBones - (atPosition - model->numRootBones);

		const uint8_t atPositionM1 = atPosition - model->numRootBones;

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
			std::memcpy(newPartsClassification, model->partClassification, lengthOfFirstPart);
			std::memcpy(newBoneInfo, model->boneInfo, sizeof(Game::XBoneInfo) * lengthOfFirstPart);
			std::memcpy(newQuats, model->quats, sizeof(uint16_t) * 4 * lengthOfFirstPartM1);
			std::memcpy(newTrans, model->trans, sizeof(float) * 3 * lengthOfFirstPartM1);
		}

		// Insert new bone
		{
			unsigned int name = Game::SL_GetString(boneName.data(), 0);
			Game::XBoneInfo boneInfo{};

			Game::DObjAnimMat mat{};

			// It's ABSOLUTE!
			mat = model->baseMat[parentIndex];

			boneInfo = model->boneInfo[parentIndex];

			// It's RELATIVE !
			uint16_t quat[4]{};
			quat[3] = 32767; // 0 0 0 32767

			float trans[3]{};

			mat.transWeight = 1.9999f; // Should be 1.9999 like everybody?

			newMats[newBoneIndex] = mat;
			newBoneInfo[newBoneIndex] = boneInfo;
			newBoneNames[newBoneIndex] = static_cast<uint16_t>(name);

			// TODO parts Classification

			std::memcpy(&newQuats[newBoneIndexMinusRoot * 4], quat, ARRAYSIZE(quat) * sizeof(uint16_t));
			std::memcpy(&newTrans[newBoneIndexMinusRoot * 3], trans, ARRAYSIZE(trans) * sizeof(float));
		}

		// Copy after
		if (lengthOfSecondPart > 0)
		{
			std::memcpy(&newBoneNames[atPosition + 1], &model->boneNames[atPosition], sizeof(uint16_t) * lengthOfSecondPart);
			std::memcpy(&newMats[atPosition + 1], &model->baseMat[atPosition], sizeof(Game::DObjAnimMat) * lengthOfSecondPart);
			std::memcpy(&newPartsClassification[atPosition+1], &model->partClassification[atPosition], lengthOfSecondPart);
			std::memcpy(&newBoneInfo[atPosition + 1], &model->boneInfo[atPosition], sizeof(Game::XBoneInfo) * lengthOfSecondPart);
			std::memcpy(&newQuats[(atPositionM1 + 1) * 4], &model->quats[atPositionM1 * 4], sizeof(uint16_t) * 4 * lengthOfSecondPartM1);
			std::memcpy(&newTrans[(atPositionM1 + 1) * 3], &model->trans[atPositionM1 * 3], sizeof(float) * 3 * lengthOfSecondPartM1);
		}

		//Game::Z_VirtualFree(model->baseMat);
		//Game::Z_VirtualFree(model->boneInfo);
		//Game::Z_VirtualFree(model->boneNames);
		//Game::Z_VirtualFree(model->quats);
		//Game::Z_VirtualFree(model->trans);
		//Game::Z_VirtualFree(model->parentList);

		// Assign reallocated
		model->baseMat = newMats;
		model->boneInfo = newBoneInfo;
		model->boneNames = newBoneNames;
		model->quats = newQuats;
		model->trans = newTrans;
		model->parentList = newParentList;

		model->numBones = newBoneCount;

		// Update vertex weight
		for (uint8_t lodIndex = 0; lodIndex < model->numLods; lodIndex++)
		{
			const auto lod = &model->lodInfo[lodIndex];

			if ((lod->partBits[5] & 0x1) == 0x1)
			{
				// surface lod already converted (more efficient)
				continue;
			}

			if (GetHighestAffectingBoneIndex(lod) >= model->numBones)
			{
				// surface lod already converted (more accurate)
				continue;
			}

			for (int surfIndex = 0; surfIndex < lod->modelSurfs->numsurfs; surfIndex++)
			{
				auto vertsBlendOffset = 0u;

				const auto surface = &lod->modelSurfs->surfs[surfIndex];

				static_assert(sizeof(Game::DObjSkelMat) == 64);

				{
					const auto fixVertexBlendIndex = [&](unsigned int offset) {

						int index = static_cast<int>(surface->vertInfo.vertsBlend[offset] / sizeof(Game::DObjSkelMat));

						if (index >= atPosition)
						{
							index++;

							if (index < 0 || index >= model->numBones)
							{
								Components::Logger::Print("Unexpected 'bone index' {} out of {} bones while working vertex blend of model {} lod {} surf {}\n", index, model->numBones, model->name, lodIndex, surfIndex);
								assert(false);
							}

							surface->vertInfo.vertsBlend[offset] = static_cast<unsigned short>(index * sizeof(Game::DObjSkelMat));
						}
					};

					//  Fix bone offsets
					if (surface->vertList)
					{
						for (auto vertListIndex = 0u; vertListIndex < surface->vertListCount; vertListIndex++)
						{
							const auto vertList = &surface->vertList[vertListIndex];

							auto index = vertList->boneOffset / sizeof(Game::DObjSkelMat);
							if (index >= atPosition)
							{
								index++;

								if (index < 0 || index >= model->numBones)
								{
									Components::Logger::Print("Unexpected 'bone index' {} out of {} bones while working list blend of model {} lod {} surf {}\n", index, model->numBones, model->name, lodIndex, surfIndex);
									assert(false);
								}

								vertList->boneOffset = static_cast<unsigned short>(index * sizeof(Game::DObjSkelMat));
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

		SetParentIndexOfBone(model, atPosition, parentIndex);

		// Restore parents
		for (const auto& kv : parentsToRestore)
		{
			// Fix parents
			const auto key = kv.first;
			const auto beforeVal = kv.second;

			const auto parentIndex = GetIndexOfBone(model, beforeVal);
			const auto index = GetIndexOfBone(model, key);
			SetParentIndexOfBone(model, index, parentIndex);
		}

#if DEBUG
		// check
		for (const auto& kv : parentsToRestore)
		{
			const auto key = kv.first;
			const auto beforeVal = kv.second;

			const auto index = GetIndexOfBone(model, key);
			const auto afterVal = GetParentOfBone(model, index);

			if (beforeVal != afterVal)
			{
				printf("");
			}
		}
		//
#endif

		return atPosition; // Bone index of added bone
	};


	void IXModel::TransferWeights(Game::XModel* model, const uint8_t origin, const uint8_t destination)
	{
		// Does not work
		return;

		const auto originalWeights = model->baseMat[origin].transWeight;
		model->baseMat[origin].transWeight = model->baseMat[destination].transWeight;
		model->baseMat[destination].transWeight = originalWeights;

		for (int i = 0; i < model->numLods; i++)
		{
			const auto lod = &model->lodInfo[i];

			for (int surfIndex = 0; surfIndex < lod->modelSurfs->numsurfs; surfIndex++)
			{
				auto vertsBlendOffset = 0u;

				const auto surface = &lod->modelSurfs->surfs[surfIndex];
				{
					const auto transferVertexBlendIndex = [&](unsigned int offset) {
						int index = static_cast<int>(surface->vertInfo.vertsBlend[offset] / sizeof(Game::DObjSkelMat));
						if (index == origin)
						{
							if (index < 0 || index >= model->numBones - 1)
							{
								assert(false);
							}

							index = destination;

							surface->vertInfo.vertsBlend[offset] = static_cast<unsigned short>(index * sizeof(Game::DObjSkelMat));
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

							if (index == origin)
							{
								index = destination;
								vertList->boneOffset = static_cast<unsigned short>(index * sizeof(Game::DObjSkelMat));
							}
						}
					}

					// 1 bone weight
					for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[0]; vertIndex++)
					{
						transferVertexBlendIndex(vertsBlendOffset + 0);

						vertsBlendOffset += 1;
					}

					// 2 bone weights
					for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[1]; vertIndex++)
					{
						transferVertexBlendIndex(vertsBlendOffset + 0);
						transferVertexBlendIndex(vertsBlendOffset + 1);

						vertsBlendOffset += 3;
					}

					// 3 bone weights
					for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[2]; vertIndex++)
					{
						transferVertexBlendIndex(vertsBlendOffset + 0);
						transferVertexBlendIndex(vertsBlendOffset + 1);
						transferVertexBlendIndex(vertsBlendOffset + 3);


						vertsBlendOffset += 5;
					}

					// 4 bone weights
					for (auto vertIndex = 0; vertIndex < surface->vertInfo.vertCount[3]; vertIndex++)
					{
						transferVertexBlendIndex(vertsBlendOffset + 0);
						transferVertexBlendIndex(vertsBlendOffset + 1);
						transferVertexBlendIndex(vertsBlendOffset + 3);
						transferVertexBlendIndex(vertsBlendOffset + 5);

						vertsBlendOffset += 7;
					}
				}
			}
		}
	};


	void IXModel::ConvertPlayerModelFromSingleplayerToMultiplayer(Game::XModel* model, Utils::Memory::Allocator& allocator)
	{
		auto indexOfSpine = GetIndexOfBone(model, "j_spinelower");
		if (indexOfSpine < UCHAR_MAX) // It has a spine so it must be some sort of humanoid
		{
			const auto nameOfParent = GetParentOfBone(model, indexOfSpine);

			if (GetIndexOfBone(model, "torso_stabilizer") == UCHAR_MAX) // Singleplayer model is likely
			{

				Components::Logger::Print("Converting {}\n", model->name);

				// No stabilizer - let's do surgery
				// We're trying to get there:
				//	tag_origin
				//		j_main_root
				//			pelvis
				//				j_hip_le
				//				j_hip_ri
				//				tag_stowed_hip_rear
				//				torso_stabilizer
				//					j_spinelower
				//						back_low
				//							j_spineupper
				//								back_mid
				//									j_spine4


				const auto root = GetIndexOfBone(model, "j_mainroot");
				if (root < UCHAR_MAX) {

#if true
					//// Works
					//InsertBone(model, "offsetron_the_great_offsetter_of_bones", "j_mainroot", allocator);

					//// Breaks the model
					//InsertBone(model, "offsetron2_the_greater_offsetter_of_bones", "j_mainroot", allocator);

					//for (auto lodIndex = 0; lodIndex < model->numLods; lodIndex++)
					//{
					//	convertedSurfs.emplace(model->lodInfo[lodIndex].modelSurfs);
					//}

					//RebuildPartBits(model);

					//return;

					// Add pelvis
					const uint8_t indexOfPelvis = InsertBone(model, "pelvis", "j_mainroot", allocator);

					TransferWeights(model, root, indexOfPelvis);

					SetParentIndexOfBone(model, GetIndexOfBone(model, "j_hip_le"), indexOfPelvis);
					SetParentIndexOfBone(model, GetIndexOfBone(model, "j_hip_ri"), indexOfPelvis);
					SetParentIndexOfBone(model, GetIndexOfBone(model, "tag_stowed_hip_rear"), indexOfPelvis);

					const uint8_t torsoStabilizer = InsertBone(model, "torso_stabilizer", "pelvis", allocator);
					SetParentIndexOfBone(model, GetIndexOfBone(model, "j_spinelower"), torsoStabilizer);

					const uint8_t backLow = InsertBone(model, "back_low", "j_spinelower", allocator);
					TransferWeights(model, GetIndexOfBone(model, "j_spinelower"), backLow);
					SetParentIndexOfBone(model, GetIndexOfBone(model, "j_spineupper"), backLow);

					const uint8_t backMid = InsertBone(model, "back_mid", "j_spineupper", allocator);
					TransferWeights(model, GetIndexOfBone(model, "j_spineupper"), backMid);
					SetParentIndexOfBone(model, GetIndexOfBone(model, "j_spine4"), backMid);


					assert(root == GetIndexOfBone(model, "j_mainroot"));
					assert(indexOfPelvis == GetIndexOfBone(model, "pelvis"));
					assert(backLow == GetIndexOfBone(model, "back_low"));
					assert(backMid == GetIndexOfBone(model, "back_mid"));

					// Fix up torso stabilizer
					model->baseMat[torsoStabilizer].quat[0] = 0.F;
					model->baseMat[torsoStabilizer].quat[1] = 0.F;
					model->baseMat[torsoStabilizer].quat[2] = 0.F;
					model->baseMat[torsoStabilizer].quat[3] = 1.F;

					const auto spineLowerM1 = GetIndexOfBone(model, "j_spinelower") - model->numRootBones;

					// This doesn't feel like it should be necessary
					model->trans[spineLowerM1 * 3 + 0] = 0.069828756;
					model->trans[spineLowerM1 * 3 + 1] = -0.0f;
					model->trans[spineLowerM1 * 3 + 2] = 5.2035017F;

					//// Euler -180.000   88.572  -90.000
					model->quats[(torsoStabilizer - model->numRootBones) * 4 + 0] = 16179; // 0.4952
					model->quats[(torsoStabilizer - model->numRootBones) * 4 + 1] = 16586; // 0.5077
					model->quats[(torsoStabilizer - model->numRootBones) * 4 + 2] = 16586; // 0.5077
					model->quats[(torsoStabilizer - model->numRootBones) * 4 + 3] = 16178; // 0.4952

#else
					const uint8_t torsoStabilizer = insertBone("torso_stabilizer", "j_mainroot");
					transferWeights(getIndexOfBone("j_mainroot"), getIndexOfBone("torso_stabilizer"));
					setParentIndexOfBone(getIndexOfBone("j_spinelower"), torsoStabilizer);
#endif

					RebuildPartBits(model);
				}
			}
			printf("");
		}
	}

}
