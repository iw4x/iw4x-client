#pragma once

extern "C"
{
  void ScriptPatches_RemoveRefToObject_Stub();

  void ScriptPatches_RemoveRefToObject_Bad(unsigned int id, unsigned int caller);
}

namespace Components::GSC
{
  class ScriptPatches : public Component
  {
  public:
    ScriptPatches();

  private:
    friend void (::ScriptPatches_RemoveRefToObject_Bad)(unsigned int id, unsigned int caller);

    static Game::game_hudelem_s* HECmd_GetHudElem(Game::scr_entref_t entref);

    static void Scr_TableLookupIStringByRow_Hk();

    static std::string DescribeFrames ();
    static void ReportCorruptState (const std::string& logger, const std::string& details);

    static unsigned int VM_ArchiveStack_Hk ();

    static void RemoveRefToObject_Bad (unsigned int id, unsigned int caller);
  };
}
