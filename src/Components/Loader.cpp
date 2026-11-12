#include <Utils/InfoString.hpp>

#include "Loader.hpp"

#include "Modules/Auth.hpp"
#include "Modules/Exception.hpp"
#include "Modules/IPCPipe.hpp"
#include "Modules/Network.hpp"
#include "Modules/Logger.hpp"
#include "Modules/Sentry.hpp"
#include "Modules/UIScript.hpp"
#include "Modules/ConfigStrings.hpp"
#include "Modules/ArenaLength.hpp"
#include "Modules/AssetHandler.hpp"
#include "Modules/Bans.hpp"
#include "Modules/Bots.hpp"
#include "Modules/Branding.hpp"
#include "Modules/Bullet.hpp"
#include "Modules/CardTitles.hpp"
#include "Modules/Ceg.hpp"

namespace Components
{
  bool Loader::Pregame = true;
  std::vector<Component*> Loader::Components;

  bool Loader::IsPregame()
  {
    return Pregame;
  }

  void Loader::Initialize()
  {
    Pregame = true;
    Utils::Memory::GetAllocator()->clear();

    // High priority.
    //
    Register(new Singleton());

    Register(new Auth());
    Register(new Command());
    Register(new Dvar());
    Register(new Exception());
    Register(new IPCPipe());
    Register(new Network());
    Register(new Logger());
    Register(new Sentry());
    Register(new UIScript());
    Register(new ConfigStrings());
    Register(new ArenaLength());
    Register(new AssetHandler());
    Register(new Bans());
    Register(new Bots());
    Register(new Branding());
    Register(new Bullet());
    Register(new CardTitles());
    Register(new Ceg());
    Register(new Scheduler());

    // @@ TODO: register components as we port them, in x86 client order
    //    (see unported sources in premake5.lua).
    //

    Pregame = false;
  }

  void Loader::Register(Component* component)
  {
    if (component)
    {
#if defined(DEBUG)
      Logger::Print("Component registered: {}\n", component->getName());
#endif
      Components.push_back(component);
    }
  }
}
