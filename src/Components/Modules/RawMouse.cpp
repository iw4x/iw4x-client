#include <STDInclude.hpp>

namespace Components
{
	void IN_ClampMouseMove()
	{
        tagRECT rc;
        tagPOINT curPos;

        GetCursorPos(&curPos);
        GetWindowRect(Game::g_wv->hWnd, &rc);
        bool isClamped = false;
        if (curPos.x >= rc.left)
        {
            if (curPos.x >= rc.right)
            {
                curPos.x = rc.right - 1;
                isClamped = true;
            }
        }
        else
        {
            curPos.x = rc.left;
            isClamped = true;
        }
        if (curPos.y >= rc.top)
        {
            if (curPos.y >= rc.bottom)
            {
                curPos.y = rc.bottom - 1;
                isClamped = true;
            }
        }
        else
        {
            curPos.y = rc.top;
            isClamped = true;
        }

        if (isClamped)
        {
            SetCursorPos(curPos.x, curPos.y);
        }
	}

    void IN_RawMouseMove()
    {
        static Game::dvar_t* r_fullscreen = Game::Dvar_FindVar("r_fullscreen");

        if (GetForegroundWindow() == Game::g_wv->hWnd)
        {
            if (r_fullscreen->current.enabled)
                IN_ClampMouseMove();


        }
    }

    Dvar::Var Mouse_RawInput;

    void IN_MouseMove()
    {
        if (Mouse_RawInput.get<bool>())
        {
            IN_RawMouseMove();
        }
        else
        {
            Game::IN_MouseMove();
        }
    }

	RawMouse::RawMouse()
	{
        Utils::Hook(0x475E65, IN_MouseMove, HOOK_JUMP).install()->quick();
        Utils::Hook(0x475E8D, IN_MouseMove, HOOK_JUMP).install()->quick();
        Utils::Hook(0x475E9E, IN_MouseMove, HOOK_JUMP).install()->quick();

        Mouse_RawInput = Dvar::Register<bool>("m_rawinput", true, Game::dvar_flag::DVAR_ARCHIVE, "Use raw mouse input.");
	}
}
