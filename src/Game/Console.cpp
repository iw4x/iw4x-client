#include "Console.hpp"

#include <version.hpp>

namespace Game
{
  Con_CheckResize_t Con_CheckResize = Con_CheckResize_t(0x1400ED190);
  CL_ConsolePrint_t CL_ConsolePrint = CL_ConsolePrint_t(0x1400EB810);
  Con_UpdateNotifyLine_t Con_UpdateNotifyLine = Con_UpdateNotifyLine_t(0x1400EE410);
  PrintableCharsCount_t PrintableCharsCount = PrintableCharsCount_t(0x1400EE4A0);
  SEH_PrintStrlen_t SEH_PrintStrlen = SEH_PrintStrlen_t(0x14024D560);
  Dvar_ForEach_t Dvar_ForEach = Dvar_ForEach_t(0x140283620);
  Dvar_ValueToString_t Dvar_ValueToString = Dvar_ValueToString_t(0x140286BB0);
  Dvar_ValuesEqual_t Dvar_ValuesEqual = Dvar_ValuesEqual_t(0x140286D80);
  I_strnicmp_t I_strnicmp = I_strnicmp_t(0x14028A8E0);
  I_stristr_t I_stristr = I_stristr_t(0x14028A710);
  I_strncat_t I_strncat = I_strncat_t(0x14028A7E0);
  I_strcmpwild_t I_strcmpwild = I_strcmpwild_t(0x14028A680);
  Cmd_TokenizeString_t Cmd_TokenizeString = Cmd_TokenizeString_t(0x1401E7DE0);
  Cmd_EndTokenizedString_t Cmd_EndTokenizedString = Cmd_EndTokenizedString_t(0x1401E7180);
  Field_KeyDownEvent_t Field_KeyDownEvent = Field_KeyDownEvent_t(0x1400EF070);
  Field_CharEvent_t Field_CharEvent = Field_CharEvent_t(0x1400EED20);
  ScrPlace_EndFrame_t ScrPlace_EndFrame = ScrPlace_EndFrame_t(0x1400F2B60);
  SCR_UpdateScreen_t SCR_UpdateScreen = SCR_UpdateScreen_t(0x140101C10);
  CL_IsNotInGame_t CL_IsNotInGame = CL_IsNotInGame_t(0x1400F3090);
  getBuildNumber_t getBuildNumber = getBuildNumber_t(0x1401E6960);

  ConChannelDef* con_channels = reinterpret_cast<ConChannelDef*>(0x1406C81B0);

  Console* con = reinterpret_cast<Console*>(0x1406C24A0);

  const dvar_t** monkeytoy = reinterpret_cast<const dvar_t**>(0x1406C2490);
  const dvar_t** con_matchPrefixOnly = reinterpret_cast<const dvar_t**>(0x1406C2498);
  const dvar_t** con_inputBoxColor = reinterpret_cast<const dvar_t**>(0x1406C4EB0);
  const dvar_t** con_inputHintBoxColor = reinterpret_cast<const dvar_t**>(0x1406C4EB8);
  const dvar_t** con_outputBarColor = reinterpret_cast<const dvar_t**>(0x1406C4EC0);
  const dvar_t** con_outputSliderColor = reinterpret_cast<const dvar_t**>(0x1406C4EC8);
  const dvar_t** con_outputWindowColor = reinterpret_cast<const dvar_t**>(0x1406C4ED0);
  const dvar_t** con_typewriterPrintSpeed = reinterpret_cast<const dvar_t**>(0x1406C1BD8);
  const dvar_t** con_typewriterDecayStartTime = reinterpret_cast<const dvar_t**>(0x1406C1CF8);
  const dvar_t** con_typewriterDecayDuration = reinterpret_cast<const dvar_t**>(0x1406C1F18);
  const dvar_t** com_filter_output = reinterpret_cast<const dvar_t**>(0x141BD7918);

  field_t* historyEditLines = reinterpret_cast<field_t*>(0x1406C5C50);

  const dvar_t* con_scale;

  static_assert(offsetof(clientStatic_t, rendererStarted) == 0x108);
  static_assert(offsetof(clientStatic_t, whiteMaterial) == 0x168);
  static_assert(offsetof(clientStatic_t, consoleFont) == 0x178);

  namespace
  {
    // Server time of the local client, cl.serverTime.
    //
    // Note that clientActive_t keeps the x86 layout, so we read it directly.
    //
    const int* cl_serverTime = reinterpret_cast<const int*>(0x1406CFE18);

    // Flag that marks a key as a character for UI_KeyEvent().
    //
    constexpr int K_CHAR_FLAG = 0x400;

    constexpr int CON_LINE_COUNT = 1024;
    constexpr int CON_TEXT_SIZE = 0x10000;
    constexpr int CON_HISTORY_COUNT = 32;

    // Most matches that the input hint lists.
    //
    constexpr int CON_MAX_SHOWN_MATCHES = 24;

    // Most arguments that the input hint lists.
    //
    constexpr int CON_MAX_SHOWN_ARGS = 16;

    // Layout of the input hint and the output window, before the IW4x
    // growth with the font height.
    //
    constexpr double CON_PADDING = 6.0;
    constexpr double CON_BORDER = 2.0;
    constexpr double CON_INPUT_BOX_HEIGHT = 32.0;
    constexpr double CON_BUILD_STRING_HEIGHT = 16.0;
    constexpr double CON_DVAR_COLUMN_WIDTH = 348.0;
    constexpr double CON_SCROLL_BAR_WIDTH = 10.0;
    constexpr float CON_SLIDER_MIN_HEIGHT = 10.0f;

    // Characters of the match names and values that the input hint shows.
    //
    constexpr int CON_NAME_CHARS = 28;
    constexpr int CON_VALUE_CHARS = 40;

    // Alignment of the input line: virtual coordinates, unanchored.
    //
    constexpr int CON_INPUT_ALIGN = 5;

    // Colors of the console text. The prompt and the build string take the
    // IW4x color in place of yellow.
    //
    constexpr float colorConsole[4] = {0.70f, 1.00f, 0.00f, 1.00f};
    constexpr float colorDvarName[4] = {1.0f, 1.0f, 0.8f, 1.0f};
    constexpr float colorDvarValue[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr float colorDvarInactive[4] = {0.8f, 0.8f, 0.8f, 1.0f};
    constexpr float colorCmdName[4] = {0.8f, 0.8f, 1.0f, 1.0f};
    constexpr float colorDomain[4] = {0.8f, 0.8f, 1.0f, 1.0f};
    constexpr float colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    // IW4x prompt in place of "IW4 MP: 1.0> ".
    //
#ifdef EXPERIMENTAL_BUILD
    constexpr const char* conPrompt = "IW4x MP: " REVISION_STR "-develop> ";
#else
    constexpr const char* conPrompt = "IW4x MP: " REVISION_STR "> ";
#endif

    // Console window.
    //
    MessageLine consoleLines[CON_LINE_COUNT];
    Message consoleMessages[CON_LINE_COUNT];
    char consoleText[CON_TEXT_SIZE];
    MessageWindow consoleWindow;

    // Line of the window at the bottom of the output, and whether the
    // output shows.
    //
    int displayLineOffset;
    bool outputVisible;

    // Channels the console shows, a bit each.
    //
    std::uint32_t consoleFilter[CON_MAX_CHANNELS / 32];

    const dvar_t* con_default_console_filter;

    // Input line, its history, and the input hint.
    //
    field_t g_consoleField;
    int nextHistoryLine;
    int historyLine;

    ConDrawInputGlob conDrawInputGlob;

    // Whether the current match pass of the input hint and the tab
    // completion is the substring one.
    //
    bool conMatchSubstring;

    // Tab completion state.
    //
    // Note that tab completes only right after another key; tab after tab
    // cycles through the matches.
    //
    bool completeOnTab;
    const char* completionString;
    int completionMatchCount;
    int completionPrefixMatchCount;
    bool shortestMatchComplete;
    char shortestMatch[1024];

    cmd_function_s Con_Clear_f_VAR;
    cmd_function_s Con_ChannelList_f_VAR;
    cmd_function_s Con_ShowChannel_f_VAR;
    cmd_function_s Con_HideChannel_f_VAR;
    cmd_function_s Con_VisibleChannelList_f_VAR;

    // Directory and extension of the file argument that the commands
    // complete.
    //
    // Note that x64 commands carry no autocomplete fields, so we keep the
    // x86 ones here.
    //
    struct CmdAutoComplete
    {
      const char* cmdName;
      const char* dir;
      const char* ext;
    };

    constexpr CmdAutoComplete cmdAutoCompletes[] =
    {
      {"demo", "demos", "dm_13"},
      {"timedemo", "demos", "dm_13"},
      {"cinematic", "video", "roq"},
      {"map", "maps/mp", "d3dbsp"},
      {"devmap", "maps/mp", "d3dbsp"},
    };

    constexpr int ALLOC_TRACK_CONSOLE = 10;

    int Cmd_Argc()
    {
      return cmd_args->argc[cmd_args->nesting];
    }

    const char* Cmd_Argv(int argIndex)
    {
      return argIndex < Cmd_Argc() ? cmd_args->argv[cmd_args->nesting][argIndex] : "";
    }

    void Cmd_ForEach(void(*callback)(const char* name))
    {
      for (cmd_function_s* cmd(*cmd_functions); cmd != nullptr; cmd = cmd->next)
      {
        callback(cmd->name);
      }
    }

    // Font metrics and text drawing, scaled for the console font.
    //
    int Con_TextHeight(Font_s* font)
    {
      return static_cast<int>(static_cast<float>(R_TextHeight(font)) * Con_GetFontScale(font));
    }

    int Con_TextWidth(const char* text, int maxChars, Font_s* font)
    {
      return static_cast<int>(static_cast<float>(R_TextWidth(text, maxChars, font)) * Con_GetFontScale(font));
    }

    void Con_DrawText(const char* text, int maxChars, Font_s* font, float x, float y, const float* color)
    {
      const float scale(Con_GetFontScale(font));
      R_AddCmdDrawText(text, maxChars, font, x, y, scale, scale, 0.0f, color, 0);
    }

    // Growth of the console font height with con_scale.
    //
    double Con_FontHeightGrowth()
    {
      return static_cast<double>(Con_TextHeight(cls->consoleFont) - R_TextHeight(cls->consoleFont));
    }

    //
    // Console window.
    //

    // Time that the typewriter effect starts printing a line with the
    // flags, or 0 if the flags don't ask for it.
    //
    int Con_GetTypewriterStartTime(const MessageWindow* msgwnd, int flags)
    {
      if ((flags & 1) == 0)
      {
        return 0;
      }

      const int time(*cl_serverTime);

      // Start after the last typewriter line still printing.
      //
      for (int i(msgwnd->activeLineCount - 1); i >= 0; --i)
      {
        MessageLine* line(&msgwnd->lines[(msgwnd->firstLineIndex + i) % msgwnd->lineCount]);

        if ((line->flags & 1) != 0)
        {
          const int start(line->typingStartTime + PrintableCharsCount(msgwnd, line) * (*con_typewriterPrintSpeed)->current.integer + 150);

          if (start - time >= 250)
          {
            return start;
          }

          break;
        }
      }

      return time + 250;
    }

    // Time that the typewriter effect takes to print the line and fade it
    // out.
    //
    int Con_GetTypewriterDuration(const MessageWindow* msgwnd, MessageLine* line)
    {
      if ((line->flags & 1) == 0)
      {
        return 0;
      }

      const int printTime(PrintableCharsCount(msgwnd, line) * (*con_typewriterPrintSpeed)->current.integer);
      return std::max(printTime, (*con_typewriterDecayStartTime)->current.integer) + (*con_typewriterDecayDuration)->current.integer;
    }

    // Keep the bottom line of the output on the window when it loses its
    // first line.
    //
    void Con_ClampDisplayLineOffset()
    {
      if (--displayLineOffset < con->visibleLineCount)
      {
        displayLineOffset = std::min(consoleWindow.activeLineCount, con->visibleLineCount);
      }
    }

    void Con_FreeFirstMessageWindowLine(MessageWindow* msgwnd)
    {
      if (++msgwnd->firstLineIndex == msgwnd->lineCount)
      {
        msgwnd->firstLineIndex = 0;
      }

      --msgwnd->activeLineCount;

      Con_ClampDisplayLineOffset();
    }

    // Copy the pending line into the window text, dropping the oldest lines
    // that it overwrites.
    //
    void Con_CopyCurrentConsoleLineText(MessageWindow* msgwnd, MessageLine* msgLine)
    {
      const int length(static_cast<int>(con->lineOffset));

      while (msgwnd->activeLineCount != 0)
      {
        const int begin(msgwnd->textBufPos);
        const int end((begin + length + 1) & (msgwnd->textBufSize - 1));
        const int first(msgwnd->lines[msgwnd->firstLineIndex].textBufPos);

        const bool overwrites(end < begin ? first >= begin || first < end : first >= begin && first < end);

        if (!overwrites)
        {
          break;
        }

        --msgwnd->activeLineCount;

        if (++msgwnd->firstLineIndex == msgwnd->lineCount)
        {
          msgwnd->firstLineIndex = 0;
        }

        Con_ClampDisplayLineOffset();
      }

      const int room(msgwnd->textBufSize - msgwnd->textBufPos);

      if (length > room)
      {
        std::memcpy(&msgwnd->circularTextBuffer[msgwnd->textBufPos], con->textTempLine, room);
        std::memcpy(msgwnd->circularTextBuffer, &con->textTempLine[room], length - room);
      }
      else
      {
        std::memcpy(&msgwnd->circularTextBuffer[msgwnd->textBufPos], con->textTempLine, length);
      }

      msgLine->textBufPos = msgwnd->textBufPos;
      msgLine->textBufSize = length;

      msgwnd->textBufPos = (msgwnd->textBufPos + length) & (msgwnd->textBufSize - 1);
      msgwnd->circularTextBuffer[msgwnd->textBufPos] = '\n';
      msgwnd->textBufPos = (msgwnd->textBufPos + 1) & (msgwnd->textBufSize - 1);
    }

    void Con_UpdateMessageWindowLine(int localClientNum, MessageWindow* msgwnd, bool lineFeed, int flags)
    {
      if (msgwnd->lineCount == 0)
      {
        return;
      }

      const int time(localClientNum < 1 ? *cl_serverTime : 0);

      MessageLine* line(&msgwnd->lines[(msgwnd->firstLineIndex + msgwnd->activeLineCount) % msgwnd->lineCount]);
      line->messageIndex = msgwnd->messageIndex;
      line->flags = flags;
      line->typingStartTime = 0;
      line->lastTypingSoundTime = 0;
      line->typingStartTime = Con_GetTypewriterStartTime(msgwnd, flags);

      if (line->typingStartTime != 0)
      {
        msgwnd->messages[line->messageIndex].endTime = Con_GetTypewriterDuration(msgwnd, line) + line->typingStartTime;
      }

      Con_CopyCurrentConsoleLineText(msgwnd, line);

      if (!lineFeed)
      {
        return;
      }

      if (msgwnd->activeLineCount == msgwnd->lineCount)
      {
        Con_FreeFirstMessageWindowLine(msgwnd);
      }

      ++msgwnd->activeLineCount;

      // Shorten the time on screen of the oldest message past the padding.
      //
      const int pastPadding(msgwnd->padding - msgwnd->lineCount + msgwnd->activeLineCount);

      if (pastPadding > 0)
      {
        Message* message(&msgwnd->messages[msgwnd->lines[(msgwnd->firstLineIndex + pastPadding - 1) % msgwnd->lineCount].messageIndex]);

        if (message->endTime - msgwnd->fadeOut > time)
        {
          message->endTime = msgwnd->fadeOut + time;
        }
      }
    }

    void Con_Clear_f()
    {
      consoleWindow.textBufPos = 0;
      consoleWindow.messageIndex = 0;
      consoleWindow.firstLineIndex = 0;
      consoleWindow.activeLineCount = 0;
      con->lineOffset = 0;
      displayLineOffset = 0;
    }

    //
    // Console channels.
    //

    void Con_SetChannelVisible(int channel, bool visible)
    {
      const std::uint32_t bit(1u << (channel & 31));

      if (visible)
      {
        consoleFilter[channel >> 5] |= bit;
      }
      else
      {
        consoleFilter[channel >> 5] &= ~bit;
      }
    }

    bool Con_IsChannelInFilter(int channel)
    {
      return (consoleFilter[channel >> 5] & (1u << (channel & 31))) != 0;
    }

    // Show or hide the channels that match filter.
    //
    void Con_FilterAdd(const char* filter, bool show)
    {
      int count(0);

      for (int channel(1); channel < CON_MAX_CHANNELS; ++channel)
      {
        const char* name(con_channels[channel].name);

        if (name[0] == '\0' || I_strcmpwild(filter, name) != 0 || Con_IsChannelInFilter(channel) == show)
        {
          continue;
        }

        Con_SetChannelVisible(channel, show);
        Com_Printf(CON_CHANNEL_DONT_FILTER, show ? "Adding channel: %s\n" : "Hiding channel: %s\n", name);
        ++count;
      }

      if (count == 0)
      {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "No channels added or hidden\n");
      }
    }

    void Con_FilterShowChannel(bool show)
    {
      const int argc(Cmd_Argc());

      if (argc < 2)
      {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "USAGE: %s <channel>\n<channel> may include wildcards */?\n", Cmd_Argv(0));
        return;
      }

      for (int i(1); i < argc; ++i)
      {
        Con_FilterAdd(Cmd_Argv(i), show);
      }

      // Have the configuration written again.
      //
      *Sys::GetTls<int>(Sys::TLS_OFFSET::DVAR_MODIFIED_FLAGS) |= DVAR_ARCHIVE;
    }

    void Con_ShowChannel_f()
    {
      Con_FilterShowChannel(true);
    }

    void Con_HideChannel_f()
    {
      Con_FilterShowChannel(false);
    }

    void Con_ChannelList_f()
    {
      for (int channel(1); channel < CON_MAX_CHANNELS; ++channel)
      {
        if (con_channels[channel].name[0] != '\0')
        {
          Com_Printf(CON_CHANNEL_DONT_FILTER, "%s\n", con_channels[channel].name);
        }
      }
    }

    void Con_VisibleChannelList_f()
    {
      for (int channel(1); channel < CON_MAX_CHANNELS; ++channel)
      {
        if (con_channels[channel].name[0] != '\0' && Con_IsChannelInFilter(channel))
        {
          Com_Printf(CON_CHANNEL_DONT_FILTER, "%s\n", con_channels[channel].name);
        }
      }
    }

    //
    // Input line and its hint.
    //

    // Tokenize the input line and return the command, without the leading
    // slash and blanks. The caller ends the tokenized string.
    //
    const char* Con_TokenizeInput()
    {
      Cmd_TokenizeString(g_consoleField.buffer);

      const char* s(Cmd_Argc() > 0 ? Cmd_Argv(0) : "");

      if (*s == '\\' || *s == '/')
      {
        ++s;
      }

      while (std::isspace(static_cast<unsigned char>(*s)))
      {
        ++s;
      }

      return s;
    }

    bool Con_IsDvarCommand(const char* cmd)
    {
      return I_stricmp(cmd, "set") == 0 ||
             I_stricmp(cmd, "seta") == 0 ||
             I_stricmp(cmd, "sets") == 0 ||
             I_stricmp(cmd, "reset") == 0 ||
             I_stricmp(cmd, "toggle") == 0 ||
             I_stricmp(cmd, "togglep") == 0;
    }

    // Whether the input line has a blank after the command.
    //
    bool Con_AnySpaceAfterCommand()
    {
      const char* s(g_consoleField.buffer);

      while (std::isspace(static_cast<unsigned char>(*s)))
      {
        ++s;
      }

      for (; *s != '\0'; ++s)
      {
        if (std::isspace(static_cast<unsigned char>(*s)))
        {
          return true;
        }
      }

      return false;
    }

    // Offset of the first argument in the input line.
    //
    int Con_FirstArgumentOffset()
    {
      const char* s(g_consoleField.buffer);
      int i(0);

      while (std::isspace(static_cast<unsigned char>(s[i])))
      {
        ++i;
      }

      while (s[i] != '\0' && !std::isspace(static_cast<unsigned char>(s[i])))
      {
        ++i;
      }

      while (std::isspace(static_cast<unsigned char>(s[i])))
      {
        ++i;
      }

      return i;
    }

    // Whether query matches the first matchTextLen characters of matchText.
    //
    // The prefix pass compares the start of query. Otherwise query must hold
    // the characters in order, next to each other with con_matchPrefixOnly.
    //
    bool Con_IsAutoCompleteMatch(const char* query, const char* matchText, int matchTextLen)
    {
      if (!conMatchSubstring && (*con_matchPrefixOnly)->current.enabled)
      {
        return I_strnicmp(query, matchText, matchTextLen) == 0;
      }

      int matched(0);
      int c(std::tolower(static_cast<unsigned char>(matchText[0])));

      for (const char* p(query); *p != '\0'; ++p)
      {
        if (std::tolower(static_cast<unsigned char>(*p)) == c)
        {
          if (++matched == matchTextLen)
          {
            return true;
          }

          c = std::tolower(static_cast<unsigned char>(matchText[matched]));
        }
        else if ((*con_matchPrefixOnly)->current.enabled && matched != 0)
        {
          matched = 0;
          c = std::tolower(static_cast<unsigned char>(matchText[0]));
        }
      }

      return false;
    }

    bool Con_CommitToAutoComplete()
    {
      if (conDrawInputGlob.matchIndex < 0 || conDrawInputGlob.autoCompleteChoice[0] == '\0')
      {
        return false;
      }

      const char* cmd(Con_TokenizeInput());

      if (Con_IsDvarCommand(cmd))
      {
        Com_sprintf(g_consoleField.buffer, sizeof(g_consoleField.buffer), "%s %s", cmd, conDrawInputGlob.autoCompleteChoice);
      }
      else
      {
        std::strcpy(g_consoleField.buffer, conDrawInputGlob.autoCompleteChoice);
      }

      Cmd_EndTokenizedString();

      g_consoleField.cursor = static_cast<int>(std::strlen(g_consoleField.buffer));
      g_consoleField.buffer[g_consoleField.cursor++] = ' ';
      g_consoleField.buffer[g_consoleField.cursor] = '\0';
      g_consoleField.drawWidth = SEH_PrintStrlen(g_consoleField.buffer);

      Con_CancelAutoComplete();
      return true;
    }

    bool Con_CycleAutoComplete(int step)
    {
      if (!conDrawInputGlob.mayAutoComplete ||
          conDrawInputGlob.matchCount <= 1 ||
          conDrawInputGlob.matchCount >= CON_MAX_SHOWN_MATCHES ||
          (conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand()))
      {
        return false;
      }

      conDrawInputGlob.matchIndex += step;

      if (conDrawInputGlob.matchIndex < 0)
      {
        conDrawInputGlob.matchIndex = conDrawInputGlob.matchCount - 1;
      }
      else if (conDrawInputGlob.matchIndex >= conDrawInputGlob.matchCount)
      {
        conDrawInputGlob.matchIndex = 0;
      }

      return true;
    }

    // Input hint drawing, at the conDrawInputGlob pen.
    //
    void ConDraw_Box(float x, float y, float width, float height, const float* color)
    {
      Material* white(cls->whiteMaterial);

      R_AddCmdDrawStretchPic(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f, color, white);

      const float border[4] = {color[0] * 0.5f, color[1] * 0.5f, color[2] * 0.5f, color[3]};
      const float w(static_cast<float>(CON_BORDER));

      R_AddCmdDrawStretchPic(x, y, w, height, 0.0f, 0.0f, 0.0f, 0.0f, border, white);
      R_AddCmdDrawStretchPic(static_cast<float>(x + width - CON_BORDER), y, w, height, 0.0f, 0.0f, 0.0f, 0.0f, border, white);
      R_AddCmdDrawStretchPic(x, y, width, w, 0.0f, 0.0f, 0.0f, 0.0f, border, white);
      R_AddCmdDrawStretchPic(x, static_cast<float>(y + height - CON_BORDER), width, w, 0.0f, 0.0f, 0.0f, 0.0f, border, white);
    }

    void ConDrawInput_Box(int lineCount, const float* color)
    {
      const float x(static_cast<float>(conDrawInputGlob.x - CON_PADDING));
      const float y(static_cast<float>(conDrawInputGlob.y - CON_PADDING));
      const float height(static_cast<float>(lineCount * conDrawInputGlob.fontHeight + 2.0 * CON_PADDING));
      const float width(con->screenMax[0] - con->screenMin[0] - (x - con->screenMin[0]));

      ConDraw_Box(x, y, width, height, color);
    }

    void ConDrawInput_Text(const char* str, const float* color)
    {
      Con_DrawText(str, std::numeric_limits<int>::max(), cls->consoleFont, conDrawInputGlob.x, conDrawInputGlob.fontHeight + conDrawInputGlob.y, color);
    }

    void ConDrawInput_TextLimitChars(const char* str, int maxChars, const float* color)
    {
      Con_DrawText(str, maxChars, cls->consoleFont, conDrawInputGlob.x, conDrawInputGlob.fontHeight + conDrawInputGlob.y, color);
    }

    void ConDrawInput_TextAndOver(const char* str, const float* color)
    {
      ConDrawInput_Text(str, color);
      conDrawInputGlob.x += static_cast<float>(Con_TextWidth(str, 0, cls->consoleFont));
    }

    void ConDrawInput_NextLine()
    {
      conDrawInputGlob.y += conDrawInputGlob.fontHeight;
      conDrawInputGlob.x = conDrawInputGlob.leftX;
    }

    void ConDrawInput_DvarColumn()
    {
      conDrawInputGlob.x = static_cast<float>(conDrawInputGlob.x + CON_DVAR_COLUMN_WIDTH * Con_GetConsoleScale());
    }

    void ConDrawInput_IncrMatchCounter(const char* str)
    {
      if (!Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
      {
        return;
      }

      if (conDrawInputGlob.matchCount == conDrawInputGlob.matchIndex)
      {
        I_strncpyz(conDrawInputGlob.autoCompleteChoice, str, sizeof(conDrawInputGlob.autoCompleteChoice));
      }

      ++conDrawInputGlob.matchCount;

      if (str[conDrawInputGlob.inputTextLen] == '\0')
      {
        conDrawInputGlob.hasExactMatch = true;
      }
    }

    void ConDrawInput_IncrDvarMatchCounter(const dvar_t* dvar, void*)
    {
      ConDrawInput_IncrMatchCounter(dvar->name);
    }

    void ConDrawInput_DvarMatch(const dvar_t* dvar, void*)
    {
      if (!Con_IsAutoCompleteMatch(dvar->name, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
      {
        return;
      }

      ConDrawInput_TextLimitChars(dvar->name, CON_NAME_CHARS, colorDvarName);
      ConDrawInput_DvarColumn();
      ConDrawInput_TextLimitChars(Dvar_DisplayableValue(dvar), CON_VALUE_CHARS, colorDvarValue);
      ConDrawInput_NextLine();
    }

    void ConDrawInput_CmdMatch(const char* str)
    {
      if (!Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
      {
        return;
      }

      ConDrawInput_Text(str, colorCmdName);
      ConDrawInput_NextLine();
    }

    // Describe the values that a dvar of the type takes into buf, setting
    // outLineCount to the lines past the first.
    //
    // Note that x64 drops the descriptions, so we keep the x86 ones here.
    //
    const char* Dvar_DomainToString(dvar_type type, DvarLimits domain, char* buf, int bufSize, int* outLineCount)
    {
      *outLineCount = 0;

      const std::size_t size(static_cast<std::size_t>(bufSize));

      auto vector = [buf, size, &domain](int components)
      {
        const float min(domain.vector.min);
        const float max(domain.vector.max);
        const bool anyMin(min == -std::numeric_limits<float>::max());
        const bool anyMax(max == std::numeric_limits<float>::max());

        if (!anyMin && !anyMax)
        {
          std::snprintf(buf, size, "Domain is any %iD vector with components from %g to %g", components, min, max);
        }
        else if (!anyMin)
        {
          std::snprintf(buf, size, "Domain is any %iD vector with components %g or bigger", components, min);
        }
        else if (!anyMax)
        {
          std::snprintf(buf, size, "Domain is any %iD vector with components %g or smaller", components, max);
        }
        else
        {
          std::snprintf(buf, size, "Domain is any %iD vector", components);
        }
      };

      switch (type)
      {
      case DVAR_TYPE_BOOL:
        std::snprintf(buf, size, "Domain is 0 or 1");
        break;

      case DVAR_TYPE_FLOAT:
        {
          const float min(domain.value.min);
          const float max(domain.value.max);
          const bool anyMin(min == -std::numeric_limits<float>::max());
          const bool anyMax(max == std::numeric_limits<float>::max());

          if (!anyMin && !anyMax)
          {
            std::snprintf(buf, size, "Domain is any number from %g to %g", min, max);
          }
          else if (!anyMin)
          {
            std::snprintf(buf, size, "Domain is any number %g or bigger", min);
          }
          else if (!anyMax)
          {
            std::snprintf(buf, size, "Domain is any number %g or smaller", max);
          }
          else
          {
            std::snprintf(buf, size, "Domain is any number");
          }

          break;
        }

      case DVAR_TYPE_FLOAT_2:
        vector(2);
        break;

      case DVAR_TYPE_FLOAT_3:
      case DVAR_TYPE_FLOAT_3_COLOR:
        vector(3);
        break;

      case DVAR_TYPE_FLOAT_4:
        vector(4);
        break;

      case DVAR_TYPE_INT:
        {
          const int min(domain.integer.min);
          const int max(domain.integer.max);

          if (min != std::numeric_limits<int>::min() && max != std::numeric_limits<int>::max())
          {
            std::snprintf(buf, size, "Domain is any integer from %i to %i", min, max);
          }
          else if (min != std::numeric_limits<int>::min())
          {
            std::snprintf(buf, size, "Domain is any integer %i or bigger", min);
          }
          else if (max != std::numeric_limits<int>::max())
          {
            std::snprintf(buf, size, "Domain is any integer %i or smaller", max);
          }
          else
          {
            std::snprintf(buf, size, "Domain is any integer");
          }

          break;
        }

      case DVAR_TYPE_ENUM:
        {
          int n(std::snprintf(buf, size, "Domain is one of the following:"));

          if (n < 0)
          {
            break;
          }

          char* p(buf + n);

          for (int i(0); i < domain.enumeration.stringCount; ++i)
          {
            n = std::snprintf(p, static_cast<std::size_t>(buf + bufSize - p), "\n  %2i: %s", i, domain.enumeration.strings[i]);

            if (n < 0)
            {
              break;
            }

            ++*outLineCount;
            p += n;
          }

          break;
        }

      case DVAR_TYPE_STRING:
        std::snprintf(buf, size, "Domain is any text");
        break;

      case DVAR_TYPE_COLOR:
        std::snprintf(buf, size, "Domain is any 4-component color, in RGBA format");
        break;

      default:
        buf[0] = '\0';
        break;
      }

      buf[bufSize - 1] = '\0';
      return buf;
    }

    // Copy name to buf, cutting off its extension.
    //
    void Com_StripExtension(const char* in, char* out)
    {
      const char* dot(nullptr);

      for (const char* p(in); *p != '\0'; ++p)
      {
        if (*p == '.')
        {
          dot = p;
        }
        else if (*p == '/' || *p == '\\')
        {
          dot = nullptr;
        }
      }

      const char* end(dot != nullptr ? dot : in + std::strlen(in));

      while (in != end)
      {
        *out++ = *in++;
      }

      *out = '\0';
    }

    // List the argument choices that start with the first argument under
    // the input line.
    //
    void ConDrawInput_AutoCompleteArg(const char** stringList, int stringCount)
    {
      const char* arg(Cmd_Argc() > 1 ? Cmd_Argv(1) : "");
      const int argLen(static_cast<int>(std::strlen(arg)));

      if (argLen == 0)
      {
        return;
      }

      char text[1024];
      const char* matches[CON_MAX_SHOWN_ARGS];
      int matchCount(0);
      int textLen(0);
      int widestMatch(0);

      for (int i(0); i < stringCount; ++i)
      {
        if (I_strnicmp(arg, stringList[i], argLen) != 0)
        {
          continue;
        }

        if (matchCount == CON_MAX_SHOWN_ARGS || std::strlen(stringList[i]) + textLen >= sizeof(text))
        {
          return;
        }

        char* match(&text[textLen]);
        Com_StripExtension(stringList[i], match);
        matches[matchCount] = match;
        textLen += static_cast<int>(std::strlen(match)) + 1;

        widestMatch = std::max(widestMatch, Con_TextWidth(match, 0, cls->consoleFont));
        ++matchCount;
      }

      if (matchCount == 0)
      {
        return;
      }

      std::qsort(matches, matchCount, sizeof(matches[0]), [](const void* a, const void* b)
      {
        return I_stricmp(*static_cast<const char* const*>(a), *static_cast<const char* const*>(b));
      });

      const float x(static_cast<float>(Con_TextWidth(g_consoleField.buffer, Con_FirstArgumentOffset(), cls->consoleFont) + conDrawInputGlob.leftX - CON_PADDING));
      const float y(con->screenMin[1] + conDrawInputGlob.fontHeight + static_cast<float>(CON_PADDING));
      const float height(static_cast<float>(conDrawInputGlob.fontHeight * matchCount + 2.0 * CON_PADDING));
      const float width(static_cast<float>(widestMatch + 2.0 * CON_PADDING));

      ConDraw_Box(x, y, width, height, (*con_inputHintBoxColor)->current.vector);

      conDrawInputGlob.x = static_cast<float>(x + CON_PADDING);
      conDrawInputGlob.y = static_cast<float>(CON_PADDING + y);
      conDrawInputGlob.leftX = conDrawInputGlob.x;

      for (int i(0); i < matchCount; ++i)
      {
        ConDrawInput_Text(matches[i], colorDomain);
        ConDrawInput_NextLine();
      }
    }

    // List of the files that the command completes its argument from, or
    // nullptr.
    //
    const char** Cmd_GetAutoCompleteFileList(const char* cmdName, int* fileCount, int allocTrackType)
    {
      *fileCount = 0;

      cmd_function_s* cmd(*cmd_functions);

      while (cmd != nullptr && std::strcmp(cmdName, cmd->name) != 0)
      {
        cmd = cmd->next;
      }

      if (cmd == nullptr)
      {
        return nullptr;
      }

      for (const CmdAutoComplete& a: cmdAutoCompletes)
      {
        if (std::strcmp(a.cmdName, cmd->name) == 0)
        {
          return FS_ListFiles(a.dir, a.ext, FS_LIST_PURE_ONLY, fileCount, allocTrackType);
        }
      }

      return nullptr;
    }

    void ConDrawInput_DetailedDvarMatch(const dvar_t* dvar, void*)
    {
      if (!Con_IsAutoCompleteMatch(dvar->name, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen) ||
          (conDrawInputGlob.hasExactMatch && dvar->name[conDrawInputGlob.inputTextLen] != '\0'))
      {
        return;
      }

      const bool hasLatchedValue(!Dvar_ValuesEqual(dvar->type, dvar->current, dvar->latched));

      ConDrawInput_Box(hasLatchedValue ? 3 : 2, (*con_inputHintBoxColor)->current.vector);

      ConDrawInput_TextLimitChars(dvar->name, CON_NAME_CHARS, colorDvarName);
      ConDrawInput_DvarColumn();
      ConDrawInput_TextLimitChars(Dvar_ValueToString(dvar, dvar->current), CON_VALUE_CHARS, colorDvarValue);
      ConDrawInput_NextLine();

      if (hasLatchedValue)
      {
        ConDrawInput_Text("  latched value", colorDvarInactive);
        ConDrawInput_DvarColumn();
        ConDrawInput_TextLimitChars(Dvar_ValueToString(dvar, dvar->latched), CON_VALUE_CHARS, colorDvarInactive);
        ConDrawInput_NextLine();
      }

      ConDrawInput_Text("  default", colorDvarInactive);
      ConDrawInput_DvarColumn();
      ConDrawInput_TextLimitChars(Dvar_ValueToString(dvar, dvar->reset), CON_VALUE_CHARS, colorDvarInactive);
      ConDrawInput_NextLine();
      ConDrawInput_NextLine();

      char domain[1024];
      int domainLineCount;
      Dvar_DomainToString(dvar->type, dvar->domain, domain, sizeof(domain), &domainLineCount);

      // Note that x86 also draws the description above the domain. x64
      // dvars have none.
      //
      ConDrawInput_Box(domainLineCount + 1, (*con_inputHintBoxColor)->current.vector);
      ConDrawInput_Text(domain, colorDomain);
      ConDrawInput_NextLine();

      if (dvar->type == DVAR_TYPE_ENUM && Cmd_Argc() == 2)
      {
        ConDrawInput_AutoCompleteArg(dvar->domain.enumeration.strings, dvar->domain.enumeration.stringCount);
      }
    }

    void ConDrawInput_DetailedCmdMatch(const char* str)
    {
      if (!Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen) ||
          (conDrawInputGlob.hasExactMatch && str[conDrawInputGlob.inputTextLen] != '\0'))
      {
        return;
      }

      ConDrawInput_Box(1, (*con_inputHintBoxColor)->current.vector);
      ConDrawInput_Text(str, colorCmdName);
      ConDrawInput_NextLine();

      if (Cmd_Argc() != 2)
      {
        return;
      }

      int fileCount;
      const char** files(Cmd_GetAutoCompleteFileList(str, &fileCount, ALLOC_TRACK_CONSOLE));

      if (fileCount != 0)
      {
        ConDrawInput_AutoCompleteArg(files, fileCount);
        FS_FreeFileList(files, ALLOC_TRACK_CONSOLE);
      }
    }

    // Field_Draw() with str in place of the field text.
    //
    // Note that x64 inlines it into Field_Draw().
    //
    void Field_DrawTextOverride(int localClientNum, const field_t* edit, int x, int y, int horzAlign, int vertAlign, const char* str, int drawLen, int cursorPos)
    {
      ScreenPlacement* scrPlace(ScrPlace_GetActivePlacement(localClientNum));
      const bool overstrike(playerKeys[localClientNum].overstrikeMode != 0);

      Font_s* font;
      float xScale;
      float yScale;
      int style;

      if (edit->fixedSize)
      {
        font = cls->consoleFont;
        xScale = 1.0f;
        yScale = 1.0f;
        style = 0;
      }
      else
      {
        const float charScale(edit->charHeight / 48.0f);
        font = UI_GetFontHandle(scrPlace, 0, charScale);
        xScale = yScale = R_NormalizedTextScale(font, charScale);

        if (vertAlign == CON_INPUT_ALIGN)
        {
          xScale *= scrPlace->scaleVirtualToReal[0];
          yScale *= scrPlace->scaleVirtualToReal[1];
        }

        style = 3;
      }

      const float scale(Con_GetFontScale(font));
      const int textY(y + static_cast<int>(static_cast<float>(Con_TextHeight(font)) * yScale));

      CL_DrawTextWithCursor(scrPlace, str, drawLen, font, static_cast<float>(x), static_cast<float>(textY), horzAlign, vertAlign, xScale * scale, yScale * scale, colorWhite, style, cursorPos, overstrike ? '_' : '|');
    }

    // Copy src into dest with the first query substring in it highlighted,
    // and a blank after. Return the length of dest.
    //
    int Con_HighlightSubstring(const char* src, const char* query, char* dest)
    {
      const char* match(I_stristr(src, query));

      if (match == nullptr)
      {
        std::strcpy(dest, src);
        return 0;
      }

      int n(static_cast<int>(match - src));
      std::memcpy(dest, src, n);
      std::memcpy(dest + n, "^2", 3);
      n += 2;

      const int queryLen(static_cast<int>(std::strlen(query)));
      std::memcpy(dest + n, match, queryLen);
      n += queryLen;
      std::memcpy(dest + n, "^7", 3);
      n += 2;

      std::strcpy(dest + n, match + queryLen);
      n += static_cast<int>(std::strlen(match + queryLen));

      dest[n++] = ' ';
      dest[n] = '\0';
      return n;
    }

    // Copy src into dest with the characters of query in it highlighted, in
    // order, and a blank after. Return the length of dest.
    //
    int Con_HighlightCharacters(const char* src, const char* query, char* dest)
    {
      bool highlight(false);
      int n(0);
      int c(std::tolower(static_cast<unsigned char>(*query)));

      for (; *src != '\0'; ++src)
      {
        const bool matches(std::tolower(static_cast<unsigned char>(*src)) == c);

        if (matches)
        {
          c = std::tolower(static_cast<unsigned char>(*++query));
        }

        if (matches != highlight)
        {
          highlight = matches;
          std::memcpy(dest + n, highlight ? "^2" : "^7", 2);
          n += 2;
        }

        dest[n++] = *src;
      }

      std::memcpy(dest + n, "^7", 2);
      n += 2;
      dest[n++] = ' ';
      dest[n] = '\0';
      return n;
    }

    void Con_DrawAutoCompleteChoice(int localClientNum, bool isDvarCommand, const char* originalCommand)
    {
      char text[256];
      int prefixLen(0);

      if (isDvarCommand)
      {
        prefixLen = std::sprintf(text, "^2%s ", originalCommand);
      }

      const int cursorPos(((*con_matchPrefixOnly)->current.enabled
                           ? Con_HighlightSubstring(conDrawInputGlob.autoCompleteChoice, conDrawInputGlob.inputText, text + prefixLen)
                           : Con_HighlightCharacters(conDrawInputGlob.autoCompleteChoice, conDrawInputGlob.inputText, text + prefixLen)) + prefixLen);

      Field_DrawTextOverride(localClientNum, &g_consoleField, static_cast<int>(conDrawInputGlob.x), static_cast<int>(conDrawInputGlob.y), CON_INPUT_ALIGN, CON_INPUT_ALIGN, text, SEH_PrintStrlen(text), cursorPos);
    }

    void Con_DrawInputField(int localClientNum)
    {
      Field_Draw(localClientNum, &g_consoleField, static_cast<int>(conDrawInputGlob.x), static_cast<int>(conDrawInputGlob.y), CON_INPUT_ALIGN, CON_INPUT_ALIGN);
    }

    // Count the matches of the input in one pass.
    //
    void Con_CountMatches(bool substring, bool isDvarCommand)
    {
      conDrawInputGlob.hasExactMatch = false;
      conDrawInputGlob.matchCount = 0;
      conMatchSubstring = substring;

      Dvar_ForEach(ConDrawInput_IncrDvarMatchCounter, nullptr);

      if (!isDvarCommand)
      {
        Cmd_ForEach(ConDrawInput_IncrMatchCounter);
      }
    }

    void Con_DrawInput(int localClientNum)
    {
      if (!Key_IsCatcherActive(localClientNum, KEYCATCH_CONSOLE) || !Sys_IsMainThread())
      {
        return;
      }

      conDrawInputGlob.fontHeight = static_cast<float>(Con_TextHeight(cls->consoleFont));
      conDrawInputGlob.x = static_cast<float>(con->screenMin[0] + CON_PADDING);
      conDrawInputGlob.y = static_cast<float>(con->screenMin[1] + CON_PADDING);
      conDrawInputGlob.leftX = conDrawInputGlob.x;

      ConDrawInput_Box(1, (*con_inputBoxColor)->current.vector);
      ConDrawInput_TextAndOver(conPrompt, colorConsole);

      conDrawInputGlob.leftX = conDrawInputGlob.x;
      g_consoleField.widthInPixels = static_cast<int>(con->screenMax[0] - CON_PADDING - conDrawInputGlob.x);

      const int previousInputTextLen(conDrawInputGlob.inputTextLen);
      const char* originalCommand(Con_TokenizeInput());

      conDrawInputGlob.inputText = originalCommand;
      conDrawInputGlob.inputTextLen = static_cast<int>(std::strlen(originalCommand));
      conDrawInputGlob.autoCompleteChoice[0] = '\0';

      if (previousInputTextLen != conDrawInputGlob.inputTextLen)
      {
        Con_CancelAutoComplete();
      }

      // Hint the dvar names to the dvar commands.
      //
      bool isDvarCommand(false);

      if (conDrawInputGlob.inputTextLen != 0 && Cmd_Argc() > 1 && Con_IsDvarCommand(originalCommand))
      {
        isDvarCommand = true;
        conDrawInputGlob.inputText = Cmd_Argv(1);
        conDrawInputGlob.inputTextLen = static_cast<int>(std::strlen(conDrawInputGlob.inputText));
      }

      if (conDrawInputGlob.inputTextLen == 0)
      {
        conDrawInputGlob.mayAutoComplete = false;
        Con_DrawInputField(localClientNum);
        Cmd_EndTokenizedString();
        return;
      }

      // With con_matchPrefixOnly, take the substring matches, then the
      // prefix ones if there are too many, then the substring ones again
      // if there are none.
      //
      // Note that IW4x leaves out the commands from the prefix pass after a
      // dvar command, as from the others.
      //
      if (!(*con_matchPrefixOnly)->current.enabled)
      {
        Con_CountMatches(false, isDvarCommand);
      }
      else
      {
        Con_CountMatches(true, isDvarCommand);

        if (conDrawInputGlob.matchCount > CON_MAX_SHOWN_MATCHES)
        {
          Con_CountMatches(false, isDvarCommand);

          if (conDrawInputGlob.matchCount == 0)
          {
            Con_CountMatches(true, isDvarCommand);
          }
        }
      }

      if (conDrawInputGlob.matchCount == 0)
      {
        Con_DrawInputField(localClientNum);
        Cmd_EndTokenizedString();
        return;
      }

      if (conDrawInputGlob.matchIndex < conDrawInputGlob.matchCount && conDrawInputGlob.autoCompleteChoice[0] != '\0')
      {
        if (conDrawInputGlob.matchIndex >= 0)
        {
          Con_DrawAutoCompleteChoice(localClientNum, isDvarCommand, originalCommand);
        }
        else
        {
          Con_DrawInputField(localClientNum);
        }
      }
      else
      {
        conDrawInputGlob.matchIndex = -1;
        Con_DrawInputField(localClientNum);
      }

      ConDrawInput_NextLine();
      ConDrawInput_NextLine();

      if (conDrawInputGlob.matchCount > CON_MAX_SHOWN_MATCHES)
      {
        const char* text(va("%i matches (too many to show here, press shift+tilde to open full console)", conDrawInputGlob.matchCount));

        ConDrawInput_Box(1, (*con_inputHintBoxColor)->current.vector);
        ConDrawInput_Text(text, colorDvarName);
      }
      else if (conDrawInputGlob.matchCount == 1 || (conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand()))
      {
        Dvar_ForEach(ConDrawInput_DetailedDvarMatch, nullptr);

        if (!isDvarCommand)
        {
          Cmd_ForEach(ConDrawInput_DetailedCmdMatch);
        }
      }
      else
      {
        ConDrawInput_Box(conDrawInputGlob.matchCount, (*con_inputHintBoxColor)->current.vector);

        Dvar_ForEach(ConDrawInput_DvarMatch, nullptr);

        if (!isDvarCommand)
        {
          Cmd_ForEach(ConDrawInput_CmdMatch);
        }
      }

      Cmd_EndTokenizedString();
    }

    //
    // Output window.
    //

    void SCR_DrawSmallStringExt(int x, int y, const char* string, const float* color)
    {
      Con_DrawText(string, std::numeric_limits<int>::max(), cls->consoleFont, static_cast<float>(x), static_cast<float>(y + Con_TextHeight(cls->consoleFont)), color);
    }

    void Con_DrawOutputScrollBar(float x, float y, float width, float height)
    {
      const float barX(static_cast<float>(x + width - CON_SCROLL_BAR_WIDTH));
      const float barWidth(static_cast<float>(CON_SCROLL_BAR_WIDTH));

      ConDraw_Box(barX, y, barWidth, height, (*con_outputBarColor)->current.vector);

      const int hiddenLineCount(consoleWindow.activeLineCount - con->visibleLineCount);

      if (hiddenLineCount <= 0)
      {
        ConDraw_Box(barX, y, barWidth, height, (*con_outputSliderColor)->current.vector);
        return;
      }

      const float invHiddenLineCount(1.0f / static_cast<float>(hiddenLineCount));
      const float position(std::clamp(static_cast<float>(displayLineOffset - con->visibleLineCount) * invHiddenLineCount, 0.0f, 1.0f));
      const float sliderHeight(std::max(static_cast<float>(std::ceil(static_cast<float>(con->visibleLineCount) * invHiddenLineCount * height)), CON_SLIDER_MIN_HEIGHT));

      ConDraw_Box(barX, y + (height - sliderHeight) * position, barWidth, sliderHeight, (*con_outputSliderColor)->current.vector);
    }

    void Con_DrawOutputText(float x, float y)
    {
      int rowCount(con->visibleLineCount);
      int firstRow(displayLineOffset - rowCount);

      // Start lower when the window doesn't fill the output.
      //
      if (firstRow < 0)
      {
        y -= static_cast<float>(con->fontHeight * firstRow);
        rowCount += firstRow;
        firstRow = 0;
      }

      Font_s* font(cls->consoleFont);
      const float scale(Con_GetFontScale(font));

      for (int row(0); row < rowCount; ++row)
      {
        const MessageLine* line(&consoleWindow.lines[(consoleWindow.firstLineIndex + row + firstRow) % consoleWindow.lineCount]);

        y += static_cast<float>(con->fontHeight);

        R_AddCmdDrawConsoleText(consoleWindow.circularTextBuffer, consoleWindow.textBufSize, line->textBufPos, line->textBufSize, font, x, y, scale, scale, colorWhite, 0);
      }
    }

    void Con_DrawOutputWindow()
    {
      const double growth(Con_FontHeightGrowth());

      float x(con->screenMin[0]);
      float y(static_cast<float>(con->screenMin[1] + (CON_INPUT_BOX_HEIGHT + growth)));
      float width(con->screenMax[0] - con->screenMin[0]);
      float height(static_cast<float>(con->screenMax[1] - con->screenMin[1] - (CON_INPUT_BOX_HEIGHT + growth)));

      ConDraw_Box(x, y, width, height, (*con_outputWindowColor)->current.vector);

      x = static_cast<float>(x + CON_PADDING);
      y = static_cast<float>(y + CON_PADDING);
      height = static_cast<float>(height - 2.0 * CON_PADDING);

      const char* build(va("Build %s %s", getBuildNumber(), "win64"));
      SCR_DrawSmallStringExt(static_cast<int>(x), static_cast<int>(height - (CON_BUILD_STRING_HEIGHT + growth) + y), build, colorConsole);

      Con_DrawOutputScrollBar(x, y, static_cast<float>(width - 2.0 * CON_PADDING), height);
      Con_DrawOutputText(x, y);
    }

    void Con_DrawSolidConsole(int localClientNum)
    {
      Sys_EnterCriticalSection(CRITSECT_CONSOLE);

      if (con->lineOffset != 0)
      {
        Con_UpdateNotifyLine(localClientNum, con->prevChannel, true, 0);
        con->lineOffset = 0;
      }

      Sys_LeaveCriticalSection(CRITSECT_CONSOLE);

      if (!Key_IsCatcherActive(localClientNum, KEYCATCH_CONSOLE))
      {
        outputVisible = false;
      }
      else if (outputVisible)
      {
        Con_DrawOutputWindow();
      }

      Con_DrawInput(localClientNum);
    }

    // Lay the console over the whole screen with a small border, sized for
    // con_scale, while it draws.
    //
    void Con_StoreSafeArea()
    {
      const ScreenPlacement* scrPlace(ScrPlace_GetFullPlacement());
      const float border(static_cast<float>(CON_PADDING));

      con->screenMin[0] = border;
      con->screenMin[1] = border;
      con->screenMax[0] = scrPlace->realViewportSize[0] - border;
      con->screenMax[1] = scrPlace->realViewportSize[1] - border;

      con->fontHeight = Con_TextHeight(cls->consoleFont);
      con->visibleLineCount = static_cast<int>((con->screenMax[1] - con->screenMin[1] - (2 * con->fontHeight) - 24.0) / con->fontHeight);
      con->visiblePixelWidth = static_cast<int>(con->screenMax[0] - con->screenMin[0] - 10.0f - 18.0);
    }

    //
    // Tab completion.
    //

    void Con_FindMatches(const char* s)
    {
      const int completionLen(static_cast<int>(std::strlen(completionString)));

      if (!Con_IsAutoCompleteMatch(s, completionString, completionLen))
      {
        return;
      }

      bool isPrefix(true);

      if (conMatchSubstring || !(*con_matchPrefixOnly)->current.enabled)
      {
        isPrefix = I_strnicmp(s, completionString, completionLen) == 0;
      }

      if (isPrefix)
      {
        ++completionPrefixMatchCount;
      }

      if (++completionMatchCount == 1 || (isPrefix && completionPrefixMatchCount == 1))
      {
        I_strncpyz(shortestMatch, s, sizeof(shortestMatch));
        shortestMatchComplete = true;
        return;
      }

      if (completionPrefixMatchCount == 0)
      {
        I_strncpyz(shortestMatch, completionString, sizeof(shortestMatch));
        return;
      }

      if (!isPrefix)
      {
        return;
      }

      // Shorten the shortest match to the part it shares with s.
      //
      int i(0);

      while (s[i] != '\0' && std::tolower(static_cast<unsigned char>(shortestMatch[i])) == std::tolower(static_cast<unsigned char>(s[i])))
      {
        ++i;
      }

      shortestMatchComplete = s[i] == '\0' || (shortestMatchComplete && shortestMatch[i] == '\0');
      shortestMatch[i] = '\0';
    }

    void Con_FindDvarMatches(const dvar_t* dvar, void*)
    {
      Con_FindMatches(dvar->name);
    }

    // Find the matches of the completion string, setting cmdMatchLen and
    // dvarMatchLen to the shortest match length after the commands and
    // after the dvars.
    //
    void Con_FindAllMatches(bool doCommands, int* cmdMatchLen, int* dvarMatchLen)
    {
      completionMatchCount = 0;
      completionPrefixMatchCount = 0;
      shortestMatch[0] = '\0';

      if (doCommands)
      {
        Cmd_ForEach(Con_FindMatches);
        *cmdMatchLen = static_cast<int>(std::strlen(shortestMatch));
      }
      else
      {
        *cmdMatchLen = 0;
      }

      Dvar_ForEach(Con_FindDvarMatches, nullptr);
      *dvarMatchLen = static_cast<int>(std::strlen(shortestMatch));
    }

    void Con_PrintMatches(const char* s)
    {
      if ((conMatchSubstring || !(*con_matchPrefixOnly)->current.enabled) &&
          I_strnicmp(s, shortestMatch, static_cast<int>(std::strlen(shortestMatch))) != 0)
      {
        return;
      }

      if (I_stristr(s, shortestMatch) != nullptr)
      {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "    %s\n", s);
      }
    }

    void Con_PrintDvarMatches(const dvar_t* dvar, void*)
    {
      Con_PrintMatches(dvar->name);
    }

    // Append the arguments to the input line, quoting the ones with blanks.
    //
    void Con_ConcatRemaining()
    {
      for (int i(1); i < Cmd_Argc(); ++i)
      {
        I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), " ");

        const char* arg(Cmd_Argv(i));
        const bool quote(std::strchr(arg, ' ') != nullptr);

        if (quote)
        {
          I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), "\"");
        }

        I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), arg);

        if (quote)
        {
          I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), "\"");
        }
      }
    }

    // Find the longest start that the list entries beginning with prefix
    // share.
    //
    void Con_ShortestMatchInList(const char** list, int count, const char* prefix, char* dest, int destSize)
    {
      const int prefixLen(static_cast<int>(std::strlen(prefix)));
      dest[0] = '\0';

      for (int i(0); i < count; ++i)
      {
        const char* s(list[i]);

        if (I_strnicmp(prefix, s, prefixLen) != 0)
        {
          continue;
        }

        if (dest[0] == '\0')
        {
          I_strncpyz(dest, s, destSize);
          continue;
        }

        int j(prefixLen);

        while (dest[j] != '\0' && s[j] == dest[j])
        {
          ++j;
        }

        dest[j] = '\0';
      }
    }

    // Replace the last argLen characters of the input line, before its
    // trailing blanks, with completion.
    //
    void Con_ReplaceArgument(const char* completion, int argLen)
    {
      if (completion[0] == '\0')
      {
        return;
      }

      int n(static_cast<int>(std::strlen(g_consoleField.buffer)));

      while (n != 0 && std::isspace(static_cast<unsigned char>(g_consoleField.buffer[n - 1])))
      {
        --n;
      }

      n -= argLen;
      I_strncpyz(&g_consoleField.buffer[n], completion, static_cast<int>(sizeof(g_consoleField.buffer)) - n);
    }

    void Con_CompleteCommandArgument()
    {
      const char* cmd(Con_TokenizeInput());

      if (Cmd_Argc() <= 1 || Cmd_Argv(1)[0] == '\0')
      {
        Cmd_EndTokenizedString();
        return;
      }

      const char* arg(Cmd_Argv(1));

      int fileCount;
      const char** files(Cmd_GetAutoCompleteFileList(cmd, &fileCount, ALLOC_TRACK_CONSOLE));

      Cmd_EndTokenizedString();

      if (fileCount == 0)
      {
        return;
      }

      char completion[256];
      Con_ShortestMatchInList(files, fileCount, arg, completion, sizeof(completion));
      FS_FreeFileList(files, ALLOC_TRACK_CONSOLE);
      Com_StripExtension(completion, completion);

      Con_ReplaceArgument(completion, static_cast<int>(std::strlen(arg)));
    }

    void Con_CompleteDvarArgument()
    {
      const dvar_t* dvar(Dvar_FindVar(Con_TokenizeInput()));

      if (dvar != nullptr && dvar->type == DVAR_TYPE_ENUM && Cmd_Argc() > 1 && Cmd_Argv(1)[0] != '\0')
      {
        const char* arg(Cmd_Argv(1));

        char completion[256];
        Con_ShortestMatchInList(dvar->domain.enumeration.strings, dvar->domain.enumeration.stringCount, arg, completion, sizeof(completion));

        Con_ReplaceArgument(completion, static_cast<int>(std::strlen(arg)));
      }

      Cmd_EndTokenizedString();
    }

    void Con_CompleteCommandLine(ScreenPlacement* scrPlace)
    {
      const char* cmd(Con_TokenizeInput());

      shortestMatch[0] = '\0';
      completionString = cmd;
      completionMatchCount = 0;
      completionPrefixMatchCount = 0;

      if (*cmd == '\0')
      {
        Cmd_EndTokenizedString();
        return;
      }

      // Complete the dvar name after a dvar command.
      //
      bool isDvarCommand(false);

      if (Cmd_Argc() > 1 && Con_IsDvarCommand(cmd))
      {
        isDvarCommand = true;
        completionString = Cmd_Argv(1);
      }

      int cmdMatchLen(0);
      int dvarMatchLen(0);

      if (!(*con_matchPrefixOnly)->current.enabled)
      {
        conMatchSubstring = false;
        Con_FindAllMatches(!isDvarCommand, &cmdMatchLen, &dvarMatchLen);
      }
      else
      {
        conMatchSubstring = true;
        Con_FindAllMatches(!isDvarCommand, &cmdMatchLen, &dvarMatchLen);

        if (completionMatchCount > CON_MAX_SHOWN_MATCHES)
        {
          conMatchSubstring = false;
          Con_FindAllMatches(!isDvarCommand, &cmdMatchLen, &dvarMatchLen);

          if (completionMatchCount == 0)
          {
            conMatchSubstring = true;
            Con_FindAllMatches(!isDvarCommand, &cmdMatchLen, &dvarMatchLen);
          }
        }
      }

      const field_t original(g_consoleField);

      if (completionMatchCount == 0)
      {
        if (!isDvarCommand)
        {
          Cmd_EndTokenizedString();
          return;
        }

        I_strncpyz(shortestMatch, completionString, sizeof(shortestMatch));
      }

      const bool complete(isDvarCommand || completionMatchCount == 1 || (shortestMatchComplete && Con_AnySpaceAfterCommand()));

      if (complete && isDvarCommand)
      {
        Com_sprintf(g_consoleField.buffer, sizeof(g_consoleField.buffer), "\\%s %s", cmd, shortestMatch);
      }
      else
      {
        Com_sprintf(g_consoleField.buffer, sizeof(g_consoleField.buffer), "\\%s", shortestMatch);
      }

      g_consoleField.cursor = static_cast<int>(std::strlen(g_consoleField.buffer));

      // Keep what followed the completed part.
      //
      if (const char* rest = std::strstr(original.buffer, completionString))
      {
        I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), rest + std::strlen(completionString));
      }
      else
      {
        Con_ConcatRemaining();
      }

      if (completionMatchCount == 0)
      {
        g_consoleField.cursor = static_cast<int>(std::strlen(g_consoleField.buffer));
      }
      else if (complete)
      {
        if (!isDvarCommand)
        {
          if (Cmd_Argc() == 1)
          {
            const std::size_t n(std::strlen(g_consoleField.buffer));

            if (n == 0 || g_consoleField.buffer[n - 1] != ' ')
            {
              I_strncat(g_consoleField.buffer, sizeof(g_consoleField.buffer), " ");
            }
          }
          else if (Cmd_Argc() == 2)
          {
            if (cmdMatchLen == dvarMatchLen)
            {
              Con_CompleteCommandArgument();
            }
            else
            {
              Con_CompleteDvarArgument();
            }
          }
        }

        g_consoleField.cursor = static_cast<int>(std::strlen(g_consoleField.buffer));
      }
      else if (conDrawInputGlob.matchCount > CON_MAX_SHOWN_MATCHES)
      {
        // List the matches that the input hint can't.
        //
        Com_Printf(CON_CHANNEL_DONT_FILTER, "]%s\n", g_consoleField.buffer);
        Cmd_ForEach(Con_PrintMatches);
        Dvar_ForEach(Con_PrintDvarMatches, nullptr);
      }

      Cmd_EndTokenizedString();
      Field_AdjustScroll(scrPlace, &g_consoleField);
    }

    //
    // Keys.
    //

    void Con_PageUp()
    {
      displayLineOffset -= 2;

      if (displayLineOffset < con->visibleLineCount)
      {
        displayLineOffset = std::min(consoleWindow.activeLineCount, con->visibleLineCount);
      }
    }

    void Con_PageDown()
    {
      displayLineOffset = std::min(displayLineOffset + 2, consoleWindow.activeLineCount);
    }

    void Con_Top()
    {
      displayLineOffset = std::min(consoleWindow.activeLineCount, con->visibleLineCount);
    }

    void Con_Bottom()
    {
      displayLineOffset = consoleWindow.activeLineCount;
    }

    void Con_ToggleConsoleOutput()
    {
      outputVisible = !outputVisible;
    }

    bool Con_IsHistoryPrevKey(int key, bool shift, bool ctrl)
    {
      if (key == K_MWHEELUP && shift)
      {
        return true;
      }

      return key == K_UPARROW || key == K_KP_UPARROW || (std::tolower(key) == 'p' && ctrl);
    }

    bool Con_IsHistoryNextKey(int key, bool shift, bool ctrl)
    {
      if (key == K_MWHEELDOWN && shift)
      {
        return true;
      }

      return key == K_DOWNARROW || key == K_KP_DOWNARROW || (std::tolower(key) == 'n' && ctrl);
    }

    void Con_ResetInputField()
    {
      Field_Clear(&g_consoleField);
      g_consoleField.charHeight = *g_console_char_height;
      g_consoleField.widthInPixels = *g_console_field_width;
      g_consoleField.fixedSize = 1;
    }

    void Console_EnterKey(int localClientNum)
    {
      if (Con_CommitToAutoComplete())
      {
        return;
      }

      Com_Printf(CON_CHANNEL_DONT_FILTER, "]%s\n", g_consoleField.buffer);

      // Note that IW4x runs the line as a command even without the leading
      // slash, where x86 says it in chat when in game.
      //
      if (g_consoleField.buffer[0] == '\\' || g_consoleField.buffer[0] == '/')
      {
        Cbuf_AddText(localClientNum, &g_consoleField.buffer[1]);
      }
      else if (g_consoleField.buffer[0] == '\0')
      {
        return;
      }
      else
      {
        Cbuf_AddText(localClientNum, g_consoleField.buffer);
      }

      Cbuf_AddText(localClientNum, "\n");

      if (g_consoleField.buffer[0] != '\0')
      {
        historyEditLines[nextHistoryLine % CON_HISTORY_COUNT] = g_consoleField;
        historyLine = ++nextHistoryLine;
      }

      Con_ResetInputField();

      // Show the line now if the frame doesn't redraw the console.
      //
      if (CL_IsNotInGame())
      {
        SCR_UpdateScreen();
      }
    }
  }

  void Con_InitConsoleWindow()
  {
    consoleWindow.lines = consoleLines;
    consoleWindow.messages = consoleMessages;
    consoleWindow.circularTextBuffer = consoleText;
    consoleWindow.textBufSize = CON_TEXT_SIZE;
    consoleWindow.lineCount = CON_LINE_COUNT;
    consoleWindow.padding = 0;
    consoleWindow.scrollTime = 0;
    consoleWindow.fadeIn = 1;
    consoleWindow.fadeOut = 1;
    consoleWindow.textBufPos = 0;
    consoleWindow.firstLineIndex = 0;
    consoleWindow.activeLineCount = 0;
    consoleWindow.messageIndex = 0;
  }

  void Con_InitInput()
  {
    Con_ResetInputField();
    conDrawInputGlob.matchIndex = -1;

    Cmd_AddCommand("clear", Con_Clear_f, &Con_Clear_f_VAR);
  }

  void Con_InitConsoleChannels()
  {
    con_default_console_filter = Dvar_RegisterString("con_default_console_filter", "*", DVAR_NONE, "Default channel filter for the console destination.");
    Con_FilterAdd(con_default_console_filter->current.string, true);

    Cmd_AddCommand("con_channellist", Con_ChannelList_f, &Con_ChannelList_f_VAR);
    Cmd_AddCommand("con_showchannel", Con_ShowChannel_f, &Con_ShowChannel_f_VAR);
    Cmd_AddCommand("con_hidechannel", Con_HideChannel_f, &Con_HideChannel_f_VAR);
    Cmd_AddCommand("con_visiblechannellist", Con_VisibleChannelList_f, &Con_VisibleChannelList_f_VAR);
  }

  bool Con_IsConsoleChannelVisible(int channel, int flags)
  {
    if (con_channels[channel].name[0] == '\0')
    {
      return false;
    }

    if (channel == CON_CHANNEL_DONT_FILTER || Con_IsChannelInFilter(channel))
    {
      return true;
    }

    // Show the errors and warnings of hidden channels if the console shows
    // the error channel.
    //
    const int type((flags >> 5) & 0x1F);
    return (type == 2 || type == 3) && Con_IsChannelInFilter(CON_CHANNEL_ERROR);
  }

  void Con_UpdateConsoleLine(int localClientNum, int channel, bool lineFeed, int flags)
  {
    if (Con_IsConsoleChannelVisible(channel, flags))
    {
      Con_UpdateMessageWindowLine(localClientNum, &consoleWindow, lineFeed, flags);
    }

    // Keep the output at the bottom if it was.
    //
    if (lineFeed && displayLineOffset == consoleWindow.activeLineCount - 1)
    {
      ++displayLineOffset;
    }
  }

  void Con_DrawConsole(int localClientNum)
  {
    Con_CheckResize();

    if (Key_IsCatcherActive(localClientNum, KEYCATCH_CONSOLE))
    {
      const Console layout(*con);

      Con_StoreSafeArea();
      Con_DrawSolidConsole(localClientNum);

      std::memcpy(&con->fontHeight, &layout.fontHeight, sizeof(*con) - offsetof(Console, fontHeight));
    }
  }

  void Con_ToggleConsole()
  {
    Field_Clear(&g_consoleField);
    Con_CancelAutoComplete();

    g_consoleField.fixedSize = 1;
    outputVisible = false;
    g_consoleField.widthInPixels = *g_console_field_width;
    g_consoleField.charHeight = *g_console_char_height;

    clientUIActives[0].keyCatchers ^= KEYCATCH_CONSOLE;
  }

  void Con_EchoCommand()
  {
    Con_ToggleConsole();

    I_strncpyz(g_consoleField.buffer, "\\echo ", sizeof(g_consoleField.buffer));
    g_consoleField.cursor = static_cast<int>(std::strlen(g_consoleField.buffer));
    Field_AdjustScroll(ScrPlace_GetFullPlacement(), &g_consoleField);
  }

  bool Con_CancelAutoComplete()
  {
    if (conDrawInputGlob.matchIndex < 0 || conDrawInputGlob.autoCompleteChoice[0] == '\0')
    {
      return false;
    }

    conDrawInputGlob.matchIndex = -1;
    conDrawInputGlob.autoCompleteChoice[0] = '\0';
    return true;
  }

  bool CL_ConsoleKeyEvent(int localClientNum, int key, int down)
  {
    const PlayerKeyState& keys(playerKeys[localClientNum]);
    const bool consoleOpen((clientUIActives[0].keyCatchers & KEYCATCH_CONSOLE) != 0);

    // With monkeytoy, home and backspace open the console.
    //
    if ((*monkeytoy)->current.enabled && !consoleOpen)
    {
      if (key == K_HOME)
      {
        if (down && keys.keys[K_BACKSPACE].down)
        {
          Con_ToggleConsole();
          return true;
        }

        return false;
      }

      return key == '`' || key == '~';
    }

    if (key != '`' && key != '~')
    {
      return false;
    }

    // Note that IW4x opens the console whatever sv_running and
    // sv_allowClientConsole say.
    //
    if (down)
    {
      if (!keys.keys[K_SHIFT].down)
      {
        Con_ToggleConsole();
      }
      else
      {
        if (!Key_IsCatcherActive(localClientNum, KEYCATCH_CONSOLE))
        {
          Con_ToggleConsole();
        }

        Con_ToggleConsoleOutput();
      }
    }

    return true;
  }

  void Console_Key(int localClientNum, int key)
  {
    const PlayerKeyState& keys(playerKeys[localClientNum]);
    const bool shift(keys.keys[K_SHIFT].down != 0);
    const bool ctrl(keys.keys[K_CTRL].down != 0);
    const bool alt(keys.keys[K_ALT].down != 0);

    const bool tabCompletes(completeOnTab);
    completeOnTab = true;

    ScreenPlacement* scrPlace(ScrPlace_GetFullPlacement());

    if (key == 'l' && ctrl)
    {
      Cbuf_AddText(localClientNum, "clear\n");
      return;
    }

    if (key == K_ENTER || key == K_KP_ENTER)
    {
      Console_EnterKey(localClientNum);
      return;
    }

    if (key == K_TAB)
    {
      if (tabCompletes)
      {
        Con_CompleteCommandLine(scrPlace);
      }
      else
      {
        Con_CycleAutoComplete(shift ? -1 : 1);
      }

      completeOnTab = false;
      return;
    }

    if ((key == K_UPARROW || key == K_DOWNARROW) && ctrl)
    {
      Con_CycleAutoComplete(key == K_UPARROW ? -1 : 1);
      return;
    }

    if (Con_IsHistoryPrevKey(key, shift, ctrl))
    {
      if (nextHistoryLine - historyLine < CON_HISTORY_COUNT && historyLine > 0)
      {
        --historyLine;
      }

      g_consoleField = historyEditLines[historyLine % CON_HISTORY_COUNT];
      Field_AdjustScroll(scrPlace, &g_consoleField);
      conDrawInputGlob.mayAutoComplete = false;
      return;
    }

    if (Con_IsHistoryNextKey(key, shift, ctrl))
    {
      if (Con_CycleAutoComplete(1) || historyLine == nextHistoryLine)
      {
        return;
      }

      ++historyLine;
      g_consoleField = historyEditLines[historyLine % CON_HISTORY_COUNT];
      Field_AdjustScroll(scrPlace, &g_consoleField);
      return;
    }

    switch (key)
    {
    case K_PGUP:
      Con_PageUp();
      return;

    case K_PGDN:
      Con_PageDown();
      return;

    case K_MWHEELUP:
      Con_PageUp();

      if (ctrl)
      {
        Con_PageUp();
        Con_PageUp();
      }

      return;

    case K_MWHEELDOWN:
      Con_PageDown();

      if (ctrl)
      {
        Con_PageDown();
        Con_PageDown();
      }

      return;

    case K_HOME:
    case K_END:
      if (ctrl)
      {
        if (key == K_HOME)
        {
          Con_Top();
        }
        else
        {
          Con_Bottom();
        }

        return;
      }

      if (!alt && !shift)
      {
        Con_CommitToAutoComplete();
      }

      break;

    case K_DEL:
    case K_ESCAPE:
      if (Con_CancelAutoComplete())
      {
        return;
      }

      break;

    case K_RIGHTARROW:
    case K_KP_RIGHTARROW:
    case K_LEFTARROW:
    case K_KP_LEFTARROW:
      Con_CommitToAutoComplete();
      break;

    default:
      if (key != K_BACKSPACE && !ctrl && !alt && !shift)
      {
        Con_CommitToAutoComplete();
      }

      break;
    }

    if (Field_KeyDownEvent(localClientNum, scrPlace, &g_consoleField, key))
    {
      conDrawInputGlob.mayAutoComplete = true;
    }
  }

  void CL_CharEvent(int localClientNum, int key)
  {
    if (key == '`' || key == '~')
    {
      return;
    }

    const int keyCatchers(clientUIActives[0].keyCatchers);

    if ((keyCatchers & KEYCATCH_CONSOLE) == 0)
    {
      if ((keyCatchers & KEYCATCH_CHAT) != 0)
      {
        Field_CharEvent(localClientNum, ScrPlace_GetViewPlacement(localClientNum), &playerKeys[localClientNum].chatField, key);
        return;
      }

      if ((keyCatchers & KEYCATCH_UI) != 0)
      {
        UI_KeyEvent(localClientNum, key | K_CHAR_FLAG, 1);
        return;
      }

      if (clientUIActives[0].connectionState != CA_DISCONNECTED)
      {
        return;
      }
    }
    else if (key == ('h' & 0x1F) && Con_CancelAutoComplete())
    {
      return;
    }

    // Console_CharEvent().
    //
    if (Field_CharEvent(localClientNum, ScrPlace_GetFullPlacement(), &g_consoleField, key))
    {
      conDrawInputGlob.mayAutoComplete = true;
    }
  }

  float Con_GetConsoleScale()
  {
    float scale(con_scale != nullptr ? con_scale->current.value : 0.0f);

    if (scale <= 0.0f)
    {
      scale = ScrPlace_GetFullPlacement()->realViewportSize[1] / 1080.0f;
    }

    return std::clamp(scale, 1.0f, 4.0f);
  }

  float Con_GetFontScale(Font_s* font)
  {
    return font != nullptr && font == cls->consoleFont ? Con_GetConsoleScale() : 1.0f;
  }
}
