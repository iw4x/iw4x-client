#include "Lagometer.hpp"

namespace Game
{
  lagometer_t lagometer;

  void (*CG_DrawLagometer_DrawDisconnect)(int localClientNum) = CG_DrawDisconnect;

  namespace
  {
    // Alignment against the right and bottom of the safe area.
    //
    constexpr int HORIZONTAL_ALIGN_RIGHT = 3;
    constexpr int VERTICAL_ALIGN_BOTTOM = 3;

    // Text alignment of CG_DrawDevString(), right and bottom.
    //
    constexpr int TEXT_ALIGN_RIGHT_BOTTOM = 10;

    // The game's SNAPFLAG_RATE_DELAYED.
    //
    constexpr int SNAPFLAG_RATE_DELAYED = 1;

    // CMD_BACKUP - 1.
    //
    constexpr int CMD_BACKUP_LAST = 127;

    constexpr int SCREEN_WIDTH = 640;

    constexpr float colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr float colorBlack[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr float colorRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr float colorGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    constexpr float colorBlue[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    constexpr float colorYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    constexpr float colorOrange[4] = {1.0f, 0.7f, 0.0f, 1.0f};
    constexpr float colorMdCyan[4] = {0.0f, 0.5f, 0.5f, 1.0f};

    // Note that x64 compiles out CL_IsCGameInitialized() along with its
    // callers here.
    //
    bool CL_IsCGameInitialized(const int localClientNum)
    {
      return clientUIActives[localClientNum].cgameInitialized;
    }

    // Copy the command out of the ring of the last CMD_BACKUP ones, unless it
    // has fallen out of it.
    //
    // Note that x64 compiles out CL_GetUserCmd() along with its callers
    // here.
    //
    bool CL_GetUserCmd(const int localClientNum, const int cmdNumber, usercmd_s* ucmd)
    {
      const auto& cl (clients[localClientNum]);

      if (cmdNumber > cl.cmdNumber)
      {
        Com_Error(ERR_DROP, "\x15" "CL_GetUserCmd: %i >= %i", cmdNumber, cl.cmdNumber);
      }

      if (cmdNumber <= cl.cmdNumber - CMD_BACKUP_LAST - 1)
      {
        return false;
      }

      *ucmd = cl.cmds[cmdNumber & CMD_BACKUP_LAST];
      return true;
    }

    void CG_DrawBigDevString(const ScreenPlacement* scrPlace, const float x, const float y, const char* text, const float alpha, const int align)
    {
      const float color[4] = {1.0f, 1.0f, 1.0f, alpha};
      CG_DrawDevString(scrPlace, x, y, 1.0f, 1.0f, text, color, align, cgMedia->bigDevFont);
    }

    // Draw the graph's caption under it and its background, both against the
    // right bottom.
    //
    void DrawGraphBack(ScreenPlacement* scrPlace, const float x, const float y, const float size, const char* text, const float* color)
    {
      UI_DrawText(scrPlace, text, 500, cgMedia->smallDevFont, static_cast<float>(x - 50.0), static_cast<float>(size + y + 18.0), HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.3f, color, ITEM_TEXTSTYLE_SHADOWED);
      UI_DrawHandlePic(scrPlace, x, y, size, size, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, colorBlack, cgMedia->whiteMaterial);
    }

    // Draw a one unit wide bar of the graph standing on its bottom.
    //
    void DrawGraphBar(ScreenPlacement* scrPlace, const float x, const float bottom, const float h, const float* color)
    {
      CL_DrawStretchPic(scrPlace, x, bottom - h, 1.0f, h, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.0f, 0.0f, 1.0f, 1.0f, color, cgMedia->whiteMaterial);
    }
  }

  void CG_AddLagometerFrameInfo(const cg_s* cgameGlob)
  {
    lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = cgameGlob->time - cgameGlob->latestSnapshotTime;
    ++lagometer.frameCount;
  }

  void CG_AddLagometerSnapshotInfo(const snapshot_s* snap)
  {
    if (snap == nullptr)
    {
      lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
      ++lagometer.snapshotCount;
      return;
    }

    lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
    lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
    ++lagometer.snapshotCount;
  }

  void CG_DrawDisconnect(const int localClientNum)
  {
    if (clientConnections[localClientNum].isServerRestarting ||
        clientConnections[localClientNum].demoplaying         ||
        clientUIActives[localClientNum].migrationState)
    {
      return;
    }

    const auto& cg (cgArray[localClientNum]);

    usercmd_s cmd;
    CL_GetUserCmd(localClientNum, clients[localClientNum].cmdNumber - CMD_BACKUP_LAST, &cmd);

    if (cmd.serverTime <= cg.predictedPlayerState.commandTime || cmd.serverTime > cg.time)
    {
      return;
    }

    auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));

    const auto* text (UI_SafeTranslateString("CGAME_CONNECTIONINTERUPTED"));
    auto* font (UI_GetFontHandle(scrPlace, 0, 0.5f));
    const auto w (UI_TextWidth(text, 0, font, 0.5f));

    UI_DrawText(scrPlace, text, std::numeric_limits<int>::max(), font, static_cast<float>((SCREEN_WIDTH - w) / 2), 100.0f, 0, 0, 0.5f, colorWhite, ITEM_TEXTSTYLE_SHADOWED);

    // Blink the icon.
    //
    if (cg.time & 0x200)
    {
      return;
    }

    UI_DrawHandlePic(scrPlace, 296.0f, 320.0f, 48.0f, 48.0f, 0, 0, nullptr, Material_RegisterHandle("net_disconnect", 7));
  }

  void CG_DrawLagometer(const int localClientNum)
  {
    if (!CL_IsCGameInitialized(localClientNum))
    {
      return;
    }

    if (!(*drawLagometer)->current.enabled || cgsArray[localClientNum].localServer)
    {
      CG_DrawLagometer_DrawDisconnect(localClientNum);
      return;
    }

    auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));

    UI_DrawHandlePic(scrPlace, -55.0f, -140.0f, 48.0f, 48.0f, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, nullptr, cgMedia->lagometerMaterial);

    // Draw the frame interpolate and extrapolate graph, above and below its
    // middle line.
    //
    for (auto i = 0; static_cast<float>(i) < 48.0f; ++i)
    {
      const auto index ((lagometer.frameCount - i - 1) & (LAG_SAMPLES - 1));
      auto v (static_cast<float>(lagometer.frameSamples[index] * 0.0533333346247673));
      const auto x (static_cast<float>(-7.0 - i));

      if (v > 0.0f)
      {
        if (v > 16.0f)
        {
          v = 16.0f;
        }

        CL_DrawStretchPic(scrPlace, x, static_cast<float>(-124.0 - v), 1.0f, v, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.0f, 0.0f, 0.0f, 0.0f, colorYellow, cgMedia->whiteMaterial);
      }
      else if (v < 0.0f)
      {
        v = -v;
        if (v > 16.0f)
        {
          v = 16.0f;
        }

        CL_DrawStretchPic(scrPlace, x, -124.0f, 1.0f, v, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.0f, 0.0f, 0.0f, 0.0f, colorBlue, cgMedia->whiteMaterial);
      }
    }

    // Draw the snapshot latency and drop graph.
    //
    for (auto i = 0; static_cast<float>(i) < 48.0f; ++i)
    {
      const auto index ((lagometer.snapshotCount - i - 1) & (LAG_SAMPLES - 1));
      const auto sample (static_cast<float>(lagometer.snapshotSamples[index]));
      const auto x (static_cast<float>(-7.0 - i));

      if (sample > 0.0f)
      {
        const auto* color ((lagometer.snapshotFlags[index] & SNAPFLAG_RATE_DELAYED) ? colorYellow : colorGreen);

        auto v (static_cast<float>(sample * 0.02666666731238365));
        if (v > 24.0f)
        {
          v = 24.0f;
        }

        CL_DrawStretchPic(scrPlace, x, static_cast<float>(-92.0 - v), 1.0f, v, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.0f, 0.0f, 0.0f, 0.0f, color, cgMedia->whiteMaterial);
      }
      else if (sample < 0.0f)
      {
        CL_DrawStretchPic(scrPlace, x, -116.0f, 1.0f, 24.0f, HORIZONTAL_ALIGN_RIGHT, VERTICAL_ALIGN_BOTTOM, 0.0f, 0.0f, 0.0f, 0.0f, colorRed, cgMedia->whiteMaterial);
      }
    }

    if ((*cg_nopredict)->current.enabled || (*g_synchronousClients)->current.enabled)
    {
      CG_DrawBigDevString(scrPlace, 633.0f, 340.0f, "snc", 1.0f, TEXT_ALIGN_RIGHT_BOTTOM);
    }

    CG_DrawLagometer_DrawDisconnect(localClientNum);
  }

  void CG_DrawEntityCount(const int localClientNum)
  {
    if (!CL_IsCGameInitialized(localClientNum) || !(*drawEntityCount)->current.enabled)
    {
      return;
    }

    const auto x ((*drawEntityCountPos)->current.vector[0]);
    const auto y ((*drawEntityCountPos)->current.vector[1]);
    auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));
    const auto size (static_cast<float>((*drawEntityCountSize)->current.integer));
    const auto bottom (size + y);

    const auto current (lagometer.frameCount & (LAG_SAMPLES - 1));
    DrawGraphBack(scrPlace, x, y, size, Utils::String::VA("Ents: %3i/%3i/%3i", lagometer.changedEntitySamples[current], lagometer.archivedEntitySamples[current], lagometer.entitySamples[current]), colorWhite);

    const auto scale (static_cast<float>(size / 650.0));
    const auto right (size + x);

    for (auto i = 0; static_cast<float>(i) < size; ++i)
    {
      const auto index ((lagometer.frameCount - i - 1) & (LAG_SAMPLES - 1));
      const auto barX (right - static_cast<float>(i) - 1.0f);

      const auto entities (lagometer.entitySamples[index]);
      DrawGraphBar(scrPlace, barX, bottom, static_cast<float>(entities) * scale, entities >= 500 ? colorRed : colorBlue);

      const auto archived (lagometer.archivedEntitySamples[index]);
      DrawGraphBar(scrPlace, barX, bottom, static_cast<float>(archived) * scale, archived >= 300 ? colorOrange : colorMdCyan);

      DrawGraphBar(scrPlace, barX, bottom, static_cast<float>(lagometer.changedEntitySamples[index]) * scale, colorWhite);
    }
  }

  void CG_DrawServerBandwidth(const int localClientNum)
  {
    if (!CL_IsCGameInitialized(localClientNum) || !(*drawServerBandwidth)->current.enabled)
    {
      return;
    }

    const auto x ((*drawServerBandwidthPos)->current.vector[0]);
    const auto y ((*drawServerBandwidthPos)->current.vector[1]);
    auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));
    const auto size (static_cast<float>((*drawServerBandwidthSize)->current.integer));
    const auto bottom (size + y);

    // The kilobits per second the server needs for its clients, with some
    // headroom.
    //
    const auto required (static_cast<int>(Live_NecessaryBandwidth((*sv_maxclients)->current.integer) * 0.0009765625 * 0.8999999761581421));

    const auto current (lagometer.serverBandwidth[lagometer.frameCount & (LAG_SAMPLES - 1)]);
    DrawGraphBack(scrPlace, x, y, size, Utils::String::VA("kbps: %3i", current), current > required ? colorRed : colorWhite);

    const auto scale (size / static_cast<float>(required));
    const auto right (size + x);

    for (auto i = 0; static_cast<float>(i) < size; ++i)
    {
      const auto index ((lagometer.frameCount - i - 1) & (LAG_SAMPLES - 1));
      const auto sample (lagometer.serverBandwidth[index]);

      auto h (static_cast<float>(sample) * scale);
      if (size < h)
      {
        h = size;
      }

      DrawGraphBar(scrPlace, right - static_cast<float>(i) - 1.0f, bottom, h, sample >= required ? colorRed : colorBlue);
    }
  }

  void CG_DrawServerKillcamData(const int localClientNum)
  {
    if (!CL_IsCGameInitialized(localClientNum) || !(*drawKillcamData)->current.enabled)
    {
      return;
    }

    const auto x ((*drawKillcamDataPos)->current.vector[0]);
    const auto y ((*drawKillcamDataPos)->current.vector[1]);
    auto* scrPlace (ScrPlace_GetActivePlacement(localClientNum));
    const auto size (static_cast<float>((*drawKillcamDataSize)->current.integer));
    const auto bottom (size + y);

    const auto current (lagometer.killcamTimes[lagometer.frameCount & (LAG_SAMPLES - 1)]);
    DrawGraphBack(scrPlace, x, y, size, Utils::String::VA("killcam: %3i", current), current < 10000 ? colorRed : colorWhite);

    const auto scale (static_cast<float>(size / 20000.0));
    const auto right (size + x);

    for (auto i = 0; static_cast<float>(i) < size; ++i)
    {
      const auto index ((lagometer.frameCount - i - 1) & (LAG_SAMPLES - 1));
      const auto sample (lagometer.killcamTimes[index]);

      auto h (static_cast<float>(sample) * scale);
      if (size < h)
      {
        h = size;
      }

      DrawGraphBar(scrPlace, right - static_cast<float>(i) - 1.0f, bottom, h, sample >= 10000 ? colorBlue : colorRed);
    }
  }
}
