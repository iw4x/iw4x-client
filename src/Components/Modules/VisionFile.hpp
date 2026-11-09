#pragma once

extern "C"
{
  bool VisionFile_LoadVisionSettingsFromBuffer_Stub();

  bool VisionFile_LoadVisionSettingsFromBuffer(const char* buffer, const char* filename, Game::visionSetVars_t* settings);
}

namespace Components
{
  class VisionFile : public Component
  {
  public:
    VisionFile();

  private:
    friend bool (::VisionFile_LoadVisionSettingsFromBuffer)(const char* buffer, const char* filename, Game::visionSetVars_t* settings);

    static std::vector<std::string> DvarExceptions;
    static std::unordered_map<std::string, std::string> VisionReplacements;

    static bool ApplyExemptDvar(const char* dvarName, const char** buffer, const char* filename);

    static bool LoadVisionSettingsFromBuffer(const char* buffer, const char* filename, Game::visionSetVars_t* settings);
  };
}
