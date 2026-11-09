#pragma once

extern "C"
{
  void ArenaLength_CompareMapNameStub1();
  void ArenaLength_CompareMapNameStub2();
  void ArenaLength_CompareMapNameStub3();
  void ArenaLength_ReadVoteReferenceStub();
}

namespace Components
{
  class ArenaLength : public Component
  {
  public:
    ArenaLength();

    static Game::newMapArena_t NewArenas[];
    static char* NewArenaInfos[];
  };
}
