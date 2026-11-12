#pragma once

namespace Components
{
  class Dvar : public Component
  {
  public:
    class Var
    {
    public:
      Var() : dvar_(nullptr) {}
      Var(Game::dvar_t* dvar) : dvar_(dvar) {}
      Var(uintptr_t ppdvar) : Var(*reinterpret_cast<Game::dvar_t**>(ppdvar)) {}
      Var(const std::string& dvarName);

      template<typename T> T get();

      void set(const char* string);
      void set(const std::string& string);

      void set(int integer);
      void set(float value);
      void set(bool enabled);

      void setRaw(int integer);
      void setRaw(float value);
      void setRaw(bool enabled);

    private:
      Game::dvar_t* dvar_;
    };

    Dvar();

    // Only strings and bools use this type of declaration
    template<typename T> static Var Register(const char* dvarName, T value, std::uint16_t flag, const char* description);
    template<typename T> static Var Register(const char* dvarName, T value, T min, T max, std::uint16_t flag, const char* description);

    static Var Name;

  private:
    static const Game::dvar_t* Dvar_RegisterName(const char* dvarName, const char* value, std::uint16_t flags, const char* description);
    static const Game::dvar_t* Dvar_RegisterSVNetworkFps(const char* dvarName, int value, int min, int max, std::uint16_t flags, const char* description);
    static const Game::dvar_t* Dvar_RegisterDrawFps(const char* dvarName, const char** valueList, int defaultIndex, std::uint16_t flags, const char* description);
    static const Game::dvar_t* Dvar_RegisterFov(const char* dvarName, float value, float min, float max, std::uint16_t flags, const char* description);
    static const Game::dvar_t* Dvar_RegisterVolume(const char* dvarName, float value, float min, float max, std::uint16_t flags, const char* description);

    static void SetFromStringByNameExternal(const char* dvarName, const char* string);
    static void SetFromStringByNameSafeExternal(const char* dvarName, const char* string);

    static bool AreArchiveDvarsUnprotected();
    static bool IsSettingDvarsDisabled();
    static void DvarSetFromStringByName_Stub(const char* dvarName, const char* value);

    static const char* Dvar_EnumToString_Stub(const Game::dvar_t* dvar);
  };
}
