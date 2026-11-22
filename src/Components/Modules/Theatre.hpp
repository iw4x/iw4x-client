#pragma once

extern "C"
{
  void Theatre_BaselineStoreStub();
  void Theatre_AdjustTimeDeltaStub();
  void Theatre_UISetActiveMenuStub();

  void Theatre_StoreBaseline(Game::msg_t* snapshotMsg);
  bool Theatre_AdjustTimeDelta();
}

namespace Components
{
  class Theatre : public Component
  {
  public:
    Theatre();

    static void StopRecording();

  private:
    friend void (::Theatre_StoreBaseline)(Game::msg_t* snapshotMsg);
    friend bool (::Theatre_AdjustTimeDelta)();

    // The extension of the demos the game records, and that of the demos
    // that x86 records (and that ServerDemo writes on x64 as well).
    //
    // Note that the number in the extension is the protocol, which is 16 on
    // x64 and 13 on x86. But since we always speak x86 on the wire (see
    // Compatibility), what goes into a demo is the same either way, so we
    // play both.
    //
    static constexpr auto* DemoExtension = "dm_16";
    static constexpr auto* X86DemoExtension = "dm_13";

    class DemoInfo
    {
    public:
      std::string name;
      std::string extension;
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
    static void StoreBaseline(Game::msg_t* snapshotMsg);

    static void LoadDemos([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
    static void DeleteDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
    static void PlayDemo([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);

    static unsigned int GetDemoCount();
    static const char* GetDemoText(unsigned int item, int column);
    static void SelectDemo(unsigned int index);
    static bool AdjustTimeDelta();

    static void GamestateWriteStub(Game::msg_t* msg, char byte);
    static void RecordGamestateStub();
    static void BaselineToFileStub();
    static bool CL_ServerTimedOut_Stub();

    static int CL_FirstSnapshot_Stub();
    static void SV_SpawnServer_Stub();

    static void CG_CompassDrawPlayerMapLocationSelector_Stub(int localClientNum, Game::CompassType compassType, const Game::rectDef_s* parentRect, const Game::rectDef_s* rect, Game::Material* material, float* color);
    static void CL_WriteDemoClientArchive_Hk(void(*write)(const void* buffer, int len, int localClientNum), const Game::playerState_s* ps, const float* viewangles, const float* selectedLocation, float selectedLocationAngle, int localClientNum, int index);

    static Game::fileHandle_t RecordStub(const char* file);
    static void CL_StopRecord_f_Hk();

    static int CL_PlayDemo_f_CompareExtension(const char* nameEnd, const char* extension);
    static int CL_PlayDemo_f_OpenDemo(const char* path, Game::fileHandle_t* file);
  };
}
