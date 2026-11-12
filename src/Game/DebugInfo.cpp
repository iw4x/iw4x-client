#include "DebugInfo.hpp"

namespace Game
{
  const dvar_t* cg_debugInfoCornerOffset;

  namespace
  {
    // Number of frames that cg_drawFPS summarizes.
    //
    constexpr int FPS_FRAMES = 32;

    // Levels of the cg_drawFPS enum.
    //
    constexpr int FPS_SIMPLE_RANGES = 2;
    constexpr int FPS_VERBOSE_VIEWPOS = 4;

    // Text alignment of DrawAlignedText(): horizontal in the low two bits,
    // vertical in the next two.
    //
    constexpr int TEXT_ALIGN_LEFT = 1;
    constexpr int TEXT_ALIGN_RIGHT = 2;
    constexpr int TEXT_ALIGN_CENTER = 3;
    constexpr int TEXT_ALIGN_HORIZONTAL = 3;
    constexpr int TEXT_ALIGN_TOP = 4;
    constexpr int TEXT_ALIGN_MIDDLE = 0xC;
    constexpr int TEXT_ALIGN_VERTICAL = 0xC;

    // Screen alignment that leaves virtual coordinates unanchored.
    //
    constexpr int SCREEN_ALIGN_LEFT = 1;
    constexpr int SCREEN_ALIGN_TOP = 1;

    constexpr float TEXT_SCALE_X = 1.0f;
    constexpr float TEXT_SCALE_Y = 1.1f;
    constexpr float LINE_SPACING = 0.75f;

    constexpr float colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr float colorRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr float colorLabel[4] = {1.0f, 1.0f, 1.0f, 0.75f};
    constexpr float colorCaption[4] = {0.0f, 1.0f, 0.0f, 0.75f};

    // Milliseconds of the last FPS_FRAMES frames, as a ring indexed by the
    // running frame count.
    //
    int frameTimes[FPS_FRAMES];
    int frameCount;
    int previousFrameTime;

    void RecordFrameTime()
    {
      const int now = Sys_Milliseconds();

      frameTimes[frameCount % FPS_FRAMES] = now - previousFrameTime;
      previousFrameTime = now;
      ++frameCount;
    }

    // Draw the text aligned to the point and return the font height.
    //
    int DrawAlignedText(const ScreenPlacement* scrPlace, float x, float y, float xScale, float yScale, const char* text, const float* color, int align, Font_s* font)
    {
      switch (align & TEXT_ALIGN_HORIZONTAL)
      {
      case TEXT_ALIGN_RIGHT:
        x -= static_cast<float>(R_TextWidth(text, 0, font)) * xScale;
        break;
      case TEXT_ALIGN_CENTER:
        x -= static_cast<float>(R_TextWidth(text, 0, font)) * xScale * 0.5f;
        break;
      }

      const int height = R_TextHeight(font);

      switch (align & TEXT_ALIGN_VERTICAL)
      {
      case TEXT_ALIGN_TOP:
        y += static_cast<float>(height) * yScale;
        break;
      case TEXT_ALIGN_MIDDLE:
        y += static_cast<float>(height) * yScale * 0.5f;
        break;
      }

      float w = xScale;
      float h = yScale;
      ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, SCREEN_ALIGN_LEFT, SCREEN_ALIGN_TOP);
      R_AddCmdDrawText(text, std::numeric_limits<int>::max(), font, x, y, w, h, 0.0f, color, ITEM_TEXTSTYLE_NORMAL);

      return height;
    }

    // Return the right edge of the debug info lines.
    //
    float CornerX(const ScreenPlacement* scrPlace)
    {
      return scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0] + cg_debugInfoCornerOffset->current.vector[0];
    }

    float LabelWidth(const char* label)
    {
      return static_cast<float>(R_TextWidth(label, 0, cgMedia->smallDevFont));
    }

    // Draw a value line ending at the corner, with its label after it when
    // cg_drawFPSLabels is set, and return the height to advance by.
    //
    float CornerPrint(const ScreenPlacement* scrPlace, float x, float y, float labelWidth, const char* text, const char* label, const float* color)
    {
      Font_s* font = cgMedia->smallDevFont;

      if ((*cg_drawFPSLabels)->current.enabled)
      {
        const float valueX = x - labelWidth;
        const int valueHeight = DrawAlignedText(scrPlace, valueX, y, TEXT_SCALE_X, TEXT_SCALE_Y, text, color, TEXT_ALIGN_RIGHT | TEXT_ALIGN_TOP, font);
        const int labelHeight = DrawAlignedText(scrPlace, valueX, y, TEXT_SCALE_X, TEXT_SCALE_Y, label, colorLabel, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP, font);

        return static_cast<float>(std::max(valueHeight, labelHeight)) * LINE_SPACING;
      }

      return static_cast<float>(DrawAlignedText(scrPlace, x, y, TEXT_SCALE_X, TEXT_SCALE_Y, text, color, TEXT_ALIGN_RIGHT | TEXT_ALIGN_TOP, font)) * LINE_SPACING;
    }

    // Draw a section caption, centered over the values when labels are on,
    // and return the height to advance by.
    //
    float CaptionPrint(const ScreenPlacement* scrPlace, float x, float y, float labelWidth, const char* text, const float* color)
    {
      Font_s* font = cgMedia->smallDevFont;
      int height;

      if ((*cg_drawFPSLabels)->current.enabled)
      {
        height = DrawAlignedText(scrPlace, x - labelWidth, y, TEXT_SCALE_X, TEXT_SCALE_Y, text, color, TEXT_ALIGN_CENTER | TEXT_ALIGN_TOP, font);
      }
      else
      {
        height = DrawAlignedText(scrPlace, x, y, TEXT_SCALE_X, TEXT_SCALE_Y, text, color, TEXT_ALIGN_RIGHT | TEXT_ALIGN_TOP, font);
      }

      return static_cast<float>(height) * LINE_SPACING;
    }

    float DrawFps(const int localClientNum, const ScreenPlacement* scrPlace, float y)
    {
      // Wait until the ring holds a full set of frames.
      //
      if (frameCount < FPS_FRAMES)
      {
        return y;
      }

      const float x = CornerX(scrPlace);

      int total = 0;
      int fastest = std::numeric_limits<int>::max();
      int slowest = 0;

      for (const int time : frameTimes)
      {
        total += time;
        fastest = std::min(fastest, time);
        slowest = std::max(slowest, time);
      }

      const float average = static_cast<float>(total) / FPS_FRAMES;

      float deviation = 0.0f;
      for (const int time : frameTimes)
      {
        deviation += std::fabs(static_cast<float>(time) - average);
      }
      deviation /= FPS_FRAMES;

      // Note that frames faster than a millisecond measure as zero, so we
      // clamp the divisors to keep the rates finite.
      //
      total = std::max(total, 1);
      fastest = std::max(fastest, 1);
      slowest = std::max(slowest, 1);

      const int fps = static_cast<int>(std::lround(1000.0f * FPS_FRAMES / static_cast<float>(total)));
      const int fpsLow = static_cast<int>(std::lround(1000.0f / static_cast<float>(slowest)));
      const int fpsHigh = static_cast<int>(std::lround(1000.0f / static_cast<float>(fastest)));

      const int fpsLevel = (*cg_drawFPS)->current.integer;
      const float labelWidth = LabelWidth(" cg ms/frame");

      if (fpsLevel >= FPS_VERBOSE_VIEWPOS)
      {
        const cg_s* cg = CL_GetLocalClientGlobals(localClientNum);
        const float* org = cg->refdef.view.org;

        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("(%.1f %.1f %.1f) %.1f", org[0], org[1], org[2], cg->refdefViewAngles[1]), " viewpos", colorWhite);
      }

      // Fade the rate from red at 50 FPS to white at 60.
      //
      const float lerp = std::clamp((static_cast<float>(fps) - 50.0f) / 10.0f, 0.0f, 1.0f);

      float color[4];
      for (int i = 0; i < 4; ++i)
      {
        color[i] = colorRed[i] + (colorWhite[i] - colorRed[i]) * lerp;
      }

      if (fpsLevel >= FPS_SIMPLE_RANGES)
      {
        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("(%i-%i, %i) %i", fpsLow, fpsHigh, static_cast<int>(std::lround(deviation)), fps), " FPS", color);
        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("(%i-%i) %1.2f", fastest, slowest, static_cast<float>(total) / FPS_FRAMES), " cg ms/frame", colorWhite);

        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("%i/%i", DOBJ_HANDLE_COUNT - *dobjFreeCount, DOBJ_HANDLE_COUNT), " used DObjs", colorWhite);
      }
      else
      {
        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("%i", fps), " FPS", color);
      }

      return y;
    }

    float DrawCornerString(const ScreenPlacement* scrPlace, float y, const char* text, const char* label)
    {
      if (*text == '\0')
      {
        return y;
      }

      return y + CornerPrint(scrPlace, CornerX(scrPlace), y, LabelWidth(" cg ms/frame"), text, label, colorWhite);
    }

    float DrawSnapshot(const int localClientNum, float y)
    {
      const ScreenPlacement* scrPlace = ScrPlace_GetActivePlacement(localClientNum);
      const cg_s* cg = CL_GetLocalClientGlobals(localClientNum);
      const cgs_t* cgs = &cgsArray[localClientNum];

      const float x = CornerX(scrPlace);
      const float labelWidth = LabelWidth(" server time");

      y += CaptionPrint(scrPlace, x, y, labelWidth, "-Snapshot-", colorCaption);

      // Note that, as on x86, the server time is the one of the snapshot we
      // are interpolating towards, which is absent until it arrives.
      //
      if (cg->nextSnap)
      {
        y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("%i", cg->nextSnap->serverTime), " server time", colorWhite);
      }

      y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("%i", cg->latestSnapshotNum), " snap num", colorWhite);
      y += CornerPrint(scrPlace, x, y, labelWidth, Utils::String::VA("%i", cgs->serverCommandSequence), " cmd", colorWhite);

      return y;
    }

    float DrawViewpos(const int localClientNum, const ScreenPlacement* scrPlace, float y)
    {
      const float* org = CL_GetLocalClientGlobals(localClientNum)->refdef.view.org;

      return y + CornerPrint(scrPlace, CornerX(scrPlace), y, LabelWidth(""), Utils::String::VA("(%.0f %.0f %.0f)\n", org[0], org[1], org[2]), "", colorWhite);
    }
  }

  void CG_DrawUpperRightDebugInfo(const int localClientNum)
  {
    const ScreenPlacement* scrPlace = ScrPlace_GetActivePlacement(localClientNum);
    float y = cg_debugInfoCornerOffset->current.vector[1];

    if ((*cg_drawFPS)->current.integer != 0)
    {
      y = DrawFps(localClientNum, scrPlace, y);
      y = DrawCornerString(scrPlace, y, (*party_hostname)->current.string, " p-host");
      y = DrawCornerString(scrPlace, y, partyTeamCounts, " teams");
    }

    if ((*cg_drawSnapshot)->current.enabled)
    {
      y = DrawSnapshot(localClientNum, y);
    }

    if ((*cg_drawViewpos)->current.enabled)
    {
      DrawViewpos(localClientNum, scrPlace, y);
    }
  }

  void CL_DrawDebugOverlays(const int localClientNum)
  {
    UI_PaintMenus(localClientNum);
    RecordFrameTime();

    if (CL_GetLocalClientConnectionState(localClientNum) == CA_ACTIVE)
    {
      CG_DrawUpperRightDebugInfo(localClientNum);
    }
  }
}
