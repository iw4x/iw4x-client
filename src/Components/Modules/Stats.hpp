#pragma once

namespace Components
{
  class Stats : public Component
  {
  public:
    static constexpr int MAX_PRESTIGE = 10;

    Stats();

    static bool IsMaxLevel();

  private:
    static void UpdateClasses([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);

    static void SendStats();

    // The Steam ID and the file name hash we encrypt the stats with, those of
    // the stat files of earlier IW4x versions.
    //
    static constexpr std::uint64_t STATS_ID = 0x110000100001337;
    static constexpr std::uint32_t STATS_HASH = 0xFC30D07E;

    static void AddScriptFunctions();

    static void SprintfLiveStorageFilename(char* target, size_t size);
    static void SprintfLiveStorageFilenameWithFsGame(char* target, size_t size, const char* fsGame);
    
    static void MoveOldStatsToNewFolder();

    static uint32_t HashFilename();

    static int LiveStorage_DataSetValue_Stub(void* state, void* ddlContext, int unused, const char* value);

    static void CG_SetStatsFromServer (int localClientNum);
    static void CG_InitUnlockedItems (int localClientNum);
  };
}
