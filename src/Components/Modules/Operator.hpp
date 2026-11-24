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
  // The server commands that x86 registers in SV_AddOperatorCommands(), of
  // which x64 only kept hostmigration_start.
  //
  // That's a good part of what a server admin types into the console: the
  // commands to look at the server and its players, to kick and briefly ban
  // them, to restart the map or kill the server, plus a few for debugging.
  // Presumably the later revision moved all of this behind its own
  // backend, since a console server admin wasn't really a thing for it. We
  // bring them back as x86 has them, down to their output, so that existing
  // admin tools and habits keep working.
  //
  // A brief ban lasts sv_kickBanTime seconds, during which the server
  // rejects the challenge of the banned guid, and it carries over to the new
  // host when the server migrates. x64 compiles out both halves of that as
  // well, so we put them back too (see the constructor).
  //
  // Note that the map and devmap commands live in Maps and map_rotate in
  // MapRotation, which is where the rest of their logic is.
  //
  class Operator : public Component
  {
  public:
    Operator();

  private:
    static void AddServerCommands();
  };
}
