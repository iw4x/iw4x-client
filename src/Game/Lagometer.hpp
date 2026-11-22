#pragma once

namespace Game
{
  constexpr int LAG_SAMPLES = 128;

  struct lagometer_t
  {
    int frameSamples[LAG_SAMPLES];
    int frameCount;
    int snapshotFlags[LAG_SAMPLES];
    int snapshotSamples[LAG_SAMPLES];
    int entitySamples[LAG_SAMPLES];
    int changedEntitySamples[LAG_SAMPLES];
    int archivedEntitySamples[LAG_SAMPLES];
    int serverBandwidth[LAG_SAMPLES];
    int killcamTimes[LAG_SAMPLES];
    int snapshotCount;
  };

  // Note that x64 compiles out the lagometer and the net graphs along with
  // the samples they draw. So we keep the samples here and reimplement from
  // x86 what records and draws them.
  //
  // Note also that x86 only ever records the frame and snapshot samples.
  // The net graphs draw the other ones as well, which just stay zero. Why
  // x86 has them at all we don't know, perhaps a leftover from a console
  // build.
  //
  extern lagometer_t lagometer;

  // Record the time since the latest snapshot for this frame.
  //
  void CG_AddLagometerFrameInfo(const cg_s* cgameGlob);

  // Record the ping and flags of the snapshot, or a dropped one if it's
  // null.
  //
  void CG_AddLagometerSnapshotInfo(const snapshot_s* snap);

  // Draw the lagometer, followed by the connection interrupted notice.
  //
  void CG_DrawLagometer(int localClientNum);

  // Draw the connection interrupted notice once our oldest command is newer
  // than what the server has acknowledged, and blink the disconnect icon.
  //
  void CG_DrawDisconnect(int localClientNum);

  // CG_DrawLagometer() draws the notice through this. It starts out as
  // CG_DrawDisconnect(), and Components::SlowMotion points it at its
  // cg_drawDisconnect check.
  //
  extern void (*CG_DrawLagometer_DrawDisconnect)(int localClientNum);

  void CG_DrawEntityCount(int localClientNum);
  void CG_DrawServerBandwidth(int localClientNum);
  void CG_DrawServerKillcamData(int localClientNum);
}
