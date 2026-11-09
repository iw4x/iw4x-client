#pragma once

extern "C"
{
  void UIFeeder_SetItemSelectionStub();
  void UIFeeder_GetItemTextStub();
  void UIFeeder_GetItemCountStub();
  void UIFeeder_HandleKeyStub();
  void UIFeeder_MouseEnterStub();
  void UIFeeder_MouseSelectStub();
  void UIFeeder_PlaySoundStub();
  void UIFeeder_CheckSelectionStub();

  bool UIFeeder_SetItemSelection();
  const char* UIFeeder_GetItemText();
  unsigned int UIFeeder_GetItemCount();
  bool UIFeeder_CheckFeeder();
  bool UIFeeder_HandleClick(Game::itemDef_s* item);
  int UIFeeder_CheckSelection(int feeder);
}

namespace Components
{
  class UIFeeder : public Component
  {
  public:
    typedef Utils::Slot<unsigned int()> GetItemCount_t;
    typedef Utils::Slot<const char*(unsigned int /*index*/, int /*column*/)> GetItemText_t;
    typedef Utils::Slot<void(unsigned int /*index*/)> Select_t;

    struct Callbacks
    {
      GetItemCount_t getItemCount;
      GetItemText_t getItemText;
      Select_t select;
    };

    UIFeeder();
    ~UIFeeder();

    static void Add(float feeder, GetItemCount_t itemCountCb, GetItemText_t itemTextCb, Select_t selectCb);
    static void Select(float feeder, unsigned int index);

    struct Container
    {
      float feeder;
      int index;
      int column;
    };

  private:
    friend bool (::UIFeeder_SetItemSelection)();
    friend const char* (::UIFeeder_GetItemText)();
    friend unsigned int (::UIFeeder_GetItemCount)();
    friend bool (::UIFeeder_CheckFeeder)();
    friend bool (::UIFeeder_HandleClick)(Game::itemDef_s* item);
    friend int (::UIFeeder_CheckSelection)(int feeder);

    static Dvar::Var UIMapLong;
    static Dvar::Var UIMapName;
    static Dvar::Var UIMapDesc;

    static unsigned int GetItemCount();

    static const char* GetItemText();

    static bool SetItemSelection();

    static bool CheckFeeder();
    static int CheckSelection(int feeder);

    static bool HandleClick(Game::itemDef_s* item);

    static std::unordered_map<float, Callbacks> Feeders;

    static unsigned int GetMapCount();
    static const char* GetMapText(unsigned int index, int column);
    static void SelectMap(unsigned int index);
    static void ApplyMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
    static void ApplyInitialMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);
  };
}

extern "C"
{
  extern Components::UIFeeder::Container UIFeeder_Current;
}
