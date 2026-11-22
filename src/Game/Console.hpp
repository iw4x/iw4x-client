#pragma once

namespace Game
{
  // The game functions that the in-game console uses.
  //
  typedef void(*Con_CheckResize_t)();
  extern Con_CheckResize_t Con_CheckResize;

  typedef void(*CL_ConsolePrint_t)(int localClientNum, int channel, const char* txt, int duration, int pixelWidth, int flags);
  extern CL_ConsolePrint_t CL_ConsolePrint;

  // Add the pending console line to the game message windows of the
  // channel, ending it if lineFeed is set.
  //
  // Note that x64 leaves the console window out, which
  // Components::Console adds back by detouring this to
  // Con_UpdateNotifyLine_Hk().
  //
  typedef void(*Con_UpdateNotifyLine_t)(int localClientNum, int channel, bool lineFeed, int flags);
  extern Con_UpdateNotifyLine_t Con_UpdateNotifyLine;

  // Return the number of letters that the typewriter effect prints for the
  // line.
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

  // Compare the string with the filter, which may hold the * and ?
  // wildcards, ignoring case. Return 0 if they match.
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

  // Return true if the local client is out of the game, that is, off the
  // main thread, behind a fullscreen menu, or disconnected.
  //
  typedef bool(*CL_IsNotInGame_t)();
  extern CL_IsNotInGame_t CL_IsNotInGame;

  typedef const char*(*getBuildNumber_t)();
  extern getBuildNumber_t getBuildNumber;

  // The console channel names, indexed by channel.
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

  // The console input history.
  //
  extern field_t* historyEditLines;

  // The console size multiplier, which x86 doesn't have and which
  // Components::Console registers. Zero derives it from the render height.
  //
  extern const dvar_t* con_scale;

  // The in-game console, that is, the one that opens with the tilde key.
  //
  // On x86 this console is simply part of the game. CL_ConsolePrint()
  // breaks whatever gets printed into lines, and each finished line goes
  // into the console window, which is a message window of 1024 lines like
  // the ones the game shows its own messages in. Con_DrawConsole() then
  // draws that window every frame along with the input line and its hint,
  // while CL_KeyEvent() toggles the console and, as long as it's open,
  // hands it every key.
  //
  // x64 keeps surprisingly little of that. CL_ConsolePrint() is still
  // there, and so is the buffer it builds the current line in. The
  // console channels and dvars survived as well, along with the input
  // history and the field editing that the input line goes through
  // (presumably because chat still needs it). But the console window
  // itself is gone, along with everything that draws it, and the tab
  // completion went with it. CL_KeyEvent() even still checks the bit
  // that says the console is open, and drops the key if it's set, but
  // nothing is left to take the key instead. The mini console and the
  // error window went the same way, con_minicon included. What we are
  // left with is the plumbing without a console at the end of it.
  //
  // So what can we do about it? We could settle for the external console
  // window, which x64 still has (see Components::Console). But that's not
  // what anyone opens in the middle of a match, and plenty of players use
  // the in-game console all the time. So instead we rebuild x86's console
  // on top of what x64 kept, which is what the rest of this header (and
  // Console.cpp) is about. It's a port rather than a rewrite, function for
  // function and with the x86 names, so that the two stay easy to compare.
  // Which also means that if something behaves differently from x86, our
  // port is probably the first place to look.
  //
  // Most of the console then lives on its own. Its windows and their text
  // are in our memory, it registers its channel commands itself, and it
  // draws with the renderer's text and material calls the way x86 does.
  // Where it does touch the game is the places that x86's console is
  // called from, and Components::Console hooks those for it.
  //
  // The lines come in through Con_UpdateNotifyLine(), which on x64 only
  // updates the game message windows. Components::Console detours it so
  // that a line goes to our console window first and then to the mini
  // console. After that it goes to the game's windows and finally to the
  // error window, which is the order x86 updates them in. Note that x64
  // also inlines a copy of that loop into CL_ConsolePrint() for the line
  // that ends a print, which would go right past our detour. So there we
  // jump to a call of the real function instead.
  //
  // The keys come in through CL_KeyEvent() and CL_CharEvent(). Into the
  // former we jump at the four points where x86 calls the console: the
  // toggle keys come first, then escape, which cancels the autocomplete,
  // and then any key pressed while the console is open or while we are
  // disconnected. The latter is small enough that we replace it
  // altogether. The console itself is drawn from the renderer pipeline of
  // Components::Scheduler, right after the frame's screen placement ends.
  //
  // Note also that the console comes with the IW4x changes that x86 makes
  // to it. The prompt and the colors are ours, the console opens whatever
  // the server says, and it runs a line as a command even without the
  // leading slash. On top of that, con_scale sizes it for resolutions that
  // the game never had in mind. Like x86 we only keep one console, for the
  // first local client.
  //

  // Set up the console window. Called by Con_OneTimeInit().
  //
  void Con_InitConsoleWindow();

  // Set up the console input. Called by Con_Init().
  //
  void Con_InitInput();

  // Register con_default_console_filter along with the channel commands,
  // and fill the console filter. Called by Con_InitChannels().
  //
  void Con_InitConsoleChannels();

  // Return true if the console shows the channel with these print flags.
  //
  bool Con_IsConsoleChannelVisible(int channel, int flags);

  // Add the pending console line to the console window, ending it if
  // lineFeed is set.
  //
  void Con_UpdateConsoleLine(int localClientNum, int channel, bool lineFeed, int flags);

  // The mini console, registered by Components::QuickPatch.
  //
  // Note that x64 compiles out the mini console and the error windows
  // along with con_minicon. So we keep them here with the console window,
  // and the components feed them through the hooks below. Like x86 we keep
  // one of each, for the first local client.
  //
  extern const dvar_t* con_minicon;

  // Set up the mini console and error windows. Called after
  // Con_InitMessageBuffer().
  //
  void Con_InitMessageWindows();

  // Clear the mini console and error windows. Called after
  // Con_ClearNotify().
  //
  void Con_ClearMessageWindows(int localClientNum);

  // Start the message of a print in the mini console and error windows if
  // they show its channel. Called before CL_ConsolePrint_AddLine(), unless
  // it's already printing.
  //
  void Con_UpdateMessageWindowsMessage(int localClientNum, int channel, int duration, int flags);

  // Add the pending console line to the mini console and error windows if
  // they show its channel. Called around Con_UpdateNotifyLine() the way x86
  // does it, the mini console before the game message windows and the
  // errors after them.
  //
  void Con_UpdateMiniconLine(int localClientNum, int channel, bool lineFeed, int flags);
  void Con_UpdateErrorLine(int localClientNum, int channel, bool lineFeed, int flags);

  // Move or shift the messages of the console windows (the mini console
  // and error ones included). Called after Con_TimeJumped() and
  // Con_TimeNudged().
  //
  void Con_TimeJumpedMessageWindows(int localClientNum, int serverTime);
  void Con_TimeNudgedMessageWindows(int localClientNum, int serverTimeNudge);

  // Draw the mini console if con_minicon is set, and the error window.
  //
  void Con_DrawMiniConsole(int localClientNum, int xPos, int yPos, float alpha);
  void Con_DrawErrors(int localClientNum, int xPos, int yPos, float alpha);

  // Draw the console if it's open, sizing it first.
  //
  void Con_DrawConsole(int localClientNum);

  // Open or close the console with a clean input line.
  //
  void Con_ToggleConsole();

  // Cancel the autocomplete choice. Return true if there was one.
  //
  bool Con_CancelAutoComplete();

  // Handle the console part of CL_KeyEvent(), that is, the toggle keys.
  // Return true if we took the key.
  //
  bool CL_ConsoleKeyEvent(int localClientNum, int key, int down);

  // Open the console with an echo command on the input line.
  //
  void Con_EchoCommand();

  // Handle a key press while the console is open.
  //
  void Console_Key(int localClientNum, int key);

  // Hooks into our console for the components, which stay unset unless a
  // component sets them.
  //
  // Con_DrawInputOverlay draws over the input line once the console has
  // drawn it, given the input field and the left edge of its text.
  // Console_KeyFilter takes a key for the input field before the console
  // handles it, returning true if it does.
  //
  // Note that on x86 the components hook the game's calls of
  // Con_DrawInput() and Console_Key() instead.
  //
  extern void (*Con_DrawInputOverlay)(int localClientNum, field_t* field, float x);
  extern bool (*Console_KeyFilter)(int localClientNum, int key, field_t* field);

  // Handle a character while the console takes the input.
  //
  void CL_CharEvent(int localClientNum, int key);

  // Return the size multiplier of the console font, which also applies to
  // the font if it's the console font.
  //
  float Con_GetConsoleScale();
  float Con_GetFontScale(Font_s* font);
}
