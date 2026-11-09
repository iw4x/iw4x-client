#pragma once

extern "C"
{
  extern const char* ScriptExtension_ReplacedPos;

  void ScriptExtension_VMExecuteInternalStub();

  void ScriptExtension_GetReplacedPos(const char* pos);
}

namespace Components::GSC
{
  class ScriptExtension : public Component
  {
  public:
    ScriptExtension();

    static const char* GetCodePosForParam(int index);

  private:
    friend void (::ScriptExtension_GetReplacedPos)(const char* pos);

    static std::unordered_map<const char*, const char*> ReplacedFunctions;

    static void GetReplacedPos(const char* pos);
    static void SetReplacedPos(const char* what, const char* with);

    static void AddFunctions();
  };
}
