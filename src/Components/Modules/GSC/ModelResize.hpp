#pragma once

namespace Components::GSC
{
	class ModelResize : public Component
	{
	public:
		ModelResize();

	private:
		enum class VisualModelOverrideTarget
		{
			ModelName,
			Entity,
		};

		struct ModelScaleTransition
		{
			Game::XModel* model;
			float startScale;
			float targetScale;
			std::chrono::steady_clock::time_point startTime;
			std::chrono::steady_clock::time_point lastUpdateTime;
			std::chrono::milliseconds duration;
		};

		struct PendingModelResetCleanup
		{
			VisualModelOverrideTarget target;
			std::string modelName;
			int entNum;
			bool restoreServerEntity;
			std::chrono::steady_clock::time_point cleanupTime;
		};

		struct PausedScriptModelAnimation
		{
			int entNum;
			int animIndex;
			std::chrono::steady_clock::time_point resumeTime;
		};

		struct VisualScaledModel
		{
			Game::XModelLodInfo originalLodInfo[4];
			Game::XModelSurfs* scaledSurfs[4];
			Game::DObjAnimMat* originalBaseMat;
			float* originalTrans;
			Game::XBoneInfo* originalBoneInfo;
			std::vector<Game::DObjAnimMat> originalBaseMats;
			std::vector<Game::DObjAnimMat> scaledBaseMats;
			std::vector<float> originalTransValues;
			std::vector<float> scaledTransValues;
			std::vector<Game::XBoneInfo> originalBoneInfos;
			std::vector<Game::XBoneInfo> scaledBoneInfos;
			float originalRadius;
			Game::Bounds originalBounds;
			float originalScale;
			float scale;
			bool initialized;
		};

		struct PendingVisualModelOverride
		{
			VisualModelOverrideTarget target;
			std::string modelName;
			int entNum;
			Game::XModel* model;
			std::chrono::steady_clock::time_point applyTime;
		};

		static std::vector<ModelScaleTransition> ModelScaleTransitions;
		static std::vector<PendingModelResetCleanup> PendingModelResetCleanups;
		static std::vector<PausedScriptModelAnimation> PausedScriptModelAnimations;
		static std::unordered_map<Game::XModel*, VisualScaledModel> VisualScaledModels;
		static int LastSceneModelOverrideApplyTime;
		static std::unordered_map<int, int> EntityModelCloneIndexes;
		static std::unordered_map<int, std::string> EntityModelCloneSources;
		static std::unordered_map<int, std::array<float, 3>> EntityModelCloneOrigins;
		static std::unordered_map<int, Game::XModel*> EntityVisualModelOverrides;
		static std::vector<PendingVisualModelOverride> PendingVisualModelOverrides;
		static std::unordered_map<std::string, Game::XModel*> GlobalVisualModelClones;
		static std::unordered_map<std::string, Game::XModel*> GlobalVisualModelOverrides;

		static Game::XModel* GetResizeModel(const char* name, unsigned int paramIndex);
		static void RemoveModelScaleTransition(Game::XModel* model);
		static float GetXModelVisualScale(Game::XModel* model);
		static float GetXModelResetScale(Game::XModel* model);
		static void SetXModelVisualScale(Game::XModel* model, float scale);
		static void SetXModelVisualSkeletonScale(Game::XModel* model, VisualScaledModel& visualModel, float scale);
		static void RestoreXModelVisualScale(Game::XModel* model, const VisualScaledModel& visualModel);
		static void ResetXModelVisualScale(Game::XModel* model);
		static void ResizeXModelDelayed(Game::XModel* model, float targetScale, float time, std::chrono::milliseconds delay);
		static void ResizeXModelsByName(const std::string& modelName, Game::XModel* model, float targetScale, float time, std::chrono::milliseconds delay);
		static void ResetXModelsByName(const std::string& modelName, Game::XModel* model, float time, std::chrono::milliseconds delay);
		static void UpdateModelScaleTransitions();
		static void ScheduleModelResetCleanup(VisualModelOverrideTarget target, const std::string& modelName, int entNum, bool restoreServerEntity, float time, std::chrono::milliseconds delay);
		static void RemovePendingModelResetCleanup(VisualModelOverrideTarget target, const std::string& modelName, int entNum);
		static void UpdatePendingModelResetCleanups();
		static void ClearVisualScaledModels();
		static bool IsScriptModelEntity(const Game::gentity_s* ent);
		static int& ScriptModelAnimIndex(Game::gentity_s* ent);
		static int& ScriptModelAnimStartTime(Game::gentity_s* ent);
		static std::chrono::milliseconds PauseScriptModelAnimationForResize(Game::gentity_s* ent, float time);
		static std::chrono::milliseconds PauseScriptModelAnimationsForResize(const std::string& modelName, float time);
		static void UpdatePausedScriptModelAnimations();
		static void ClearPausedScriptModelAnimations();
		static void ScheduleEntityVisualModelOverride(int entNum, Game::XModel* model, std::chrono::milliseconds delay);
		static void ScheduleGlobalVisualModelOverride(const std::string& modelName, Game::XModel* model, std::chrono::milliseconds delay);
		static void RemovePendingVisualModelOverride(VisualModelOverrideTarget target, const std::string& modelName, int entNum);
		static void UpdatePendingVisualModelOverrides();
		static void ClearPendingVisualModelOverrides();
		static Game::XModel* GetGlobalVisualModelOverride(const char* modelName);
		static void ApplyDObjModelOverride(Game::DObj* obj, unsigned int entNum);
		static void ApplySceneModelOverrides();
		static void R_AddDObjToScene_Stub();
		static void R_GenerateSortedDrawSurfs_Hk(void* viewInfo);
		static void R_AddSceneSurfaces_Hk(int viewIndex);
		static void ClearSceneModelOverrideState();
		static Game::gentity_s* Scr_GetEntity(Game::scr_entref_t entref);
		static Game::XModel* GetModelByIndex(int modelIndex);
		static int FindModelIndexByName(const std::string& modelName);
		static Game::XModel* CloneModel(Game::XModel* sourceModel, const char* cloneName);
		static Game::XModel* GetOrCreateGlobalModelClone(const std::string& modelName, Game::XModel* sourceModel);
		static int AllocateEntityModelCloneIndex();
		static Game::XModel* CloneModelForIndex(Game::XModel* sourceModel, int modelIndex, const char* cloneName);
		static Game::XModel* GetOrCreateEntityModelClone(Game::gentity_s* ent, bool activateOverride);
		static Game::XModel* GetOrCreateClientModelClone(int entNum, int modelIndex, const char* sourceModelName, bool activateOverride);
		static void ClearEntityModelClone(int entNum, bool restoreServerEntity);
		static void ClearEntityModelClones();
		static void GScr_ResizeModels();
		static void GScr_ResetModels();
		static void ScrCmd_ResizeModel(Game::scr_entref_t entref);
		static void ScrCmd_ResetModel(Game::scr_entref_t entref);
		static void AddResizeFunction();
	};
}
