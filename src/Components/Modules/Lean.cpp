#include "Lean.hpp"
#include "Events.hpp"

namespace Components
{
  Dvar::Var Lean::BGLean;

  Game::kbutton_t Lean::in_leanleft;
  Game::kbutton_t Lean::in_leanright;

  void Lean::IN_LeanLeft_Up()
  {
    Game::IN_KeyUp(&in_leanleft);
  }

  void Lean::IN_LeanLeft_Down()
  {
    Game::IN_KeyDown(&in_leanleft);
  }

  void Lean::IN_LeanRight_Up()
  {
    Game::IN_KeyUp(&in_leanright);
  }

  void Lean::IN_LeanRight_Down()
  {
    Game::IN_KeyDown(&in_leanright);
  }

  void Lean::ApplyLeanFlags(Game::usercmd_s* cmd)
  {
    if ((in_leanleft.active || in_leanleft.wasPressed) && BGLean.get<bool>())
    {
      cmd->buttons |= Game::CMD_BUTTON_LEAN_LEFT;
    }

    if ((in_leanright.active || in_leanright.wasPressed) && BGLean.get<bool>())
    {
      cmd->buttons |= Game::CMD_BUTTON_LEAN_RIGHT;
    }

    in_leanleft.wasPressed = false;
    in_leanright.wasPressed = false;
  }

  void Lean::PM_UpdateLean_Stub(Game::playerState_s* ps, float msec, Game::usercmd_s* cmd, void(*capsuleTrace)(Game::trace_t*, const float*, const float*, const Game::Bounds*, int, int))
  {
    if (BGLean.get<bool>())
    {
      Game::PM_UpdateLean(ps, msec, cmd, capsuleTrace);
    }
  }

  Lean::Lean()
  {
    // Note that on x86 we also register these as key commands. Cmd_AddCommand()
    // has no such flag on x64, where the key code only binds the commands of
    // its own table.
    //
    // @@ So for now the lean commands can't be bound on x64.
    //
    Command::AddRaw("+leanleft", IN_LeanLeft_Down);
    Command::AddRaw("-leanleft", IN_LeanLeft_Up);

    Command::AddRaw("+leanright", IN_LeanRight_Down);
    Command::AddRaw("-leanright", IN_LeanRight_Up);

    Events::OnClientCmdButtons(ApplyLeanFlags);

    Utils::Hook(0x140092AC5, PM_UpdateLean_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x140092C6C, PM_UpdateLean_Stub, HOOK_CALL).install()->quick();

    BGLean = Dvar::Register<bool>("bg_lean", true, Game::DVAR_CODINFO, "Enable CoD4 leaning");
  }
}
