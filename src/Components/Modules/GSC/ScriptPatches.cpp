#include <Components/Modules/Sentry.hpp>
#include "ScriptPatches.hpp"
#include "Script.hpp"

using namespace Utils::String;

namespace Components::GSC
{
  constexpr auto offset = 511;

  Game::game_hudelem_s* ScriptPatches::HECmd_GetHudElem(Game::scr_entref_t entref)
  {
    if (entref.classnum != 1)
    {
      Game::Scr_ObjectError("not a hud element");
      return nullptr;
    }

    assert(entref.entnum < 1024);
    return &Game::g_hudelems[entref.entnum];
  }

  void ScriptPatches::Scr_TableLookupIStringByRow_Hk()
  {
    if (Game::Scr_GetNumParam() < 3)
    {
      Game::Scr_Error("USAGE: tableLookupIStringByRow( filename, rowNum, returnValueColumnNum )");
      return;
    }

    const auto* fileName = Game::Scr_GetString(0);
    const auto rowNum = Game::Scr_GetInt(1);
    const auto returnValueColumnNum = Game::Scr_GetInt(2);

    const auto* table = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_STRINGTABLE, fileName).stringTable;

    if (!table)
    {
      Game::Scr_ParamError(0, Utils::String::VA("%s does not exist", fileName));
      return;
    }

    const auto* value = Game::StringTable_GetColumnValueForRow(table, rowNum, returnValueColumnNum);
    Game::Scr_AddIString(value);
  }

  // Make sure the stack of a thread that is about to wait is sane before the
  // game archives it.
  //
  // When a thread waits, the game copies what its frame has on the stack,
  // that is, from the start of the frame to the top. We've seen the top end
  // up below the start (with Bot Warfare in a private match), at which point
  // the copy runs off the bottom of the stack and crashes. Something corrupts
  // the VM before that and we don't know what yet. So instead of crashing we
  // end the match with an error and offer to report what we know about the
  // stack, which is hopefully enough to find the culprit.
  //
  // Note that we don't try to recover: by now the stack can't be trusted and
  // carrying on would only move the crash somewhere else.
  //
  unsigned int ScriptPatches::VM_ArchiveStack_Hk()
  {
    const auto& fs (*reinterpret_cast<Game::function_stack_t*> (0x2040CD4));

    if (fs.top < fs.startTop)
    {
      // Describe the frames as offsets into the program buffer, innermost
      // first. The positions are only meaningful together with the scripts,
      // but that's what we have outside of developer mode.
      //
      const auto* program (Game::scrVarPub->programBuffer);

      std::string frames (std::format ("{}", fs.pos - program));
      for (int i (Game::scrVmPub->function_count - 1); i > 0; --i)
        frames += std::format (" {}", Game::scrVmPub->function_frame_start[i].fs.pos - program);

      const auto details (
        std::format ("script stack is corrupt: top is {} values below the "
                     "start of the frame (frames {}, positions {}, fs_game "
                     "'{}')",
                     fs.startTop - fs.top,
                     Game::scrVmPub->function_count,
                     frames,
                     (*Game::fs_gameDirVar)->current.string));

      Logger::PrintError (Game::CON_CHANNEL_ERROR, "{}\n", details);

      if (auto* r = Sentry::Runtime ())
        r->capture (SENTRY_LEVEL_ERROR, "script_stack", details);

      Logger::Error (Game::ERR_DROP,
                     "Script error: the game's script state is corrupt, so "
                     "the match had to end.");
    }

    return Utils::Hook::Call<unsigned int ()> (0x61DB70) ();
  }

  ScriptPatches::ScriptPatches()
  {
    // Fix format string in Scr_RandomFloatRange
    Utils::Hook::Set<const char*>(0x5F10C6, "Scr_RandomFloatRange parms: %f %f ");

    Utils::Hook (0x6206C8, VM_ArchiveStack_Hk, HOOK_CALL).install ()->quick (); // VM_Execute: wait.
    Utils::Hook (0x62080B, VM_ArchiveStack_Hk, HOOK_CALL).install ()->quick (); // VM_Execute: waittillframeend.
    Utils::Hook (0x62101C, VM_ArchiveStack_Hk, HOOK_CALL).install ()->quick (); // VM_Execute: waittill.

    // Correct builtin function pointer
    Utils::Hook::Set<Game::BuiltinFunction>(0x79A90C, Scr_TableLookupIStringByRow_Hk);

    Script::AddMethod("ClearHudText", [](Game::scr_entref_t entref) -> void
    {
      auto* hud = HECmd_GetHudElem(entref);

      // Frees config string up
      if ((hud->elem).text)
      {
        Game::SV_SetConfigstring((hud->elem).text + offset, nullptr);
      }
    });
  }
}
