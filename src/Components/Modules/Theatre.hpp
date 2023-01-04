#pragma once

namespace Components
{
	class Theatre : public Component
	{
	public:
		Theatre();

		static void StopRecording();

	private:
		class DemoInfo
		{
		public:
			std::string name;
			std::string mapname;
			std::string gametype;
			std::string author;
			int length;
			std::time_t timeStamp;

			[[nodiscard]] nlohmann::json to_json() const;
		};

		static DemoInfo CurrentInfo;
		static unsigned int CurrentSelection;
		static std::vector<DemoInfo> Demos;

		static Dvar::Var CLAutoRecord;
		static Dvar::Var CLDemosKeep;

		static char BaselineSnapshot[131072];
		static int BaselineSnapshotMsgLen;
		static int BaselineSnapshotMsgOff;

		static void WriteBaseline();
		static void StoreBaseline(PBYTE snapshotMsg);

		static void LoadDemos([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
		static void DeleteDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
		static void PlayDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);

		static unsigned int GetDemoCount();
		static const char* GetDemoText(unsigned int item, int column);
		static void SelectDemo(unsigned int index);

		static void GamestateWriteStub(Game::msg_t* msg, char byte);
		static void RecordGamestateStub();
		static void BaselineStoreStub();
		static void BaselineToFileStub();
		static void AdjustTimeDeltaStub();
		static void ServerTimedOutStub();
		static void UISetActiveMenuStub();

		static int CL_FirstSnapshot_Stub();
		static void SV_SpawnServer_Stub();

		static void CG_CompassDrawPlayerMapLocationSelector_Stub(int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, Game::Material* material, float* color);
		static void CL_WriteDemoClientArchive_Hk(void(*write)(const void* buffer, int len, int localClientNum), const Game::playerState_s* ps, const float* viewangles, const float* selectedLocation, float selectedLocationAngle, int localClientNum, int index);

		static void RecordStub(int channel, char* message, char* file);
		static void StopRecordStub(int channel, char* message);
	};
}
