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
  // the samples they draw, so we keep the samples here and reimplement what
  // records and draws them from x86.
  //
  // Note also that x86 records only the frame and snapshot samples. The
  // net graphs draw the other ones, which stay zero.
  //
  extern lagometer_t lagometer;

  // Record the time since the latest snapshot for this frame.
  //
  void CG_AddLagometerFrameInfo(const cg_s* cgameGlob);

  // Record the ping and flags of the snapshot, or a dropped one if it's null.
  //
  void CG_AddLagometerSnapshotInfo(const snapshot_s* snap);

  // Draw the lagometer, and the connection interrupted notice after it.
  //
  void CG_DrawLagometer(int localClientNum);

  // Draw the connection interrupted notice once our oldest command is
  // newer than what the server acknowledged, and blink the disconnect icon.
  //
  void CG_DrawDisconnect(int localClientNum);

  // CG_DrawLagometer() draws the notice through this, which starts out as
  // CG_DrawDisconnect() and Components::SlowMotion points at its
  // cg_drawDisconnect check.
  //
  extern void (*CG_DrawLagometer_DrawDisconnect)(int localClientNum);

  void CG_DrawEntityCount(int localClientNum);
  void CG_DrawServerBandwidth(int localClientNum);
  void CG_DrawServerKillcamData(int localClientNum);
}
