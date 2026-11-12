#pragma once

namespace Components
{
  class ArenaLength : public Component
  {
  public:
    ArenaLength();

    static Game::newMapArena_t* NewArenas;
    static char** NewArenaInfos;

  private:
    static void CopyMapName(char* dest, const char* src, int destsize);
    static void SetMapNameDvar(const char* dvarName, const char* oldMapName);
    static int CompareMapName(const char* name, const char* oldMapName);
    static const char* FormatMapName(const char* fmt, const char* oldMapName);
  };
}
