#pragma once

namespace Game
{
  // Game functions the in-game console uses.
  //
  typedef void(*Con_CheckResize_t)();
  extern Con_CheckResize_t Con_CheckResize;

  typedef void(*CL_ConsolePrint_t)(int localClientNum, int channel, const char* txt, int duration, int pixelWidth, int flags);
  extern CL_ConsolePrint_t CL_ConsolePrint;

  // Add the pending console line to the game message windows of the
  // channel, ending it if lineFeed is set.
  //
  // Note that x64 leaves the console window out. Components::Console
  // detours it to Con_UpdateNotifyLine_Hk(), which adds it back.
  //
  typedef void(*Con_UpdateNotifyLine_t)(int localClientNum, int channel, bool lineFeed, int flags);
  extern Con_UpdateNotifyLine_t Con_UpdateNotifyLine;

  // Number of letters the typewriter effect prints for the line.
  //
  typedef int(*PrintableCharsCount_t)(const MessageWindow* msgwnd, MessageLine* line);
  extern PrintableCharsCount_t PrintableCharsCount;

  typedef int(*SEH_PrintStrlen_t)(const char* string);
  extern SEH_PrintStrlen_t SEH_PrintStrlen;

  typedef void(*Dvar_ForEach_t)(void(*callback)(const dvar_t* dvar, void* userData), void* userData);
  extern Dvar_ForEach_t Dvar_ForEach;

  typedef const char*(*Dvar_ValueToString_t)(const dvar_t* dvar, DvarValue value);
  extern Dvar_ValueToString_t Dvar_ValueToString;

  typedef int(*Dvar_ValuesEqual_t)(dvar_type type, DvarValue val0, DvarValue val1);
  extern Dvar_ValuesEqual_t Dvar_ValuesEqual;

  typedef int(*I_strnicmp_t)(const char* s0, const char* s1, int n);
  extern I_strnicmp_t I_strnicmp;

  typedef const char*(*I_stristr_t)(const char* s0, const char* substr);
  extern I_stristr_t I_stristr;

  typedef void(*I_strncat_t)(char* dest, int size, const char* src);
  extern I_strncat_t I_strncat;

  // Compare string with filter, which may hold the * and ? wildcards,
  // ignoring case. Return 0 if they match.
  //
  typedef int(*I_strcmpwild_t)(const char* filter, const char* string);
  extern I_strcmpwild_t I_strcmpwild;

  typedef void(*Cmd_TokenizeString_t)(const char* text);
  extern Cmd_TokenizeString_t Cmd_TokenizeString;

  typedef void(*Cmd_EndTokenizedString_t)();
  extern Cmd_EndTokenizedString_t Cmd_EndTokenizedString;

  typedef bool(*Field_KeyDownEvent_t)(int localClientNum, ScreenPlacement* scrPlace, field_t* edit, int key);
  extern Field_KeyDownEvent_t Field_KeyDownEvent;

  typedef bool(*Field_CharEvent_t)(int localClientNum, ScreenPlacement* scrPlace, field_t* edit, int ch);
  extern Field_CharEvent_t Field_CharEvent;

  typedef void(*ScrPlace_EndFrame_t)();
  extern ScrPlace_EndFrame_t ScrPlace_EndFrame;

  typedef void(*SCR_UpdateScreen_t)();
  extern SCR_UpdateScreen_t SCR_UpdateScreen;

  // Whether the local client is out of the game: off the main thread,
  // behind a fullscreen menu, or disconnected.
  //
  typedef bool(*CL_IsNotInGame_t)();
  extern CL_IsNotInGame_t CL_IsNotInGame;

  typedef const char*(*getBuildNumber_t)();
  extern getBuildNumber_t getBuildNumber;

  // Console channel names, indexed by channel.
  //
  struct ConChannelDef
  {
    char name[32];
    bool allowScript;
  };

  static_assert(sizeof(ConChannelDef) == 0x21);

  constexpr int CON_MAX_CHANNELS = 256;

  extern ConChannelDef* con_channels;

  extern Console* con;

  extern const dvar_t** monkeytoy;
  extern const dvar_t** con_matchPrefixOnly;
  extern const dvar_t** con_inputBoxColor;
  extern const dvar_t** con_inputHintBoxColor;
  extern const dvar_t** con_outputBarColor;
  extern const dvar_t** con_outputSliderColor;
  extern const dvar_t** con_outputWindowColor;
  extern const dvar_t** con_typewriterPrintSpeed;
  extern const dvar_t** con_typewriterDecayStartTime;
  extern const dvar_t** con_typewriterDecayDuration;
  extern const dvar_t** com_filter_output;

  // Console input history.
  //
  extern field_t* historyEditLines;

  // Console size multiplier that x86 doesn't have, registered by
  // Components::Console. Zero derives it from the render height.
  //
  extern const dvar_t* con_scale;

  // In-game console that x64 compiles out, rebuilt from x86.
  //
  // Note that this includes the IW4x changes to it: the prompt and colors,
  // the console that always opens and always runs commands, and the
  // con_scale size multiplier.
  //

  // Set up the console window. Called by Con_OneTimeInit().
  //
  void Con_InitConsoleWindow();

  // Set up the console input. Called by Con_Init().
  //
  void Con_InitInput();

  // Register con_default_console_filter and the channel commands, and fill
  // the console filter. Called by Con_InitChannels().
  //
  void Con_InitConsoleChannels();

  // Whether the console shows the channel with the print flags.
  //
  bool Con_IsConsoleChannelVisible(int channel, int flags);

  // Add the pending console line to the console window, ending it if
  // lineFeed is set.
  //
  void Con_UpdateConsoleLine(int localClientNum, int channel, bool lineFeed, int flags);

  // Draw the console if it is open, sizing it first.
  //
  void Con_DrawConsole(int localClientNum);

  // Open or close the console with a clean input line.
  //
  void Con_ToggleConsole();

  // Cancel the autocomplete choice. Return true if there was one.
  //
  bool Con_CancelAutoComplete();

  // Handle the console part of CL_KeyEvent(): the toggle keys. Return true
  // if the key is taken.
  //
  bool CL_ConsoleKeyEvent(int localClientNum, int key, int down);

  // Open the console with an echo command on the input line.
  //
  void Con_EchoCommand();

  // Handle a key press while the console is open.
  //
  void Console_Key(int localClientNum, int key);

  // Handle a character while the console takes the input.
  //
  void CL_CharEvent(int localClientNum, int key);

  // Size multiplier of the console font, and of font if it is the console
  // font.
  //
  float Con_GetConsoleScale();
  float Con_GetFontScale(Font_s* font);
}
