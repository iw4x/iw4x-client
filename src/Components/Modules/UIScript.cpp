#include "UIScript.hpp"

namespace Components
{
  std::unordered_map<std::string, UIScript::UIScriptHandler> UIScript::UIScripts;
  std::unordered_map<int, std::function<void()>> UIScript::UIOwnerDraws;

  template<> int UIScript::Token::get() const
  {
    if (this->isValid())
    {
      return std::strtol(this->token, nullptr, 0);
    }

    return 0;
  }

  template<> const char* UIScript::Token::get() const
  {
    if (this->isValid())
    {
      return this->token;
    }

    return "";
  }

  template<> std::string UIScript::Token::get() const
  {
    return {this->get<const char*>()};
  }

  bool UIScript::Token::isValid() const
  {
    return (this->token && this->token[0]);
  }

  void UIScript::Token::parse(const char** args)
  {
    if (args)
    {
      this->token = Game::Com_Parse(args);
    }
  }

  Game::uiInfo_s* UIScript::UI_GetClientInfo(int localClientNum)
  {
    AssertIn(localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);
    return &Game::uiInfoArray[localClientNum];
  }

  void UIScript::Add(const std::string& name, const UIScriptHandler& callback)
  {
    UIScripts[name] = callback;
  }

  void UIScript::AddOwnerDraw(int ownerdraw, const std::function<void()>& callback)
  {
    UIOwnerDraws[ownerdraw] = callback;
  }

  bool UIScript::RunMenuScript(const char* name, const char** args)
  {
    if (const auto itr = UIScripts.find(name); itr != UIScripts.end())
    {
      const auto* info = UI_GetClientInfo(0);
      itr->second(Token(args), info);
      return true;
    }

    return false;
  }

  int UIScript::OwnerDrawHandleKeyStub(int ownerDraw, int flags, float *special, int key)
  {
    if (key == 200 || key == 201) // mouse buttons
    {
      for (auto i = UIOwnerDraws.begin(); i != UIOwnerDraws.end(); ++i)
      {
        if (i->first == ownerDraw)
        {
          i->second();
        }
      }
    }

    return Utils::Hook::Call<int(int, int, float*, int)>(0x14026EE10)(ownerDraw, flags, special, key);
  }

  UIScript::UIScript()
  {
    AssertSize(Game::uiInfo_s, 0x3948);

    if (Dedicated::IsEnabled()) return;

    // Install handler
    //
    // We jump to the stub where UI_RunMenuScript() goes on past its
    // "StartServer" case. The stub passes the script's name (at rbp+30h)
    // and the rest of its arguments (through rbx) to our handler. If one of
    // ours took the script, then it jumps to where the game is done with it,
    // and otherwise it redoes the store we replaced and lets the game look
    // further. Note that this means the game's own scripts up to
    // "StartServer" always win over ours, which so far seems to be what we
    // want.
    //
    Utils::Hook(0x1402703E7, UIScript_RunMenuScriptStub, HOOK_JUMP).install()->quick();

    // Install ownerdraw handler
    Utils::Hook(0x14025DB9B, OwnerDrawHandleKeyStub, HOOK_CALL).install()->quick();
  }

  UIScript::~UIScript()
  {
    UIScripts.clear();
    UIOwnerDraws.clear();
  }
}

bool UIScript_RunMenuScript(const char* name, const char** args)
{
  return Components::UIScript::RunMenuScript(name, args);
}
