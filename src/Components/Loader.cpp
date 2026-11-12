#include <Utils/InfoString.hpp>

#include "Loader.hpp"

#include "Modules/Auth.hpp"

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
