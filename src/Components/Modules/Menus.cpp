#include "Menus.hpp"

namespace Components
{
  Menus::Menus()
  {
    if (Dedicated::IsEnabled()) return;

    // The stock ASSET_TYPE_MENU clone handler copies runtime state from the existing menu to its
    // replacement, assuming both menus have identical item layouts. Disable it to prevent state
    // from being copied between unrelated items when the layouts differ.
    Utils::Hook::Set<Game::DB_DynamicCloneXAssetHandler_t>(
      &Game::DB_DynamicCloneXAssetHandler[Game::ASSET_TYPE_MENU], nullptr);

    // Hunk size (stock is 0xA00000). First raised to 0xB00000 for heavy-loaded menus. Large
    // ported maps also exhaust it once PMem is big enough to let them get that far, dying on
    // allocations of a few hundred bytes: "Hunk_AllocAlign failed on 380 bytes (total 11 MB...)".
    // Like PMem, Com_InitHunkMemory reserves this and commits on demand, so the cost is
    // address space rather than memory.
    Utils::Hook::Replace32(0x14027D854, 5, 0xA00000, 0x4000000); // Com_InitHunkMemory, 64 MiB
    Utils::Hook::Replace32(0x14027D859, 11, 0xA00000, 0x4000000);

    // Don't open connect menu twice - it gets stuck!
    Utils::Hook::Nop(0x14026AC4B, 5);

    // Note that on x86 we also disable the two tokens of ItemParse_rect that
    // IW4 added, which x64 compiles out along with the rest of the parser.
    //

    // make Com_Error and similar go back to main_text instead of menu_xboxlive.
    Utils::Hook::SetString(0x140379490, "main_text");

    Command::Add("openmenu", [](const Command::Params* params)
      {
        if (params->size() != 2)
        {
          Logger::Print("USAGE: openmenu <menu name>\n");
          return;
        }

        // Not quite sure if we want to do this if we're not ingame, but it's only needed for ingame menus.
        if ((*Game::cl_ingame)->current.enabled)
        {
          Game::Key_SetCatcher(0, Game::KEYCATCH_UI);
        }

        const char* menuName = params->get(1);

        Game::Menus_OpenByName(Game::uiContext, menuName);
      });
  }
}
