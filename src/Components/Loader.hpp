#pragma once

namespace Components
{
  class Component
  {
  public:
    Component() = default;
    virtual ~Component() = default;

#if defined(DEBUG)
    virtual std::string getName()
    {
      std::string name = typeid(*this).name();
      Utils::String::Replace(name, "class Components::", "");
      return name;
    };
#endif
  };

  class Loader
  {
  public:
    static void Initialize();
    static void Register(Component* component);

    static bool IsPregame();

#if defined(__cpp_rtti) || defined(_CPPRTTI)
    template <typename T>
    static T* GetInstance()
    {
      for (auto& component : Components)
      {
        if (typeid(*component) == typeid(T))
        {
          return reinterpret_cast<T*>(component);
        }
      }

      return nullptr;
    }
#endif

  private:
    static bool Pregame;
    static std::vector<Component*> Components;
  };
}
