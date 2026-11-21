#pragma once

namespace Components
{
  struct rawMouseValue_t
  {
    int current = 0;
    int previous = 0;

    void ResetDelta();
    int GetDelta() const;
    void Update(int value, bool absolute);
  };

  class RawMouse : public Component
  {
  public:
    RawMouse();

    static void IN_MouseMove();

    static LRESULT OnLBDown(LPARAM lParam, WPARAM wParam);
    static LRESULT OnLBUp(LPARAM lParam, WPARAM wParam);

    static LRESULT OnRBDown(LPARAM lParam, WPARAM wParam);
    static LRESULT OnRBUp(LPARAM lParam, WPARAM wParam);

    static LRESULT OnMBDown(LPARAM lParam, WPARAM wParam);
    static LRESULT OnMBUp(LPARAM lParam, WPARAM wParam);

    static LRESULT OnXBDown(LPARAM lParam, WPARAM wParam);
    static LRESULT OnXBUp(LPARAM lParam, WPARAM wParam);

  private:
    static Dvar::Var M_RawInput, M_RawInputVerbose, R_DisplayMode, R_AutoPriority;
    static rawMouseValue_t MouseRawX, MouseRawY;
    static uint32_t MouseRawEvents;
    static bool InRawInput, FirstRawInputUpdate, InFocus;

    static void IN_ClampMouseMove();
    static void ResetMouseRawEvents();
    static void ProcessMouseRawEvent(DWORD usButtonFlags, DWORD flag_down, DWORD mouse_event);
    static bool GetRawInput(LPARAM lParam, RAWINPUT& raw, UINT& dwSize);
    static LRESULT OnRawInput(LPARAM lParam, WPARAM);
    static bool IsMouseInClientBounds();
    static LRESULT OnLegacyMouseEvent(UINT Msg, LPARAM lParam, WPARAM wParam);
    static LRESULT OnKillFocus(LPARAM lParam, WPARAM);
    static LRESULT OnSetFocus(LPARAM lParam, WPARAM);
    static void IN_RawMouseMove();
    static bool ToggleRawInput(bool enable = true);
    static void IN_RawMouse_Init();
    static void IN_Init();
    static void IN_Frame();
    static BOOL IN_ClipCursor();
    static BOOL IN_RecenterMouse();
  };
}
