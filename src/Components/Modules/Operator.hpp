#pragma once

extern "C"
{
  void Operator_SV_GetChallengeStub();
  void Operator_ArchiveTempBansStub();
  void Operator_RestoreTempBansStub();

  bool Operator_SV_GetChallenge(const char* guid, Game::netadr_t* challenge);
  void Operator_ArchiveTempBans(void* memFile);
  void Operator_RestoreTempBans(void* memFile);
}

namespace Components
{
  // The server commands that x86 registers in SV_AddOperatorCommands(),
  // where x64 only registers hostmigration_start: the commands to look at
  // the server and its players, to kick and briefly ban them, to restart the
  // map and kill the server, and a few for debugging.
  //
  // A brief ban lasts sv_kickBanTime seconds, during which the server
  // rejects the banned guid's challenge, and carries over to the new host
  // when the server migrates. x64 compiles out both, so we put them back.
  //
  // Note that the map, devmap and map_rotate commands are added by the Maps
  // and MapRotation components.
  //
  class Operator : public Component
  {
  public:
    Operator();

  private:
    static void AddServerCommands();
  };
}
