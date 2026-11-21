#pragma once

namespace Components
{
  // Note that x64 has neither the precompiler nor the menu parser that we
  // load the menus on disk with on x86. So our menus come from our zones and
  // here we only keep the openmenu command and our patches of the game.
  //
  class Menus : public Component
  {
  public:
    Menus();
  };
}
