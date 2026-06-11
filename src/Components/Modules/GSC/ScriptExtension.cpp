
#include <Components/Modules/Events.hpp>
#include <Components/Modules/ModelCache.hpp>
#include <Components/Modules/ModelSurfs.hpp>
#include <Components/Modules/Scheduler.hpp>
#include <Components/Modules/ServerCommands.hpp>

#include "ScriptExtension.hpp"
#include "Script.hpp"

#include <mutex>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

namespace Components::GSC
{
	namespace
	{
		constexpr auto ResizeServerCommand = 23;
		constexpr auto MinModelScale = 0.01f;
		constexpr auto MaxModelScale = 64.0f;
		constexpr auto ModelScaleUpdateInterval = std::chrono::milliseconds(0);
		constexpr auto ScriptModelAnimationClearDelay = std::chrono::milliseconds(75);
		constexpr auto ScriptModelAnimationResumeDelay = std::chrono::milliseconds(50);
		constexpr auto ScriptModelAnimIndexOffset = 0x5C;
		constexpr auto ScriptModelAnimStartTimeOffset = 0x60;

		std::mutex SkinnedCacheScalesMutex;

		bool IsValidScale(const float scale)
		{
			return std::isfinite(scale) && scale >= MinModelScale && scale <= MaxModelScale;
		}

		float Lerp(const float from, const float to, const float fraction)
		{
			return from + ((to - from) * fraction);
		}

		std::chrono::milliseconds GetScaleDuration(const float time)
		{
			if (time <= 0.0f)
			{
				return std::chrono::milliseconds(0);
			}

			return std::chrono::milliseconds(std::max(1, static_cast<int>(std::min(time * 1000.0f, static_cast<float>(std::numeric_limits<int>::max())))));
		}

		void ScaleBounds(Game::Bounds& bounds, const float scale)
		{
			for (auto component = 0; component < 3; ++component)
			{
				bounds.midPoint[component] *= scale;
				bounds.halfSize[component] *= scale;
			}
		}
	}

	std::unordered_map<const char*, const char*> ScriptExtension::ReplacedFunctions;
	const char* ScriptExtension::ReplacedPos = nullptr;
	std::vector<ScriptExtension::ModelScaleTransition> ScriptExtension::ModelScaleTransitions;
	std::vector<ScriptExtension::PendingModelResetCleanup> ScriptExtension::PendingModelResetCleanups;
	std::vector<ScriptExtension::PausedScriptModelAnimation> ScriptExtension::PausedScriptModelAnimations;
	std::unordered_map<Game::XModel*, ScriptExtension::VisualScaledModel> ScriptExtension::VisualScaledModels;
	std::vector<ScriptExtension::SceneScaleTransition> ScriptExtension::SceneScaleTransitions;
	std::unordered_map<std::string, float> ScriptExtension::ModelSceneScales;
	std::unordered_map<int, float> ScriptExtension::EntitySceneScales;
	int ScriptExtension::LastSceneScaleApplyTime = std::numeric_limits<int>::min();
	std::unordered_map<int, int> ScriptExtension::EntityModelCloneIndexes;
	std::unordered_map<int, std::string> ScriptExtension::EntityModelCloneSources;
	std::unordered_map<int, std::array<float, 3>> ScriptExtension::EntityModelCloneOrigins;
	std::unordered_map<int, Game::XModel*> ScriptExtension::EntityVisualModelOverrides;
	std::vector<ScriptExtension::PendingVisualModelOverride> ScriptExtension::PendingVisualModelOverrides;
	std::unordered_map<std::string, Game::XModel*> ScriptExtension::GlobalVisualModelClones;
	std::unordered_map<std::string, Game::XModel*> ScriptExtension::GlobalVisualModelOverrides;
	std::unordered_map<Game::DObj*, std::vector<Game::DObjAnimMat>> ScriptExtension::ScaledDObjAnimMats;
	std::unordered_map<void*, ScriptExtension::SkinnedCacheScale> ScriptExtension::SkinnedCacheScales;
	thread_local ScriptExtension::SkinnedCacheScale ScriptExtension::CurrentSkinnedCacheScale{};
	thread_local bool ScriptExtension::HasCurrentSkinnedCacheScale = false;

	const char* ScriptExtension::GetCodePosForParam(int index)
	{
		if (static_cast<unsigned int>(index) >= Game::scrVmPub->outparamcount)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "GetCodePosForParam: Index is out of range!");
			return "";
		}

		const auto* value = &Game::scrVmPub->top[-index];

		if (value->type != Game::VAR_FUNCTION)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "GetCodePosForParam: Expects a function as parameter!");
			return "";
		}

		return value->u.codePosValue;
	}

	void ScriptExtension::GetReplacedPos(const char* pos)
	{
		if (!pos)
		{
			// This seems to happen often and there should not be pointers to NULL in our map
			return;
		}

		if (const auto itr = ReplacedFunctions.find(pos); itr != ReplacedFunctions.end())
		{
			ReplacedPos = itr->second;
		}
	}

	void ScriptExtension::SetReplacedPos(const char* what, const char* with)
	{
		if (!*what || !*with)
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "Invalid parameters passed to ReplacedFunctions\n");
			return;
		}

		if (ReplacedFunctions.contains(what))
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "ReplacedFunctions already contains codePosValue for a function\n");
		}

		ReplacedFunctions[what] = with;
	}

	__declspec(naked) void ScriptExtension::VMExecuteInternalStub()
	{
		__asm
		{
			pushad

			push edx
			call GetReplacedPos

			pop edx
			popad

			cmp ReplacedPos, 0
			jne SetPos

			movzx eax, byte ptr [edx]
			inc edx

		Loc1:
			cmp eax, 0x8B

			push ecx

			mov ecx, 0x2045094
			mov [ecx], eax

			mov ecx, 0x2040CD4
			mov [ecx], edx

			pop ecx

			push 0x61E944
			ret

		SetPos:
			mov edx, ReplacedPos
			mov ReplacedPos, 0

			movzx eax, byte ptr [edx]
			inc edx

			jmp Loc1
		}
	}

	Game::XModel* ScriptExtension::GetResizeModel(const char* name, const unsigned int paramIndex)
	{
		if (!name || !*name)
		{
			Game::Scr_ParamError(paramIndex, "ResizeModel: Illegal model parameter!");
			return nullptr;
		}

		auto* model = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, name).model;
		if (!model || Game::DB_IsXAssetDefault(Game::ASSET_TYPE_XMODEL, name))
		{
			Game::Scr_ParamError(paramIndex, Utils::String::VA("ResizeModel: xmodel '%s' does not exist", name));
			return nullptr;
		}

		return model;
	}

	void ScriptExtension::RemoveModelScaleTransition(Game::XModel* model)
	{
		std::erase_if(ModelScaleTransitions, [model](const ModelScaleTransition& transition)
		{
			return transition.model == model;
		});
	}

	float ScriptExtension::GetXModelVisualScale(Game::XModel* model)
	{
		if (const auto visualModel = VisualScaledModels.find(model); visualModel != VisualScaledModels.end())
		{
			return visualModel->second.scale;
		}

		return model ? model->scale : 1.0f;
	}

	float ScriptExtension::GetXModelResetScale(Game::XModel* model)
	{
		if (const auto visualModel = VisualScaledModels.find(model); visualModel != VisualScaledModels.end() && visualModel->second.initialized)
		{
			return visualModel->second.originalScale;
		}

		return model ? model->scale : 1.0f;
	}

	void ScriptExtension::SetXModelVisualSkeletonScale(Game::XModel* model, VisualScaledModel& visualModel, const float scale)
	{
		if (!model)
		{
			return;
		}

		if (!visualModel.originalBaseMats.empty())
		{
			visualModel.scaledBaseMats = visualModel.originalBaseMats;
			for (auto& boneMat : visualModel.scaledBaseMats)
			{
				boneMat.trans[0] *= scale;
				boneMat.trans[1] *= scale;
				boneMat.trans[2] *= scale;
			}

			model->baseMat = visualModel.scaledBaseMats.data();
		}
		else
		{
			model->baseMat = visualModel.originalBaseMat;
		}

		if (!visualModel.originalTransValues.empty())
		{
			visualModel.scaledTransValues = visualModel.originalTransValues;
			for (auto& value : visualModel.scaledTransValues)
			{
				value *= scale;
			}

			model->trans = visualModel.scaledTransValues.data();
		}
		else
		{
			model->trans = visualModel.originalTrans;
		}

		if (!visualModel.originalBoneInfos.empty())
		{
			visualModel.scaledBoneInfos = visualModel.originalBoneInfos;
			for (auto& boneInfo : visualModel.scaledBoneInfos)
			{
				ScaleBounds(boneInfo.bounds, scale);
				boneInfo.radiusSquared *= scale * scale;
			}

			model->boneInfo = visualModel.scaledBoneInfos.data();
		}
		else
		{
			model->boneInfo = visualModel.originalBoneInfo;
		}
	}

	void ScriptExtension::RestoreXModelVisualScale(Game::XModel* model, const VisualScaledModel& visualModel)
	{
		if (!model || !visualModel.initialized)
		{
			return;
		}

		std::memcpy(model->lodInfo, visualModel.originalLodInfo, sizeof(model->lodInfo));
		model->baseMat = visualModel.originalBaseMat;
		model->trans = visualModel.originalTrans;
		model->boneInfo = visualModel.originalBoneInfo;
		model->radius = visualModel.originalRadius;
		model->bounds = visualModel.originalBounds;
		model->scale = visualModel.originalScale;
	}

	void ScriptExtension::SetXModelVisualScale(Game::XModel* model, const float scale)
	{
		if (!model)
		{
			return;
		}

		auto& visualModel = VisualScaledModels[model];
		if (!visualModel.initialized)
		{
			std::memcpy(visualModel.originalLodInfo, model->lodInfo, sizeof(model->lodInfo));
			std::memset(visualModel.scaledSurfs, 0, sizeof(visualModel.scaledSurfs));
			visualModel.originalRadius = model->radius;
			visualModel.originalBounds = model->bounds;
			visualModel.originalScale = model->scale;
			visualModel.originalBaseMat = model->baseMat;
			visualModel.originalTrans = model->trans;
			visualModel.originalBoneInfo = model->boneInfo;

			const auto boneCount = static_cast<std::size_t>(model->numBones);
			if (visualModel.originalBaseMat && boneCount)
			{
				visualModel.originalBaseMats.assign(visualModel.originalBaseMat, visualModel.originalBaseMat + boneCount);
			}

			const auto rootBoneCount = static_cast<std::size_t>(model->numRootBones);
			const auto transValueCount = boneCount > rootBoneCount ? (boneCount - rootBoneCount) * 3u : 0u;
			if (visualModel.originalTrans && transValueCount)
			{
				visualModel.originalTransValues.assign(visualModel.originalTrans, visualModel.originalTrans + transValueCount);
			}

			if (visualModel.originalBoneInfo && boneCount)
			{
				visualModel.originalBoneInfos.assign(visualModel.originalBoneInfo, visualModel.originalBoneInfo + boneCount);
			}

			visualModel.scale = model->scale;
			visualModel.initialized = true;
		}

		for (auto lodIndex = 0; lodIndex < ARRAYSIZE(model->lodInfo); ++lodIndex)
		{
			const auto& originalLod = visualModel.originalLodInfo[lodIndex];
			if (!originalLod.modelSurfs || !originalLod.modelSurfs->surfs)
			{
				continue;
			}

			if (!visualModel.scaledSurfs[lodIndex])
			{
				visualModel.scaledSurfs[lodIndex] = ModelSurfs::CloneAndScaleSurfaces(
					originalLod.modelSurfs,
					Utils::String::VA("resize_%s_lod%i", model->name, lodIndex),
					scale);
			}
			else
			{
				ModelSurfs::UpdateScaledSurfaces(visualModel.scaledSurfs[lodIndex], originalLod.modelSurfs, scale);
			}

			if (visualModel.scaledSurfs[lodIndex])
			{
				model->lodInfo[lodIndex].modelSurfs = visualModel.scaledSurfs[lodIndex];
				model->lodInfo[lodIndex].surfs = visualModel.scaledSurfs[lodIndex]->surfs;
				model->lodInfo[lodIndex].numsurfs = visualModel.scaledSurfs[lodIndex]->numsurfs;
			}
		}

		model->scale = scale;
		model->radius = visualModel.originalRadius * scale;
		model->bounds = visualModel.originalBounds;
		ScaleBounds(model->bounds, scale);
		SetXModelVisualSkeletonScale(model, visualModel, scale);
		visualModel.scale = scale;
	}

	void ScriptExtension::ResetXModelVisualScale(Game::XModel* model)
	{
		if (!model)
		{
			return;
		}

		RemoveModelScaleTransition(model);

		const auto visualModel = VisualScaledModels.find(model);
		if (visualModel == VisualScaledModels.end())
		{
			return;
		}

		if (visualModel->second.initialized)
		{
			RestoreXModelVisualScale(model, visualModel->second);
		}

		VisualScaledModels.erase(visualModel);
	}

	void ScriptExtension::ResizeXModel(Game::XModel* model, const float targetScale, const float time)
	{
		ResizeXModelDelayed(model, targetScale, time, std::chrono::milliseconds(0));
	}

	void ScriptExtension::ResizeXModelDelayed(Game::XModel* model, const float targetScale, const float time, const std::chrono::milliseconds delay)
	{
		if (!model)
		{
			return;
		}

		RemoveModelScaleTransition(model);

		if (time <= 0.0f && delay.count() <= 0)
		{
			SetXModelVisualScale(model, targetScale);
			return;
		}

		const auto duration = GetScaleDuration(time);

		const auto now = std::chrono::steady_clock::now();
		const auto startTime = now + std::max(delay, std::chrono::milliseconds(0));
		ModelScaleTransitions.push_back({
			model,
			GetXModelVisualScale(model),
			targetScale,
			startTime,
			startTime - ModelScaleUpdateInterval,
			duration,
		});
	}

	void ScriptExtension::ResizeXModelsByName(const std::string& modelName, Game::XModel* model, const float targetScale, const float time, const std::chrono::milliseconds delay)
	{
		RemovePendingModelResetCleanup(VisualModelOverrideTarget::ModelName, modelName, 0);
		ModelSceneScales.erase(modelName);
		RemoveSceneScaleTransition(SceneScaleTarget::ModelName, modelName, 0);

		if (auto* clone = GetOrCreateGlobalModelClone(modelName, model))
		{
			ScheduleGlobalVisualModelOverride(modelName, clone, delay);
			ResizeXModelDelayed(clone, targetScale, time, delay);
		}

		for (const auto& [entNum, cloneIndex] : EntityModelCloneIndexes)
		{
			auto* clone = GetModelByIndex(cloneIndex);
			if (!clone || clone == model)
			{
				continue;
			}

			const auto sourceModel = EntityModelCloneSources.find(entNum);
			if (sourceModel == EntityModelCloneSources.end() || sourceModel->second != modelName)
			{
				continue;
			}

			ScheduleEntityVisualModelOverride(entNum, clone, delay);
			ResizeXModelDelayed(clone, targetScale, time, delay);
		}
	}

	void ScriptExtension::ResetXModelsByName(const std::string& modelName, Game::XModel* model, const float time, const std::chrono::milliseconds delay)
	{
		RemovePendingModelResetCleanup(VisualModelOverrideTarget::ModelName, modelName, 0);
		RemovePendingVisualModelOverride(VisualModelOverrideTarget::ModelName, modelName, 0);
		ModelSceneScales.erase(modelName);
		RemoveSceneScaleTransition(SceneScaleTarget::ModelName, modelName, 0);

		if (time > 0.0f || delay.count() > 0)
		{
			auto scheduledReset = false;
			if (const auto clone = GlobalVisualModelClones.find(modelName); clone != GlobalVisualModelClones.end() && clone->second)
			{
				ResizeXModelDelayed(clone->second, GetXModelResetScale(clone->second), time, delay);
				scheduledReset = true;
			}

			for (const auto& [entNum, clone] : EntityVisualModelOverrides)
			{
				static_cast<void>(entNum);

				if (!clone || clone == model)
				{
					continue;
				}

				const auto sourceModel = EntityModelCloneSources.find(entNum);
				if (sourceModel == EntityModelCloneSources.end() || sourceModel->second != modelName)
				{
					continue;
				}

				ResizeXModelDelayed(clone, GetXModelResetScale(clone), time, delay);
				scheduledReset = true;
			}

			if (scheduledReset)
			{
				ScheduleModelResetCleanup(VisualModelOverrideTarget::ModelName, modelName, 0, false, time, delay);
				return;
			}
		}

		if (const auto clone = GlobalVisualModelClones.find(modelName); clone != GlobalVisualModelClones.end())
		{
			ResetXModelVisualScale(clone->second);
			GlobalVisualModelClones.erase(clone);
		}

		GlobalVisualModelOverrides.erase(modelName);

		for (const auto& [entNum, clone] : EntityVisualModelOverrides)
		{
			static_cast<void>(entNum);

			if (!clone || clone == model)
			{
				continue;
			}

			const auto sourceModel = EntityModelCloneSources.find(entNum);
			if (sourceModel == EntityModelCloneSources.end() || sourceModel->second != modelName)
			{
				continue;
			}

			ResetXModelVisualScale(clone);
		}
	}

	void ScriptExtension::ScheduleModelResetCleanup(const VisualModelOverrideTarget target, const std::string& modelName, const int entNum, const bool restoreServerEntity, const float time, const std::chrono::milliseconds delay)
	{
		RemovePendingModelResetCleanup(target, modelName, entNum);

		const auto cleanupDelay = std::max(delay, std::chrono::milliseconds(0)) + GetScaleDuration(time);
		PendingModelResetCleanups.push_back({
			target,
			modelName,
			entNum,
			restoreServerEntity,
			std::chrono::steady_clock::now() + cleanupDelay,
		});
	}

	void ScriptExtension::RemovePendingModelResetCleanup(const VisualModelOverrideTarget target, const std::string& modelName, const int entNum)
	{
		std::erase_if(PendingModelResetCleanups, [target, &modelName, entNum](const PendingModelResetCleanup& cleanup)
		{
			if (cleanup.target != target)
			{
				return false;
			}

			if (target == VisualModelOverrideTarget::Entity)
			{
				return cleanup.entNum == entNum;
			}

			return cleanup.modelName == modelName;
		});
	}

	void ScriptExtension::UpdatePendingModelResetCleanups()
	{
		const auto now = std::chrono::steady_clock::now();
		std::vector<PendingModelResetCleanup> dueCleanups;

		for (auto i = PendingModelResetCleanups.begin(); i != PendingModelResetCleanups.end();)
		{
			if (now < i->cleanupTime)
			{
				++i;
				continue;
			}

			dueCleanups.push_back(*i);
			i = PendingModelResetCleanups.erase(i);
		}

		for (const auto& cleanup : dueCleanups)
		{
			if (cleanup.target == VisualModelOverrideTarget::Entity)
			{
				ClearEntityModelClone(cleanup.entNum, cleanup.restoreServerEntity);
				continue;
			}

			ResetXModelsByName(cleanup.modelName, nullptr, 0.0f, std::chrono::milliseconds(0));
		}
	}

	void ScriptExtension::UpdateModelScaleTransitions()
	{
		const auto now = std::chrono::steady_clock::now();

		for (auto i = ModelScaleTransitions.begin(); i != ModelScaleTransitions.end();)
		{
			if (!i->model)
			{
				i = ModelScaleTransitions.erase(i);
				continue;
			}

			if (now < i->startTime)
			{
				++i;
				continue;
			}

			const auto elapsed = now - i->startTime;
			if (i->duration.count() <= 0 || elapsed >= i->duration)
			{
				SetXModelVisualScale(i->model, i->targetScale);
				i = ModelScaleTransitions.erase(i);
				continue;
			}

			if (now - i->lastUpdateTime < ModelScaleUpdateInterval)
			{
				++i;
				continue;
			}

			const auto fraction = std::clamp(static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()) / static_cast<float>(i->duration.count()), 0.0f, 1.0f);
			SetXModelVisualScale(i->model, Lerp(i->startScale, i->targetScale, fraction));
			i->lastUpdateTime = now;
			++i;
		}

		UpdatePendingModelResetCleanups();
	}

	void ScriptExtension::ClearVisualScaledModels()
	{
		for (auto& [model, visualModel] : VisualScaledModels)
		{
			if (!model)
			{
				continue;
			}

			RestoreXModelVisualScale(model, visualModel);
		}

		VisualScaledModels.clear();
		ModelScaleTransitions.clear();
		PendingModelResetCleanups.clear();
		PendingVisualModelOverrides.clear();
		GlobalVisualModelClones.clear();
		GlobalVisualModelOverrides.clear();
	}

	bool ScriptExtension::IsScriptModelEntity(const Game::gentity_s* ent)
	{
		return ent && ent->s.eType == 6 && Game::scr_const && ent->classname == Game::scr_const->script_model;
	}

	int& ScriptExtension::ScriptModelAnimIndex(Game::gentity_s* ent)
	{
		return *reinterpret_cast<int*>(reinterpret_cast<char*>(ent) + ScriptModelAnimIndexOffset);
	}

	int& ScriptExtension::ScriptModelAnimStartTime(Game::gentity_s* ent)
	{
		return *reinterpret_cast<int*>(reinterpret_cast<char*>(ent) + ScriptModelAnimStartTimeOffset);
	}

	std::chrono::milliseconds ScriptExtension::PauseScriptModelAnimationForResize(Game::gentity_s* ent, const float time)
	{
		static_cast<void>(time);

		if (!IsScriptModelEntity(ent))
		{
			return std::chrono::milliseconds(0);
		}

		if (ScriptModelAnimIndex(ent) == 0)
		{
			return std::chrono::milliseconds(0);
		}

		return ScriptModelAnimationClearDelay;
	}

	std::chrono::milliseconds ScriptExtension::PauseScriptModelAnimationsForResize(const std::string& modelName, const float time)
	{
		auto delay = std::chrono::milliseconds(0);
		const auto entityLimit = Game::level ? std::min<int>(Game::level->num_entities, static_cast<int>(Game::MAX_GENTITIES)) : static_cast<int>(Game::MAX_GENTITIES);

		for (auto entNum = 0; entNum < entityLimit; ++entNum)
		{
			auto* ent = &Game::g_entities[entNum];
			if (!IsScriptModelEntity(ent))
			{
				continue;
			}

			auto matchesModel = false;
			if (const auto sourceModel = EntityModelCloneSources.find(entNum); sourceModel != EntityModelCloneSources.end())
			{
				matchesModel = sourceModel->second == modelName;
			}
			else if (auto* model = GetModelByIndex(ent->model); model && model->name)
			{
				matchesModel = modelName == model->name;
			}

			if (!matchesModel)
			{
				continue;
			}

			delay = std::max(delay, PauseScriptModelAnimationForResize(ent, time));
		}

		return delay;
	}

	void ScriptExtension::UpdatePausedScriptModelAnimations()
	{
		if (!Game::level)
		{
			ClearPausedScriptModelAnimations();
			return;
		}

		const auto now = std::chrono::steady_clock::now();
		for (auto i = PausedScriptModelAnimations.begin(); i != PausedScriptModelAnimations.end();)
		{
			if (now < i->resumeTime)
			{
				++i;
				continue;
			}

			if (i->entNum >= 0 && i->entNum < static_cast<int>(Game::MAX_GENTITIES))
			{
				auto* ent = &Game::g_entities[i->entNum];
				if (IsScriptModelEntity(ent) && ScriptModelAnimIndex(ent) == 0 && i->animIndex != 0)
				{
					ScriptModelAnimIndex(ent) = i->animIndex;
					ScriptModelAnimStartTime(ent) = Game::level->time;
				}
			}

			i = PausedScriptModelAnimations.erase(i);
		}
	}

	void ScriptExtension::ClearPausedScriptModelAnimations()
	{
		PausedScriptModelAnimations.clear();
	}

	void ScriptExtension::ScheduleEntityVisualModelOverride(const int entNum, Game::XModel* model, const std::chrono::milliseconds delay)
	{
		RemovePendingVisualModelOverride(VisualModelOverrideTarget::Entity, {}, entNum);

		if (!model)
		{
			EntityVisualModelOverrides.erase(entNum);
			return;
		}

		if (delay.count() <= 0)
		{
			EntityVisualModelOverrides[entNum] = model;
			return;
		}

		PendingVisualModelOverrides.push_back({
			VisualModelOverrideTarget::Entity,
			{},
			entNum,
			model,
			std::chrono::steady_clock::now() + delay,
		});
	}

	void ScriptExtension::ScheduleGlobalVisualModelOverride(const std::string& modelName, Game::XModel* model, const std::chrono::milliseconds delay)
	{
		RemovePendingVisualModelOverride(VisualModelOverrideTarget::ModelName, modelName, 0);

		if (!model)
		{
			GlobalVisualModelOverrides.erase(modelName);
			return;
		}

		if (delay.count() <= 0)
		{
			GlobalVisualModelOverrides[modelName] = model;
			return;
		}

		PendingVisualModelOverrides.push_back({
			VisualModelOverrideTarget::ModelName,
			modelName,
			0,
			model,
			std::chrono::steady_clock::now() + delay,
		});
	}

	void ScriptExtension::RemovePendingVisualModelOverride(const VisualModelOverrideTarget target, const std::string& modelName, const int entNum)
	{
		std::erase_if(PendingVisualModelOverrides, [target, &modelName, entNum](const PendingVisualModelOverride& pending)
		{
			if (pending.target != target)
			{
				return false;
			}

			if (target == VisualModelOverrideTarget::Entity)
			{
				return pending.entNum == entNum;
			}

			return pending.modelName == modelName;
		});
	}

	void ScriptExtension::UpdatePendingVisualModelOverrides()
	{
		const auto now = std::chrono::steady_clock::now();
		for (auto i = PendingVisualModelOverrides.begin(); i != PendingVisualModelOverrides.end();)
		{
			if (now < i->applyTime)
			{
				++i;
				continue;
			}

			if (i->target == VisualModelOverrideTarget::Entity)
			{
				if (i->model)
				{
					EntityVisualModelOverrides[i->entNum] = i->model;
				}
			}
			else if (i->model)
			{
				GlobalVisualModelOverrides[i->modelName] = i->model;
			}

			i = PendingVisualModelOverrides.erase(i);
		}
	}

	void ScriptExtension::ClearPendingVisualModelOverrides()
	{
		PendingVisualModelOverrides.clear();
	}

	Game::XModel* ScriptExtension::GetGlobalVisualModelOverride(const char* modelName)
	{
		if (!modelName)
		{
			return nullptr;
		}

		if (const auto globalOverride = GlobalVisualModelOverrides.find(modelName); globalOverride != GlobalVisualModelOverrides.end())
		{
			return globalOverride->second;
		}

		constexpr std::string_view globalClonePrefix = "resizeModels_";
		const auto currentName = std::string_view(modelName);
		if (currentName.starts_with(globalClonePrefix))
		{
			const auto sourceName = std::string(currentName.substr(globalClonePrefix.size()));
			if (const auto globalOverride = GlobalVisualModelOverrides.find(sourceName); globalOverride != GlobalVisualModelOverrides.end())
			{
				return globalOverride->second;
			}
		}

		for (const auto& [sourceName, clone] : GlobalVisualModelClones)
		{
			if (!clone || !clone->name || std::strcmp(modelName, clone->name) != 0)
			{
				continue;
			}

			if (const auto globalOverride = GlobalVisualModelOverrides.find(sourceName); globalOverride != GlobalVisualModelOverrides.end())
			{
				return globalOverride->second;
			}
		}

		return nullptr;
	}

	float ScriptExtension::GetModelSceneScale(const std::string& modelName)
	{
		if (const auto scale = ModelSceneScales.find(modelName); scale != ModelSceneScales.end())
		{
			return scale->second;
		}

		return 1.0f;
	}

	float ScriptExtension::GetEntitySceneScale(const int entNum)
	{
		if (const auto scale = EntitySceneScales.find(entNum); scale != EntitySceneScales.end())
		{
			return scale->second;
		}

		return 1.0f;
	}

	float ScriptExtension::GetEntitySceneScaleForModelAt(const char* modelName, const float* origin)
	{
		if (!modelName || !origin)
		{
			return 1.0f;
		}

		auto bestScale = 1.0f;
		auto bestDistance = 256.0f * 256.0f;

		for (const auto& [entNum, scale] : EntitySceneScales)
		{
			if (scale == 1.0f)
			{
				continue;
			}

			const auto sourceModel = EntityModelCloneSources.find(entNum);
			const auto storedOrigin = EntityModelCloneOrigins.find(entNum);
			if (sourceModel == EntityModelCloneSources.end() || storedOrigin == EntityModelCloneOrigins.end())
			{
				continue;
			}

			if (sourceModel->second != modelName)
			{
				continue;
			}

			const auto dx = origin[0] - storedOrigin->second[0];
			const auto dy = origin[1] - storedOrigin->second[1];
			const auto dz = origin[2] - storedOrigin->second[2];
			const auto distance = dx * dx + dy * dy + dz * dz;
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestScale = scale;
			}
		}

		return bestScale;
	}

	void ScriptExtension::RemoveSceneScaleTransition(const SceneScaleTarget target, const std::string& modelName, const int entNum)
	{
		std::erase_if(SceneScaleTransitions, [target, &modelName, entNum](const SceneScaleTransition& transition)
		{
			if (transition.target != target)
			{
				return false;
			}

			if (target == SceneScaleTarget::Entity)
			{
				return transition.entNum == entNum;
			}

			return transition.modelName == modelName;
		});
	}

	void ScriptExtension::ResizeModelSceneScale(const std::string& modelName, const float targetScale, const float time)
	{
		RemoveSceneScaleTransition(SceneScaleTarget::ModelName, modelName, 0);

		if (time <= 0.0f)
		{
			ModelSceneScales[modelName] = targetScale;
			return;
		}

		const auto duration = std::chrono::milliseconds(std::max(1, static_cast<int>(std::min(time * 1000.0f, static_cast<float>(std::numeric_limits<int>::max())))));

		SceneScaleTransitions.push_back({
			SceneScaleTarget::ModelName,
			modelName,
			0,
			GetModelSceneScale(modelName),
			targetScale,
			std::chrono::steady_clock::now(),
			duration,
		});
	}

	void ScriptExtension::ResizeEntitySceneScale(const int entNum, const float targetScale, const float time)
	{
		RemoveSceneScaleTransition(SceneScaleTarget::Entity, {}, entNum);

		if (time <= 0.0f)
		{
			EntitySceneScales[entNum] = targetScale;
			return;
		}

		const auto duration = std::chrono::milliseconds(std::max(1, static_cast<int>(std::min(time * 1000.0f, static_cast<float>(std::numeric_limits<int>::max())))));

		SceneScaleTransitions.push_back({
			SceneScaleTarget::Entity,
			{},
			entNum,
			GetEntitySceneScale(entNum),
			targetScale,
			std::chrono::steady_clock::now(),
			duration,
		});
	}

	void ScriptExtension::UpdateSceneScaleTransitions()
	{
		const auto now = std::chrono::steady_clock::now();

		for (auto i = SceneScaleTransitions.begin(); i != SceneScaleTransitions.end();)
		{
			const auto elapsed = now - i->startTime;
			const auto done = elapsed >= i->duration;
			const auto fraction = done ? 1.0f : std::clamp(static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()) / static_cast<float>(i->duration.count()), 0.0f, 1.0f);
			const auto scale = Lerp(i->startScale, i->targetScale, fraction);

			if (i->target == SceneScaleTarget::Entity)
			{
				EntitySceneScales[i->entNum] = scale;
			}
			else
			{
				ModelSceneScales[i->modelName] = scale;
			}

			if (done)
			{
				i = SceneScaleTransitions.erase(i);
				continue;
			}

			++i;
		}
	}

	void ScriptExtension::ApplyDObjModelOverride(Game::DObj* obj, const unsigned int entNum)
	{
		if (!obj || !obj->models)
		{
			return;
		}

		auto entityOverride = EntityVisualModelOverrides.find(entNum);
		auto sourceModel = EntityModelCloneSources.find(entNum);
		if (entityOverride == EntityVisualModelOverrides.end())
		{
			entityOverride = EntityVisualModelOverrides.find(obj->entnum);
			sourceModel = EntityModelCloneSources.find(obj->entnum);
		}

		auto* entityModel = entityOverride == EntityVisualModelOverrides.end() ? nullptr : entityOverride->second;

		for (auto modelIndex = 0; modelIndex < obj->numModels; ++modelIndex)
		{
			auto*& model = obj->models[modelIndex];
			if (!model)
			{
				continue;
			}

			if (entityModel && model == entityModel)
			{
				continue;
			}

			if (entityModel && sourceModel != EntityModelCloneSources.end() && model->name && sourceModel->second == model->name)
			{
				model = entityModel;
				continue;
			}

			if (model->name)
			{
				if (auto* globalOverride = GetGlobalVisualModelOverride(model->name))
				{
					model = globalOverride;
				}
			}
		}

		if (entityModel && obj->numModels == 1)
		{
			obj->models[0] = entityModel;
		}

		if (obj->numModels > 0 && obj->models[0])
		{
			obj->radius = obj->models[0]->radius;
		}
	}

	float ScriptExtension::GetDObjVisualModelScale(Game::DObj* obj)
	{
		if (!obj || !obj->models)
		{
			return 1.0f;
		}

		for (auto modelIndex = 0; modelIndex < obj->numModels; ++modelIndex)
		{
			auto* model = obj->models[modelIndex];
			if (!model)
			{
				continue;
			}

			if (const auto visualModel = VisualScaledModels.find(model); visualModel != VisualScaledModels.end() && visualModel->second.initialized)
			{
				return visualModel->second.scale;
			}
		}

		return 1.0f;
	}

	Game::DObjAnimMat* ScriptExtension::GetScaledDObjAnimMats(Game::DObj* obj, Game::DObjAnimMat* mat, const float scale)
	{
		if (!obj || !mat || scale == 1.0f || scale <= 0.0f || !std::isfinite(scale))
		{
			return mat;
		}

		const auto boneCount = static_cast<unsigned char>(obj->numBones);
		if (!boneCount)
		{
			return mat;
		}

		auto& scaledMats = ScaledDObjAnimMats[obj];
		scaledMats.resize(boneCount);
		std::memcpy(scaledMats.data(), mat, sizeof(Game::DObjAnimMat) * boneCount);

		for (auto boneIndex = 0u; boneIndex < boneCount; ++boneIndex)
		{
			auto& boneMat = scaledMats[boneIndex];
			boneMat.trans[0] *= scale;
			boneMat.trans[1] *= scale;
			boneMat.trans[2] *= scale;
		}

		return scaledMats.data();
	}

	void ScriptExtension::StoreSkinnedCacheScale(Game::GfxSceneEntity* sceneDObj, Game::DObj* obj)
	{
		if (!sceneDObj)
		{
			return;
		}

		auto* firstSurf = sceneDObj->cull.skinnedSurfs.firstSurf;
		if (!firstSurf)
		{
			return;
		}

		const auto scale = GetDObjVisualModelScale(obj);
		std::lock_guard lock(SkinnedCacheScalesMutex);

		if (scale == 1.0f || scale <= 0.0f || !std::isfinite(scale))
		{
			SkinnedCacheScales.erase(firstSurf);
			return;
		}

		SkinnedCacheScales[firstSurf] = {
			scale,
			{0.0f, 0.0f, 0.0f},
		};
	}

	void ScriptExtension::ScaleSkinnedMatrix(Game::DObjSkelMat& matrix, const float scale, const std::array<float, 3>& origin)
	{
		for (auto axis = 0; axis < 3; ++axis)
		{
			for (auto component = 0; component < 3; ++component)
			{
				matrix.axis[axis][component] *= scale;
			}
		}

		for (auto component = 0; component < 3; ++component)
		{
			matrix.origin[component] = origin[component] + ((matrix.origin[component] - origin[component]) * scale);
		}
	}

	void ScriptExtension::ScaleSkinnedMatrixOrigin(Game::DObjSkelMat& matrix, const float scale, const std::array<float, 3>& origin)
	{
		for (auto component = 0; component < 3; ++component)
		{
			matrix.origin[component] = origin[component] + ((matrix.origin[component] - origin[component]) * scale);
		}
	}

	void ScriptExtension::ApplyDObjVisualScale(Game::GfxSceneEntity& sceneDObj, const float scale, const bool scaleSceneBounds)
	{
		auto* obj = sceneDObj.obj;
		if (!obj || scale <= 0.0f || !std::isfinite(scale))
		{
			return;
		}

		if (scaleSceneBounds)
		{
			obj->radius *= scale;
			sceneDObj.cull.bounds.halfSize[0] *= scale;
			sceneDObj.cull.bounds.halfSize[1] *= scale;
			sceneDObj.cull.bounds.halfSize[2] *= scale;
		}
	}

	void ScriptExtension::ApplySceneScales()
	{
		if (!Game::CL_IsCgameInitialized() || !Game::scene)
		{
			return;
		}

		UpdateSceneScaleTransitions();

		if (LastSceneScaleApplyTime == Game::scene->def.time)
		{
			return;
		}

		LastSceneScaleApplyTime = Game::scene->def.time;

		for (auto i = 0; i < Game::scene->sceneModelCount; ++i)
		{
			auto& sceneModel = Game::scene->sceneModel[i];

			if (sceneModel.model && sceneModel.model->name)
			{
				if (auto* globalOverride = GetGlobalVisualModelOverride(sceneModel.model->name))
				{
					sceneModel.model = globalOverride;
					sceneModel.radius = globalOverride->radius * sceneModel.placement.scale;
				}
			}

			auto scale = GetEntitySceneScale(sceneModel.entnum);
			if (scale == 1.0f && sceneModel.model && sceneModel.model->name)
			{
				scale = GetEntitySceneScaleForModelAt(sceneModel.model->name, sceneModel.placement.base.origin);
			}

			if (scale == 1.0f && sceneModel.model && sceneModel.model->name)
			{
				scale = GetModelSceneScale(sceneModel.model->name);
			}

			if (scale != 1.0f)
			{
				sceneModel.placement.scale *= scale;
			}
		}

		for (auto i = 0; i < Game::scene->sceneDObjCount; ++i)
		{
			auto& sceneDObj = Game::scene->sceneDObj[i];
			if (!sceneDObj.obj || sceneDObj.obj->numModels <= 0 || !sceneDObj.obj->models)
			{
				continue;
			}

			auto visualModelScale = GetDObjVisualModelScale(sceneDObj.obj);
			auto sceneScale = GetEntitySceneScale(sceneDObj.entnum);
			auto* model = sceneDObj.obj->models[0];

			if (sceneScale == 1.0f)
			{
				for (auto modelIndex = 0; modelIndex < sceneDObj.obj->numModels; ++modelIndex)
				{
					auto* candidate = sceneDObj.obj->models[modelIndex];
					if (!candidate || !candidate->name)
					{
						continue;
					}

					sceneScale = GetEntitySceneScaleForModelAt(candidate->name, sceneDObj.placement.origin);
					if (sceneScale != 1.0f)
					{
						model = candidate;
						break;
					}
				}
			}

			if (sceneScale == 1.0f)
			{
				for (auto modelIndex = 0; modelIndex < sceneDObj.obj->numModels; ++modelIndex)
				{
					auto* candidate = sceneDObj.obj->models[modelIndex];
					if (!candidate || !candidate->name)
					{
						continue;
					}

					sceneScale = GetModelSceneScale(candidate->name);
					if (sceneScale != 1.0f)
					{
						model = candidate;
						break;
					}
				}
			}

			const auto scale = visualModelScale * sceneScale;
			if (scale == 1.0f || !model)
			{
				continue;
			}

			ApplyDObjVisualScale(sceneDObj, scale, sceneScale != 1.0f);
		}
	}

	__declspec(naked) void ScriptExtension::R_AddDObjToScene_Stub()
	{
		__asm
		{
			pushad

			mov eax, [esp + 24h]
			mov ecx, [esp + 2Ch]
			push ecx
			push eax
			call ApplyDObjModelOverride
			add esp, 8h

			popad

			fldz
			sub esp, 14h
			push 50B705h
			ret
		}
	}

	int ScriptExtension::R_UpdateSkinnedCachedSurfs_Hk(Game::GfxSceneEntity* sceneDObj, Game::DObj* obj, Game::DObjAnimMat* mat)
	{
		const auto scale = GetDObjVisualModelScale(obj);
		auto* renderMat = GetScaledDObjAnimMats(obj, mat, scale);
		return Utils::Hook::Call<int(Game::GfxSceneEntity*, Game::DObj*, Game::DObjAnimMat*)>(0x549A60)(sceneDObj, obj, renderMat);
	}

	void ScriptExtension::R_ProcessSkinnedCacheCmd_Hk(void* command)
	{
		const auto previousScale = CurrentSkinnedCacheScale;
		const auto previousHasScale = HasCurrentSkinnedCacheScale;

		HasCurrentSkinnedCacheScale = false;

		if (command)
		{
			auto* firstSurf = *static_cast<void**>(command);
			std::lock_guard lock(SkinnedCacheScalesMutex);

			if (const auto cacheScale = SkinnedCacheScales.find(firstSurf); cacheScale != SkinnedCacheScales.end())
			{
				CurrentSkinnedCacheScale = cacheScale->second;
				HasCurrentSkinnedCacheScale = true;
			}
		}

		Utils::Hook::Call<void(void*)>(0x54EA60)(command);

		CurrentSkinnedCacheScale = previousScale;
		HasCurrentSkinnedCacheScale = previousHasScale;
	}

	void ScriptExtension::R_SkinRigidXSurface_Original(void* surfData, Game::XSurface* surface, Game::DObjSkelMat* matrices)
	{
		__asm
		{
			mov eax, surfData
			push matrices
			push surface
			mov ecx, 54E320h
			call ecx
			add esp, 8h
		}
	}

	void ScriptExtension::R_SkinRigidXSurface_Hk(void* surfData, Game::XSurface* surface, Game::DObjSkelMat* matrices)
	{
		if (!HasCurrentSkinnedCacheScale || !surface || !surface->vertList || !matrices || CurrentSkinnedCacheScale.scale == 1.0f)
		{
			R_SkinRigidXSurface_Original(surfData, surface, matrices);
			return;
		}

		std::array<bool, 256> usedMatrices{};
		for (auto vertListIndex = 0u; vertListIndex < surface->vertListCount; ++vertListIndex)
		{
			const auto matrixIndex = static_cast<unsigned int>(surface->vertList[vertListIndex].boneOffset / sizeof(Game::DObjSkelMat));
			if (matrixIndex < usedMatrices.size())
			{
				usedMatrices[matrixIndex] = true;
			}
		}

		std::vector<std::pair<unsigned int, Game::DObjSkelMat>> backups;
		backups.reserve(surface->vertListCount);

		for (auto matrixIndex = 0u; matrixIndex < usedMatrices.size(); ++matrixIndex)
		{
			if (!usedMatrices[matrixIndex])
			{
				continue;
			}

			backups.emplace_back(matrixIndex, matrices[matrixIndex]);
			ScaleSkinnedMatrixOrigin(matrices[matrixIndex], CurrentSkinnedCacheScale.scale, CurrentSkinnedCacheScale.origin);
		}

		R_SkinRigidXSurface_Original(surfData, surface, matrices);

		for (const auto& [matrixIndex, backup] : backups)
		{
			matrices[matrixIndex] = backup;
		}
	}

	__declspec(naked) void ScriptExtension::R_SkinRigidXSurface_Stub()
	{
		__asm
		{
			pushad

			mov eax, [esp + 28h]
			mov ecx, [esp + 24h]
			mov edx, [esp + 1Ch]
			push eax
			push ecx
			push edx
			call R_SkinRigidXSurface_Hk
			add esp, 0Ch

			popad
			ret
		}
	}

	void ScriptExtension::R_SkinXSurface_Hk(Game::GfxPackedVertex* verts0, Game::XSurfaceVertexInfo* vertInfo, Game::DObjSkelMat* matrices, void* surfData)
	{
		if (!HasCurrentSkinnedCacheScale || !vertInfo || !vertInfo->vertsBlend || !matrices || CurrentSkinnedCacheScale.scale == 1.0f)
		{
			Utils::Hook::Call<void(Game::GfxPackedVertex*, Game::XSurfaceVertexInfo*, Game::DObjSkelMat*, void*)>(0x54DFA0)(verts0, vertInfo, matrices, surfData);
			return;
		}

		std::array<bool, 256> usedMatrices{};
		auto vertsBlendOffset = 0u;

		const auto markMatrix = [&usedMatrices, vertInfo](const unsigned int offset)
		{
			const auto matrixIndex = static_cast<unsigned int>(vertInfo->vertsBlend[offset] / sizeof(Game::DObjSkelMat));
			if (matrixIndex < usedMatrices.size())
			{
				usedMatrices[matrixIndex] = true;
			}
		};

		for (auto vertIndex = 0u; vertIndex < vertInfo->vertCount[0]; ++vertIndex)
		{
			markMatrix(vertsBlendOffset + 0);
			vertsBlendOffset += 1;
		}

		for (auto vertIndex = 0u; vertIndex < vertInfo->vertCount[1]; ++vertIndex)
		{
			markMatrix(vertsBlendOffset + 0);
			markMatrix(vertsBlendOffset + 1);
			vertsBlendOffset += 3;
		}

		for (auto vertIndex = 0u; vertIndex < vertInfo->vertCount[2]; ++vertIndex)
		{
			markMatrix(vertsBlendOffset + 0);
			markMatrix(vertsBlendOffset + 1);
			markMatrix(vertsBlendOffset + 3);
			vertsBlendOffset += 5;
		}

		for (auto vertIndex = 0u; vertIndex < vertInfo->vertCount[3]; ++vertIndex)
		{
			markMatrix(vertsBlendOffset + 0);
			markMatrix(vertsBlendOffset + 1);
			markMatrix(vertsBlendOffset + 3);
			markMatrix(vertsBlendOffset + 5);
			vertsBlendOffset += 7;
		}

		std::vector<std::pair<unsigned int, Game::DObjSkelMat>> backups;
		backups.reserve(32);

		for (auto matrixIndex = 0u; matrixIndex < usedMatrices.size(); ++matrixIndex)
		{
			if (!usedMatrices[matrixIndex])
			{
				continue;
			}

			backups.emplace_back(matrixIndex, matrices[matrixIndex]);
			ScaleSkinnedMatrixOrigin(matrices[matrixIndex], CurrentSkinnedCacheScale.scale, CurrentSkinnedCacheScale.origin);
		}

		Utils::Hook::Call<void(Game::GfxPackedVertex*, Game::XSurfaceVertexInfo*, Game::DObjSkelMat*, void*)>(0x54DFA0)(verts0, vertInfo, matrices, surfData);

		for (const auto& [matrixIndex, backup] : backups)
		{
			matrices[matrixIndex] = backup;
		}
	}

	void ScriptExtension::R_GenerateSortedDrawSurfs_Hk(void* viewInfo)
	{
		ApplySceneScales();
		Utils::Hook::Call<void(void*)>(0x53BEB0)(viewInfo);
	}

	void ScriptExtension::R_AddSceneSurfaces_Hk(const int viewIndex)
	{
		ApplySceneScales();
		Utils::Hook::Call<void(int)>(0x514A60)(viewIndex);
	}

	void ScriptExtension::ClearSceneScales()
	{
		SceneScaleTransitions.clear();
		ModelSceneScales.clear();
		EntitySceneScales.clear();
		ScaledDObjAnimMats.clear();
		{
			std::lock_guard lock(SkinnedCacheScalesMutex);
			SkinnedCacheScales.clear();
		}
		LastSceneScaleApplyTime = std::numeric_limits<int>::min();
	}

	Game::gentity_s* ScriptExtension::Scr_GetEntity(const Game::scr_entref_t entref)
	{
		if (entref.classnum)
		{
			Game::Scr_ObjectError("not an entity");
			return nullptr;
		}

		assert(entref.entnum < Game::MAX_GENTITIES);
		return &Game::g_entities[entref.entnum];
	}

	Game::XModel* ScriptExtension::GetModelByIndex(const int modelIndex)
	{
		if (modelIndex <= 0)
		{
			return nullptr;
		}

		if (ModelCache::modelsHaveBeenReallocated)
		{
			if (modelIndex >= ModelCache::G_MODELINDEX_LIMIT)
			{
				return nullptr;
			}

			return ModelCache::cached_models_reallocated[modelIndex];
		}

		if (modelIndex >= Game::MAX_MODELS)
		{
			return nullptr;
		}

		return Game::G_GetModel(modelIndex);
	}

	int ScriptExtension::FindModelIndexByName(const std::string& modelName)
	{
		if (modelName.empty())
		{
			return 0;
		}

		const auto limit = ModelCache::modelsHaveBeenReallocated ? ModelCache::G_MODELINDEX_LIMIT : Game::MAX_MODELS;
		for (auto modelIndex = 1; modelIndex < limit; ++modelIndex)
		{
			auto* model = GetModelByIndex(modelIndex);
			if (model && model->name && modelName == model->name)
			{
				return modelIndex;
			}
		}

		return 0;
	}

	Game::XModel* ScriptExtension::CloneModel(Game::XModel* sourceModel, const char* cloneName)
	{
		if (!sourceModel || !cloneName)
		{
			return nullptr;
		}

		auto* clone = Utils::Memory::GetAllocator()->allocate<Game::XModel>();
		std::memcpy(clone, sourceModel, sizeof(Game::XModel));
		clone->name = Utils::Memory::GetAllocator()->duplicateString(cloneName);

		if (const auto visualModel = VisualScaledModels.find(sourceModel); visualModel != VisualScaledModels.end() && visualModel->second.initialized)
		{
			RestoreXModelVisualScale(clone, visualModel->second);
		}

		return clone;
	}

	Game::XModel* ScriptExtension::GetOrCreateGlobalModelClone(const std::string& modelName, Game::XModel* sourceModel)
	{
		if (!sourceModel)
		{
			return nullptr;
		}

		if (const auto clone = GlobalVisualModelClones.find(modelName); clone != GlobalVisualModelClones.end())
		{
			return clone->second;
		}

		auto* clone = CloneModel(sourceModel, Utils::String::VA("resizeModels_%s", sourceModel->name));
		if (clone)
		{
			GlobalVisualModelClones[modelName] = clone;
		}

		return clone;
	}

	int ScriptExtension::AllocateEntityModelCloneIndex()
	{
		if (!ModelCache::modelsHaveBeenReallocated)
		{
			return 0;
		}

		const auto firstPrivateModel = ModelCache::BASE_GMODEL_COUNT + ModelCache::ADDITIONAL_GMODELS + 1;
		for (auto i = ModelCache::G_MODELINDEX_LIMIT - 1; i >= firstPrivateModel; --i)
		{
			if (!ModelCache::cached_models_reallocated[i] && !ModelCache::gameModels_reallocated[i])
			{
				return i;
			}
		}

		return 0;
	}

	Game::XModel* ScriptExtension::CloneModelForIndex(Game::XModel* sourceModel, const int modelIndex, const char* cloneName)
	{
		if (!sourceModel || modelIndex <= 0 || !ModelCache::modelsHaveBeenReallocated || modelIndex >= ModelCache::G_MODELINDEX_LIMIT)
		{
			return nullptr;
		}

		auto* clone = CloneModel(sourceModel, cloneName);
		if (!clone)
		{
			return nullptr;
		}

		ModelCache::cached_models_reallocated[modelIndex] = clone;
		ModelCache::gameModels_reallocated[modelIndex] = clone;
		return clone;
	}

	Game::XModel* ScriptExtension::GetOrCreateEntityModelClone(Game::gentity_s* ent, const bool activateOverride)
	{
		if (!ent)
		{
			return nullptr;
		}

		const auto entNum = ent->s.number;
		if (const auto cloneIndex = EntityModelCloneIndexes.find(entNum); cloneIndex != EntityModelCloneIndexes.end())
		{
			if (auto* clone = GetModelByIndex(cloneIndex->second))
			{
				if (activateOverride)
				{
					EntityVisualModelOverrides[entNum] = clone;
				}

				return clone;
			}

			RemoveModelScaleTransition(GetModelByIndex(cloneIndex->second));
			ModelCache::cached_models_reallocated[cloneIndex->second] = nullptr;
			ModelCache::gameModels_reallocated[cloneIndex->second] = nullptr;
			EntityModelCloneIndexes.erase(cloneIndex);
			EntityModelCloneSources.erase(entNum);
			EntityModelCloneOrigins.erase(entNum);
			EntityVisualModelOverrides.erase(entNum);
		}

		auto* sourceModel = GetModelByIndex(ent->model);
		if (!sourceModel)
		{
			Game::Scr_ObjectError("ResizeModel: entity has no xmodel");
			return nullptr;
		}

		const auto cloneIndex = AllocateEntityModelCloneIndex();
		if (!cloneIndex)
		{
			Game::Scr_Error("ResizeModel: no free model index for entity clone");
			return nullptr;
		}

		const auto sourceName = std::string(sourceModel->name);
		auto* clone = CloneModelForIndex(sourceModel, cloneIndex, Utils::String::VA("resizeModel_%i_%s", entNum, sourceModel->name));
		if (!clone)
		{
			Game::Scr_Error("ResizeModel: failed to clone xmodel");
			return nullptr;
		}

		EntityModelCloneIndexes[entNum] = cloneIndex;
		EntityModelCloneSources[entNum] = sourceName;
		if (activateOverride)
		{
			EntityVisualModelOverrides[entNum] = clone;
		}

		return clone;
	}

	Game::XModel* ScriptExtension::GetOrCreateClientModelClone(const int entNum, const int modelIndex, const char* sourceModelName, const bool activateOverride)
	{
		if (modelIndex <= 0 || modelIndex >= ModelCache::G_MODELINDEX_LIMIT)
		{
			return nullptr;
		}

		if (const auto cloneIndex = EntityModelCloneIndexes.find(entNum); cloneIndex != EntityModelCloneIndexes.end())
		{
			if (cloneIndex->second == modelIndex)
			{
				auto* clone = GetModelByIndex(cloneIndex->second);
				if (clone && activateOverride)
				{
					EntityVisualModelOverrides[entNum] = clone;
				}

				return clone;
			}

			RemoveModelScaleTransition(GetModelByIndex(cloneIndex->second));
			ModelCache::cached_models_reallocated[cloneIndex->second] = nullptr;
			ModelCache::gameModels_reallocated[cloneIndex->second] = nullptr;
			EntityModelCloneIndexes.erase(cloneIndex);
			EntityModelCloneSources.erase(entNum);
			EntityModelCloneOrigins.erase(entNum);
			EntityVisualModelOverrides.erase(entNum);
		}

		auto* sourceModel = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, sourceModelName).model;
		if (!sourceModel || Game::DB_IsXAssetDefault(Game::ASSET_TYPE_XMODEL, sourceModelName))
		{
			return nullptr;
		}

		auto* clone = CloneModelForIndex(sourceModel, modelIndex, Utils::String::VA("resizeModel_%i_%s", entNum, sourceModelName));
		if (clone)
		{
			EntityModelCloneIndexes[entNum] = modelIndex;
			EntityModelCloneSources[entNum] = sourceModelName;
			if (activateOverride)
			{
				EntityVisualModelOverrides[entNum] = clone;
			}
		}

		return clone;
	}

	void ScriptExtension::ClearEntityModelClone(const int entNum, const bool restoreServerEntity)
	{
		RemovePendingModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum);
		const auto cloneIndex = EntityModelCloneIndexes.find(entNum);
		const auto sourceModel = EntityModelCloneSources.find(entNum);

		if (restoreServerEntity && entNum >= 0 && entNum < static_cast<int>(Game::MAX_GENTITIES) && sourceModel != EntityModelCloneSources.end())
		{
			const auto sourceModelIndex = FindModelIndexByName(sourceModel->second);
			if (sourceModelIndex > 0)
			{
				auto* ent = &Game::g_entities[entNum];
				ent->model = static_cast<unsigned __int16>(sourceModelIndex);
				ent->s.index.xmodel = sourceModelIndex;
			}
		}

		if (cloneIndex != EntityModelCloneIndexes.end() && cloneIndex->second > 0 && cloneIndex->second < ModelCache::G_MODELINDEX_LIMIT)
		{
			if (auto* clone = GetModelByIndex(cloneIndex->second))
			{
				ResetXModelVisualScale(clone);
			}

			ModelCache::cached_models_reallocated[cloneIndex->second] = nullptr;
			ModelCache::gameModels_reallocated[cloneIndex->second] = nullptr;
		}

		EntityModelCloneIndexes.erase(entNum);
		EntityModelCloneSources.erase(entNum);
		EntityModelCloneOrigins.erase(entNum);
		EntityVisualModelOverrides.erase(entNum);
		EntitySceneScales.erase(entNum);
		RemovePendingVisualModelOverride(VisualModelOverrideTarget::Entity, {}, entNum);
		RemoveSceneScaleTransition(SceneScaleTarget::Entity, {}, entNum);
	}

	void ScriptExtension::ClearEntityModelClones()
	{
		for (const auto& [entNum, modelIndex] : EntityModelCloneIndexes)
		{
			static_cast<void>(entNum);

			if (modelIndex > 0 && modelIndex < ModelCache::G_MODELINDEX_LIMIT)
			{
				if (auto* clone = GetModelByIndex(modelIndex))
				{
					RemoveModelScaleTransition(clone);
					VisualScaledModels.erase(clone);
				}

				ModelCache::cached_models_reallocated[modelIndex] = nullptr;
				ModelCache::gameModels_reallocated[modelIndex] = nullptr;
			}

			RemovePendingVisualModelOverride(VisualModelOverrideTarget::Entity, {}, entNum);
		}

		EntityModelCloneIndexes.clear();
		EntityModelCloneSources.clear();
		EntityModelCloneOrigins.clear();
		EntityVisualModelOverrides.clear();
		PendingVisualModelOverrides.clear();
	}

	void ScriptExtension::GScr_ResizeModels()
	{
		const auto numParams = Game::Scr_GetNumParam();
		if (numParams < 2 || numParams > 3)
		{
			Game::Scr_Error("ResizeModels: Usage resizeModels(<xmodel>, <factor>, [time])");
			return;
		}

		const auto* modelName = Game::Scr_GetString(0);
		auto* model = GetResizeModel(modelName, 0);
		if (!model)
		{
			return;
		}

		const auto targetScale = Game::Scr_GetFloat(1);
		if (!IsValidScale(targetScale))
		{
			Game::Scr_ParamError(1, "ResizeModels: factor must be between 0.01 and 64.0");
			return;
		}

		auto time = 0.0f;
		if (numParams == 3)
		{
			time = Game::Scr_GetFloat(2);
			if (!std::isfinite(time))
			{
				Game::Scr_ParamError(2, "ResizeModels: time must be finite");
				return;
			}
		}

		const auto delay = PauseScriptModelAnimationsForResize(modelName, time);
		ResizeXModelsByName(modelName, model, targetScale, time, delay);
		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_RELIABLE, Utils::String::Format("{:c} resizeModels \"{}\" {} {} {}", ResizeServerCommand, modelName, targetScale, time, delay.count()));
	}

	void ScriptExtension::GScr_ResetModels()
	{
		const auto numParams = Game::Scr_GetNumParam();
		if (numParams < 1 || numParams > 2)
		{
			Game::Scr_Error("ResetModels: Usage resetModels(<xmodel>, [time])");
			return;
		}

		const auto* modelName = Game::Scr_GetString(0);
		auto* model = GetResizeModel(modelName, 0);
		if (!model)
		{
			return;
		}

		auto time = 0.0f;
		if (numParams == 2)
		{
			time = Game::Scr_GetFloat(1);
			if (!std::isfinite(time))
			{
				Game::Scr_ParamError(1, "ResetModels: time must be finite");
				return;
			}
		}

		const auto delay = time > 0.0f ? PauseScriptModelAnimationsForResize(modelName, time) : std::chrono::milliseconds(0);
		ResetXModelsByName(modelName, model, time, delay);
		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_RELIABLE, Utils::String::Format("{:c} resetModels \"{}\" {} {}", ResizeServerCommand, modelName, time, delay.count()));
	}

	void ScriptExtension::ScrCmd_ResizeModel(const Game::scr_entref_t entref)
	{
		const auto numParams = Game::Scr_GetNumParam();
		if (numParams < 1 || numParams > 2)
		{
			Game::Scr_Error("ResizeModel: Usage <entity> resizeModel(<factor>, [time])");
			return;
		}

		auto* ent = Scr_GetEntity(entref);
		if (!ent)
		{
			return;
		}

		const auto targetScale = Game::Scr_GetFloat(0);
		if (!IsValidScale(targetScale))
		{
			Game::Scr_ParamError(0, "ResizeModel: factor must be between 0.01 and 64.0");
			return;
		}

		auto time = 0.0f;
		if (numParams == 2)
		{
			time = Game::Scr_GetFloat(1);
			if (!std::isfinite(time))
			{
				Game::Scr_ParamError(1, "ResizeModel: time must be finite");
				return;
			}
		}

		const auto entNum = ent->s.number;
		std::string sourceModelName;
		if (const auto source = EntityModelCloneSources.find(entNum); source != EntityModelCloneSources.end())
		{
			sourceModelName = source->second;
		}
		else if (auto* model = GetModelByIndex(ent->model); model && model->name)
		{
			sourceModelName = model->name;
		}

		if (sourceModelName.empty())
		{
			Game::Scr_ObjectError("ResizeModel: entity has no xmodel");
			return;
		}

		EntitySceneScales.erase(entNum);
		RemoveSceneScaleTransition(SceneScaleTarget::Entity, {}, entNum);
		RemovePendingModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum);

		const auto delay = PauseScriptModelAnimationForResize(ent, time);
		auto* clone = GetOrCreateEntityModelClone(ent, delay.count() <= 0);
		if (!clone)
		{
			return;
		}

		ScheduleEntityVisualModelOverride(entNum, clone, delay);
		ResizeXModelDelayed(clone, targetScale, time, delay);
		const auto cloneIndex = EntityModelCloneIndexes[entNum];
		EntityModelCloneOrigins[entNum] = {ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]};
		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_RELIABLE, Utils::String::Format(
			"{:c} resizeModel {} {} \"{}\" {} {} {} {} {} {}",
			ResizeServerCommand,
			entNum,
			cloneIndex,
			sourceModelName,
			ent->r.currentOrigin[0],
			ent->r.currentOrigin[1],
			ent->r.currentOrigin[2],
			targetScale,
			time,
			delay.count()));
	}

	void ScriptExtension::ScrCmd_ResetModel(const Game::scr_entref_t entref)
	{
		const auto numParams = Game::Scr_GetNumParam();
		if (numParams > 1)
		{
			Game::Scr_Error("ResetModel: Usage <entity> resetModel([time])");
			return;
		}

		auto* ent = Scr_GetEntity(entref);
		if (!ent)
		{
			return;
		}

		auto time = 0.0f;
		if (numParams == 1)
		{
			time = Game::Scr_GetFloat(0);
			if (!std::isfinite(time))
			{
				Game::Scr_ParamError(0, "ResetModel: time must be finite");
				return;
			}
		}

		const auto entNum = ent->s.number;
		const auto delay = time > 0.0f ? PauseScriptModelAnimationForResize(ent, time) : std::chrono::milliseconds(0);
		if (time <= 0.0f && delay.count() <= 0)
		{
			ClearEntityModelClone(entNum, true);
		}
		else
		{
			RemovePendingModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum);
			RemovePendingVisualModelOverride(VisualModelOverrideTarget::Entity, {}, entNum);

			if (const auto cloneIndex = EntityModelCloneIndexes.find(entNum); cloneIndex != EntityModelCloneIndexes.end())
			{
				if (auto* clone = GetModelByIndex(cloneIndex->second))
				{
					ResizeXModelDelayed(clone, GetXModelResetScale(clone), time, delay);
					ScheduleModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum, true, time, delay);
				}
			}
		}

		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_RELIABLE, Utils::String::Format("{:c} resetModel {} {} {}", ResizeServerCommand, entNum, time, delay.count()));
	}

	void ScriptExtension::AddResizeFunction()
	{
		Script::AddFunction("resize", GScr_ResizeModels); // gsc: resize(<xmodel>, <factor>, [time])
		Script::AddFunction("resizeModels", GScr_ResizeModels); // gsc: resizeModels(<xmodel>, <factor>, [time])
		Script::AddFunction("resetModels", GScr_ResetModels); // gsc: resetModels(<xmodel>, [time])
		Script::AddMethod("resizeModel", ScrCmd_ResizeModel); // gsc: <entity> resizeModel(<factor>, [time])
		Script::AddMethod("resetModel", ScrCmd_ResetModel); // gsc: <entity> resetModel([time])

		ServerCommands::OnCommand(ResizeServerCommand, [](const Command::Params* params)
		{
			if (params->size() < 2)
			{
				return false;
			}

			if (std::strcmp(params->get(1), "resizeModels") == 0)
			{
				if (params->size() < 4)
				{
					return true;
				}

				auto* model = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, params->get(2)).model;
				if (!model || Game::DB_IsXAssetDefault(Game::ASSET_TYPE_XMODEL, params->get(2)))
				{
					return true;
				}

				const auto targetScale = static_cast<float>(std::atof(params->get(3)));
				const auto time = params->size() >= 5 ? static_cast<float>(std::atof(params->get(4))) : 0.0f;
				const auto delay = params->size() >= 6 ? std::chrono::milliseconds(std::max(0, std::atoi(params->get(5)))) : std::chrono::milliseconds(0);

				if (!IsValidScale(targetScale) || !std::isfinite(time))
				{
					return true;
				}

				ResizeXModelsByName(params->get(2), model, targetScale, time, delay);
				return true;
			}

			if (std::strcmp(params->get(1), "resetModels") == 0)
			{
				if (params->size() < 3)
				{
					return true;
				}

				auto* model = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, params->get(2)).model;
				if (!model || Game::DB_IsXAssetDefault(Game::ASSET_TYPE_XMODEL, params->get(2)))
				{
					return true;
				}

				const auto time = params->size() >= 4 ? static_cast<float>(std::atof(params->get(3))) : 0.0f;
				const auto delay = params->size() >= 5 ? std::chrono::milliseconds(std::max(0, std::atoi(params->get(4)))) : std::chrono::milliseconds(0);
				if (!std::isfinite(time))
				{
					return true;
				}

				ResetXModelsByName(params->get(2), model, time, delay);
				return true;
			}

			if (std::strcmp(params->get(1), "resizeModel") == 0)
			{
				if (params->size() < 10)
				{
					return true;
				}

				const auto entNum = std::atoi(params->get(2));
				if (entNum < 0 || entNum >= static_cast<int>(Game::MAX_GENTITIES))
				{
					return true;
				}

				const auto modelIndex = std::atoi(params->get(3));
				const auto* sourceModelName = params->get(4);
				const std::array<float, 3> origin = {
					static_cast<float>(std::atof(params->get(5))),
					static_cast<float>(std::atof(params->get(6))),
					static_cast<float>(std::atof(params->get(7))),
				};

				const auto targetScale = static_cast<float>(std::atof(params->get(8)));
				const auto time = static_cast<float>(std::atof(params->get(9)));
				const auto delay = params->size() >= 11 ? std::chrono::milliseconds(std::max(0, std::atoi(params->get(10)))) : std::chrono::milliseconds(0);

				if (!IsValidScale(targetScale) || !std::isfinite(time))
				{
					return true;
				}

				EntitySceneScales.erase(entNum);
				RemoveSceneScaleTransition(SceneScaleTarget::Entity, {}, entNum);
				RemovePendingModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum);

				auto* model = GetOrCreateClientModelClone(entNum, modelIndex, sourceModelName, delay.count() <= 0);
				if (!model)
				{
					return true;
				}

				EntityModelCloneOrigins[entNum] = origin;
				ScheduleEntityVisualModelOverride(entNum, model, delay);
				ResizeXModelDelayed(model, targetScale, time, delay);
				return true;
			}

			if (std::strcmp(params->get(1), "resetModel") == 0)
			{
				if (params->size() < 3)
				{
					return true;
				}

				const auto entNum = std::atoi(params->get(2));
				if (entNum < 0 || entNum >= static_cast<int>(Game::MAX_GENTITIES))
				{
					return true;
				}

				const auto time = params->size() >= 4 ? static_cast<float>(std::atof(params->get(3))) : 0.0f;
				const auto delay = params->size() >= 5 ? std::chrono::milliseconds(std::max(0, std::atoi(params->get(4)))) : std::chrono::milliseconds(0);
				if (!std::isfinite(time))
				{
					return true;
				}

				if (time <= 0.0f && delay.count() <= 0)
				{
					ClearEntityModelClone(entNum, false);
					return true;
				}

				RemovePendingModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum);
				RemovePendingVisualModelOverride(VisualModelOverrideTarget::Entity, {}, entNum);

				if (const auto cloneIndex = EntityModelCloneIndexes.find(entNum); cloneIndex != EntityModelCloneIndexes.end())
				{
					if (auto* clone = GetModelByIndex(cloneIndex->second))
					{
						ResizeXModelDelayed(clone, GetXModelResetScale(clone), time, delay);
						ScheduleModelResetCleanup(VisualModelOverrideTarget::Entity, {}, entNum, false, time, delay);
					}
				}

				return true;
			}

			return false;
		});
	}

	void ScriptExtension::AddFunctions()
	{
		Script::AddFunction("IsArray", [] // gsc: IsArray(<object>)
		{
			auto type = Game::Scr_GetType(0);

			bool result;
			if (type == Game::VAR_POINTER)
			{
				type = Game::Scr_GetPointerType(0);
				assert(type >= Game::FIRST_OBJECT);
				result = (type == Game::VAR_ARRAY);
			}
			else
			{
				assert(type < Game::FIRST_OBJECT);
				result = false;
			}

			Game::Scr_AddBool(result);
		});

		Script::AddFunction("ReplaceFunc", [] // gsc: ReplaceFunc(<function>, <function>)
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("ReplaceFunc: Needs two parameters!");
				return;
			}

			const auto what = GetCodePosForParam(0);
			const auto with = GetCodePosForParam(1);

			SetReplacedPos(what, with);
		});


		Script::AddFunction("GetSystemMilliseconds", [] // gsc: GetSystemMilliseconds()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wMilliseconds);
		});

		Script::AddFunction("Exec", [] // gsc: Exec(<string>)
		{
			const auto* str = Game::Scr_GetString(0);
			if (!str)
			{
				Game::Scr_ParamError(0, "Exec: Illegal parameter!");
				return;
			}

			Command::Execute(str, false);
		});

		// Allow printing to the console even when developer is 0
		Script::AddFunction("PrintConsole", [] // gsc: PrintConsole(<string>)
		{
			for (std::size_t i = 0; i < Game::Scr_GetNumParam(); ++i)
			{
				const auto* str = Game::Scr_GetString(i);
				if (!str)
				{
					Game::Scr_ParamError(i, "PrintConsole: Illegal parameter!");
					return;
				}

				Logger::Print(Game::level->scriptPrintChannel, "{}", str);
			}
		});

		AddResizeFunction();
	}

	ScriptExtension::ScriptExtension()
	{
		AddFunctions();

		Utils::Hook(0x61E92E, VMExecuteInternalStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x61E933, 1);

		Events::OnVMShutdown([]
		{
			ReplacedFunctions.clear();
			ClearVisualScaledModels();
			ClearSceneScales();
			ClearEntityModelClones();
			ClearPausedScriptModelAnimations();
			ClearPendingVisualModelOverrides();
		});

		Events::OnCLDisconnected([]([[maybe_unused]] bool wasConnected)
		{
			ClearVisualScaledModels();
			ClearSceneScales();
			ClearEntityModelClones();
			ClearPausedScriptModelAnimations();
			ClearPendingVisualModelOverrides();
		});

		Scheduler::Loop(UpdatePendingVisualModelOverrides, Scheduler::Pipeline::MAIN);
		Scheduler::Loop(UpdateModelScaleTransitions, Scheduler::Pipeline::MAIN);
		Scheduler::Loop(UpdatePausedScriptModelAnimations, Scheduler::Pipeline::MAIN);
		Utils::Hook(0x50B700, R_AddDObjToScene_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x50E6F3, R_GenerateSortedDrawSurfs_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x50E762, R_AddSceneSurfaces_Hk, HOOK_CALL).install()->quick();
	}
}
