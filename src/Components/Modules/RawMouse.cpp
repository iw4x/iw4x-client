#include "RawMouse.hpp"

#include "Gamepad.hpp"
#include "Window.hpp"

namespace Components
{
  // Engine specific constants.
  //
  constexpr int mw_up = 205;
  constexpr int mw_down = 206;

  void
  rawMouseValue_t::ResetDelta ()
  {
    // Snapshot the current total so next GetDelta() returns 0.
    //
    this->previous = this->current;
  }

  int
  rawMouseValue_t::GetDelta () const
  {
    return this->current - this->previous;
  }

  void
  rawMouseValue_t::Update (int v, bool a)
  {
    // If the device reports absolute positioning), we reset our accumulator.
    // The incoming value isn't a delta in this case, it's the new coordinate,
    // and if we treated it as a delta, the view would spin uncontrollably
    // because the coordinates are usually large.
    //
    if (a)
      this->current = 0;

    this->current += v;
  }

  // Static state initialization.
  //
  Dvar::Var RawMouse::M_RawInput;
  Dvar::Var RawMouse::M_RawInputVerbose;
  Dvar::Var RawMouse::R_AutoPriority = nullptr;
  Dvar::Var RawMouse::R_FullScreen = nullptr;

  rawMouseValue_t RawMouse::MouseRawX {0, 0};
  rawMouseValue_t RawMouse::MouseRawY {0, 0};
  uint32_t RawMouse::MouseRawEvents = 0;

  bool RawMouse::InRawInput = false;
  bool RawMouse::InFocus = false;
  bool RawMouse::FirstRawInputUpdate = true;

  // We need to keep the OS cursor confined to the window rect. If we don't,
  // clicks on the edge might register outside the context (losing focus) or
  // trigger window resizing.
  //
  void
  ClampMousePos (POINT& p)
  {
    tagRECT rc;
    if (GetWindowRect (Window::GetWindow (), &rc) != TRUE)
      return;

    auto c (false);

    // X-axis.
    //
    if (p.x >= rc.left)
    {
      if (p.x >= rc.right)
      {
        p.x = rc.right - 1;
        c = true;
      }
    }
    else
    {
      p.x = rc.left;
      c = true;
    }

    // Y-axis.
    //
    if (p.y >= rc.top)
    {
      if (p.y >= rc.bottom)
      {
        p.y = rc.bottom - 1;
        c = true;
      }
    }
    else
    {
      p.y = rc.top;
      c = true;
    }

    // Only talk to the OS if we actually modified the coordinates to avoid
    // unnecessary IPC overhead/context switches.
    //
    if (c)
      SetCursorPos (p.x, p.y);
  }

  void
  RawMouse::IN_ClampMouseMove ()
  {
    tagPOINT p;
    GetCursorPos (&p);
    ClampMousePos (p);
  }

  bool
  CheckButtonFlag (DWORD f, DWORD m)
  {
    return (f & m) != 0u;
  }

  void
  RawMouse::ResetMouseRawEvents ()
  {
    // We used to try force-releasing buttons here during alt-tab to prevent
    // "stuck" firing, but that logic was flaky. Now we just zero out the
    // event state and reset the update flag so the delta calculation doesn't
    // snap angles on refocus.
    //
    MouseRawEvents = 0u;
    FirstRawInputUpdate = true;
  }

  // Translates raw input flags into our internal bitmask for button states.
  // We have to be careful about matching press/release pairs to avoid logical
  // desyncs where the game thinks a key is held down forever.
  //
  void
  RawMouse::ProcessMouseRawEvent (DWORD f, DWORD d, DWORD e)
  {
    const uint32_t p (MouseRawEvents);

    // Down.
    //
    if (CheckButtonFlag (f, d))
    {
      if (M_RawInputVerbose.get<bool> ())
      {
        if ((p & e) != 0u)
          Logger::Debug ("Pressing button that wasn't released");

        Logger::Debug ("Mouse button down: [{}, {}]", e, p);
      }

      MouseRawEvents |= e;
    }

    // Up (shifted flag).
    //
    if (CheckButtonFlag (f, d << 1u))
    {
      // Protection against the "Alt-Tab Ghost Release" scenario. Sometimes
      // Windows sends a release event for a button we never saw getting
      // pressed (because we weren't focused). Ignore those or the game state
      // might get corrupted.
      //
      if ((p & e) == 0u)
      {
        if (M_RawInputVerbose.get<bool> ())
          Logger::Debug ("!! Releasing button that wasn't pressed");

        return;
      }

      if (M_RawInputVerbose.get<bool> ())
        Logger::Debug ("Mouse button up: [{}, {}]", e, p);

      MouseRawEvents &= ~e;
    }
  }

  bool
  RawMouse::GetRawInput (LPARAM l, RAWINPUT& r, UINT& s)
  {
    const UINT res (GetRawInputData (reinterpret_cast<HRAWINPUT> (l),
                                     RID_INPUT,
                                     &r,
                                     &s,
                                     sizeof (RAWINPUTHEADER)));

    if (res == static_cast<UINT> (-1) || r.header.dwType != RIM_TYPEMOUSE)
      return false;

    return true;
  }

  BOOL
  RawMouse::OnRawInput (LPARAM l, WPARAM)
  {
    // If the dvar is disabled, we still receive the message but should ignore
    // it. Also, reset events to not conflict with legacy handling if we
    // switched modes at runtime.
    //
    if (!InRawInput)
    {
      ResetMouseRawEvents ();
      return TRUE;
    }

    UINT s (sizeof (RAWINPUT));
    static RAWINPUT r;

    if (!GetRawInput (l, r, s))
      return TRUE;

    // Ignore background input to prevent shooting while typing in another
    // window.
    //
    if (!RawMouse::InFocus)
      return TRUE;

    // Does absolute mouse movement actually exist in the wild for gaming
    // mice? Probably not, but the spec says yes, so we handle the flag.
    //
    const bool a ((r.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0u);

    MouseRawX.Update (r.data.mouse.lLastX, a);
    MouseRawY.Update (r.data.mouse.lLastY, a);

    // Fix for the violent angle snap that happens when alt-tabbing back in.
    // The first update usually contains a massive delta from the cursor
    // moving across the screen while we were backgrounded.
    //
    if (FirstRawInputUpdate)
    {
      MouseRawX.ResetDelta ();
      MouseRawY.ResetDelta ();
      FirstRawInputUpdate = false;
    }

    // Map the platform specific RI flags to our internal engine indices.
    // 1=LMB, 2=RMB, 4=MMB, etc.
    //
    ProcessMouseRawEvent (r.data.mouse.usButtonFlags, RI_MOUSE_BUTTON_1_DOWN, 1);
    ProcessMouseRawEvent (r.data.mouse.usButtonFlags, RI_MOUSE_BUTTON_2_DOWN, 2);
    ProcessMouseRawEvent (r.data.mouse.usButtonFlags, RI_MOUSE_BUTTON_3_DOWN, 4);
    ProcessMouseRawEvent (r.data.mouse.usButtonFlags, RI_MOUSE_BUTTON_4_DOWN, 8);
    ProcessMouseRawEvent (r.data.mouse.usButtonFlags, RI_MOUSE_BUTTON_5_DOWN, 16);

    Game::IN_MouseEvent (MouseRawEvents);

    // Handle scroll wheel separately as it's not a boolean state but a value.
    // We map positive/negative deltas to virtual key events.
    //
    if (r.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
    {
      const SHORT d (static_cast<SHORT> (r.data.mouse.usButtonData));

      if (d > 0)
        Game::Sys_QueEvents (Game::g_wv->sysMsgTime, 1, mw_down, 0, 0);
      if (d < 0)
        Game::Sys_QueEvents (Game::g_wv->sysMsgTime, 1, mw_up, 0, 0);
    }

    return TRUE;
  }

  bool
  RawMouse::IsMouseInClientBounds ()
  {
    POINT p;
    GetCursorPos (&p);
    ScreenToClient (Window::GetWindow (), &p);

    RECT rc;
    Window::Dimension (Window::GetWindow (), &rc);

    return (p.y >= 0 && p.x >= 0 && (rc.right - rc.left) >= p.x &&
            (rc.bottom - rc.top) >= p.y);
  }

  // Fallback handler for standard Windows mouse messages.
  //
  BOOL
  RawMouse::OnLegacyMouseEvent (UINT m, LPARAM l, WPARAM w)
  {
    int e ((w & MK_LBUTTON) != 0);

    if ((w & MK_RBUTTON) != 0)  e |= 2u;
    if ((w & MK_MBUTTON) != 0)  e |= 4u;
    if ((w & MK_XBUTTON1) != 0) e |= 8u;
    if ((w & MK_XBUTTON2) != 0) e |= 0x10u;

    // If raw input is active, we generally ignore legacy messages to avoid
    // double inputs, but we still track them for debugging or if we lose
    // focus.
    //
    if (M_RawInput.get<bool> ())
    {
      if (e == 0 || !RawMouse::InFocus)
        return FALSE;

      if (M_RawInputVerbose.get<bool> ())
        Logger::Debug ("Window Mouse Message: [{}, {}]", e, MouseRawEvents);

      MouseRawEvents = e;
    }

    Game::IN_MouseEvent (e);

    // We have to call the default proc here because the game expects certain
    // window behaviors (like drag/move) if we aren't trapping input.
    //
    return DefWindowProcA (Window::GetWindow (), m, w, l);
  }

  BOOL
  RawMouse::OnKillFocus ([[maybe_unused]] LPARAM l, WPARAM)
  {
    RawMouse::InFocus = false;

    // When losing focus, we must release the raw input device. If we don't,
    // we might "steal" the mouse from other applications or the desktop.
    //
    ToggleRawInput (false);
    ResetMouseRawEvents ();

    // Clear all key and button states to prevent stuck inputs (firing, ADS,
    // etc.) when alt-tabbing while holding mouse buttons. We uses the
    // engine's own state clearing mechanism here so let's pray that it
    // properly releases both PlayerKeyState and kbutton_t states.
    //
    Game::Key_ClearStates (0);

    // Drop priority to save CPU when we aren't the active window.
    //
    if (R_AutoPriority.get<Game::dvar_t*> () && R_AutoPriority.get<bool> ())
      SetPriorityClass (GetCurrentProcess (), IDLE_PRIORITY_CLASS);

    return DefWindowProc (Window::GetWindow (), WM_KILLFOCUS, 0, 0);
  }

  BOOL
  RawMouse::OnSetFocus ([[maybe_unused]] LPARAM l, WPARAM)
  {
    RawMouse::InFocus = true;

    // Restore priority when we become the active window.
    //
    if (R_AutoPriority.get<Game::dvar_t*> () && R_AutoPriority.get<bool> ())
      SetPriorityClass (GetCurrentProcess (), HIGH_PRIORITY_CLASS);

    return DefWindowProc (Window::GetWindow (), WM_SETFOCUS, 0, 0);
  }

  // The actual movement logic when Raw Input is active.
  //
  void
  RawMouse::IN_RawMouseMove ()
  {
    auto dx (MouseRawX.GetDelta ());
    auto dy (MouseRawY.GetDelta ());

    // Reset accumulators immediately so we don't process the same movement
    // twice if the next frame comes in fast.
    //
    MouseRawX.ResetDelta ();
    MouseRawY.ResetDelta ();

    // Even with raw input, the game menu logic relies on client coordinates
    // for UI interaction (hovering buttons). We grab the cursor, convert, and
    // store it for the UI system.
    //
    tagPOINT p;
    GetCursorPos (&p);
    Game::s_wmv->oldPos = p;
    ScreenToClient (Window::GetWindow (), &p);

    Gamepad::OnMouseMove (p.x, p.y, dx, dy);

    // CL_MouseEvent returns false if we are in a state where the mouse should
    // float freely (e.g., menu). If true, it means we are in-game and looking
    // around, so we need to lock/clip the cursor to the window.
    //
    if (!Game::CL_MouseEvent (p.x, p.y, dx, dy))
    {
      ClipCursor (NULL);
      return;
    }

    // Force the cursor back to the center if we are in FPS mode to never hit
    // the screen edge.
    //
    RECT rc;
    if (GetWindowRect (Window::GetWindow (), &rc) == TRUE)
    {
      RawMouse::IN_RecenterMouse ();
    }
  }

  bool
  RawMouse::ToggleRawInput (bool e)
  {
    // If the Dvar is off, force disable regardless of requested state. We
    // don't want to enable raw input if the user explicitly turned it off in
    // the config.
    //
    if (!M_RawInput.get<bool> ())
    {
      if (!InRawInput)
        return false;

      e = false;
    }
    else
    {
      // No change needed.
      //
      if (InRawInput == e)
        return InRawInput;
    }

    constexpr DWORD f (RIDEV_INPUTSINK | RIDEV_NOLEGACY);

    RAWINPUTDEVICE rid [1];
    rid [0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid [0].usUsage = HID_USAGE_GENERIC_MOUSE;
    rid [0].dwFlags = e ? f : RIDEV_REMOVE;
    rid [0].hwndTarget = e ? Window::GetWindow () : NULL;

    bool ok (RegisterRawInputDevices (rid, ARRAYSIZE (rid), sizeof (rid [0])) ==
             TRUE);

    if (!ok)
    {
      Logger::Warning (Game::CON_CHANNEL_SYSTEM,
                       "RawInputDevices: failed: {}\n",
                       GetLastError ());
    }
    else
    {
      InRawInput = (rid [0].dwFlags & RIDEV_REMOVE) == 0u;

      if (M_RawInputVerbose.get<bool> ())
      {
        if (InRawInput)
          Logger::Debug ("Raw Input enabled");
        else
          Logger::Debug ("Raw Input disabled");
      }

      // If we just disabled it, clear any lingering events so they don't fire
      // when we re-enable later.
      //
      if (!InRawInput)
        ResetMouseRawEvents ();
    }

    return true;
  }

  void
  RawMouse::IN_RawMouse_Init ()
  {
    if (Window::GetWindow () && ToggleRawInput (true))
    {
      Logger::Debug ("Raw Mouse Init");
    }
  }

  void
  RawMouse::IN_Init ()
  {
    Game::IN_Init ();
    IN_RawMouse_Init ();
    ResetMouseRawEvents ();

    R_AutoPriority = Dvar::Var ("r_autopriority");
    R_FullScreen = Dvar::Var (0x069F0DA0);
  }

  void
  RawMouse::IN_Frame ()
  {
    // Only toggle raw input on if the mouse is actually inside our window,
    // otherwise we steal input from the rest of the OS.
    //
    if (RawMouse::InFocus)
      ToggleRawInput (IsMouseInClientBounds ());

    return Game::IN_Frame ();
  }

  BOOL
  RawMouse::IN_ClipCursor ()
  {
    RECT rc;
    if (!GetClientRect (Window::GetWindow (), &rc))
    {
      return FALSE;
    }

    // Convert client area to screen coordinates because ClipCursor expects
    // global screen positions, not window-relative ones.
    //
    ClientToScreen (Window::GetWindow (), std::bit_cast<POINT*> (&rc.left));
    ClientToScreen (Window::GetWindow (), std::bit_cast<POINT*> (&rc.right));
    return ClipCursor (&rc);
  }

  BOOL
  RawMouse::IN_RecenterMouse ()
  {
    IN_ClipCursor ();
    return Game::IN_RecenterMouse ();
  }

  // The main mouse entry point hooked from the engine. Decides whether to use
  // our Raw implementation or legacy behavior based on focus and dvar
  // settings.
  //
  void
  RawMouse::IN_MouseMove ()
  {
    if (!RawMouse::InFocus)
    {
      ClipCursor (NULL);
      return;
    }

    if (InRawInput)
    {
      return IN_RawMouseMove ();
    }

    // Legacy path below.
    //
    if (GetForegroundWindow () != Window::GetWindow ())
      return;

    tagPOINT c;
    static tagPOINT p;

    GetCursorPos (&c);
    if (R_FullScreen.get<Game::dvar_t*> () && R_FullScreen.get<bool> ())
      ClampMousePos (c);

    int dx (c.x - p.x);
    int dy (c.y - p.y);
    p = c;

    ScreenToClient (Window::GetWindow (), &c);
    auto recenter (Game::CL_MouseEvent (c.x, c.y, dx, dy));

    if (recenter && (dx || dy))
    {
      RECT rc;
      if (GetWindowRect (Window::GetWindow (), &rc) == TRUE)
      {
        RawMouse::IN_ClipCursor ();

        // Reset the hardware cursor to the center of the window so we don't
        // hit the screen edge.
        //
        int cx ((rc.right + rc.left) / 2);
        int cy ((rc.top + rc.bottom) / 2);
        SetCursorPos (cx, cy);

        p.x = cx;
        p.y = cy;
      }
    }
    else if (!recenter)
    {
      ClipCursor (NULL);
    }
  }

  BOOL
  RawMouse::OnLBDown (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_LBUTTONDOWN, l, w);
  }

  BOOL
  RawMouse::OnLBUp (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_LBUTTONUP, l, w);
  }

  BOOL
  RawMouse::OnRBDown (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_RBUTTONDOWN, l, w);
  }

  BOOL
  RawMouse::OnRBUp (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_RBUTTONUP, l, w);
  }

  BOOL
  RawMouse::OnMBDown (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_MBUTTONDOWN, l, w);
  }

  BOOL
  RawMouse::OnMBUp (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_MBUTTONUP, l, w);
  }

  BOOL
  RawMouse::OnXBDown (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_XBUTTONDOWN, l, w);
  }

  BOOL
  RawMouse::OnXBUp (LPARAM l, WPARAM w)
  {
    return OnLegacyMouseEvent (WM_XBUTTONUP, l, w);
  }

  RawMouse::RawMouse ()
  {
    Utils::Hook (0x475E65, IN_MouseMove, HOOK_JUMP).install ()->quick ();
    Utils::Hook (0x475E8D, IN_MouseMove, HOOK_JUMP).install ()->quick ();

    Utils::Hook (0x467C03, IN_Init, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x64D095, IN_Init, HOOK_JUMP).install ()->quick ();

    Utils::Hook (0x60BFB9, IN_Frame, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x4A87E2, IN_Frame, HOOK_CALL).install ()->quick ();
    Utils::Hook (0x48A0E6, IN_Frame, HOOK_CALL).install ()->quick ();

    Utils::Hook (0x473517, IN_RecenterMouse, HOOK_CALL).install ()->quick ();

    M_RawInput =
      Dvar::Register<bool> ("m_rawinput",
                            true,
                            Game::DVAR_ARCHIVE | Game::DVAR_SAVED,
                            "Use raw mouse input");
    M_RawInputVerbose =
      Dvar::Register<bool> ("m_rawinput_verbose",
                            false,
                            Game::DVAR_ARCHIVE | Game::DVAR_SAVED,
                            "Show raw mouse input log");

    Window::OnWndMessage (WM_KILLFOCUS, OnKillFocus);
    Window::OnWndMessage (WM_SETFOCUS, OnSetFocus);

    // It's verbose to hook every button message individually, but we need the
    // Msg ID in arguments for the legacy handler.
    //
    Window::OnWndMessage (WM_LBUTTONDOWN, OnLBDown);
    Window::OnWndMessage (WM_LBUTTONUP, OnLBUp);
    Window::OnWndMessage (WM_RBUTTONDOWN, OnRBDown);
    Window::OnWndMessage (WM_RBUTTONUP, OnRBUp);
    Window::OnWndMessage (WM_MBUTTONDOWN, OnMBDown);
    Window::OnWndMessage (WM_MBUTTONUP, OnMBUp);
    Window::OnWndMessage (WM_XBUTTONDOWN, OnXBDown);
    Window::OnWndMessage (WM_XBUTTONUP, OnXBUp);

    Window::OnWndMessage (WM_INPUT, OnRawInput);
    Window::OnCreate (IN_RawMouse_Init);
  }
}
