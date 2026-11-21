#pragma once

extern "C"
{
  void UIFeeder_MouseEnterStub();
  void UIFeeder_PlaySoundStub();
  void UIFeeder_CheckSelectionStub();

  bool UIFeeder_CheckFeeder();
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
    friend bool (::UIFeeder_CheckFeeder)();
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

    static int (*UI_FeederCount_Original)(int localClientNum, float feeder);
    static const char* (*UI_FeederItemText_Original)(int localClientNum, Game::itemDef_s* item, float feeder, int index, int column, float* s0, float* t0, float* s1, float* t1, Game::Material** handle);
    static void (*UI_FeederSelection_Original)(int localClientNum, float feeder, int index);

    static int UI_FeederCount_Hk(int localClientNum, float feeder);
    static const char* UI_FeederItemText_Hk(int localClientNum, Game::itemDef_s* item, float feeder, int index, int column, float* s0, float* t0, float* s1, float* t1, Game::Material** handle);
    static void UI_FeederSelection_Hk(int localClientNum, float feeder, int index);
    static void MouseSelect_Hk(int localClientNum, float feeder, int index);
    static void HandleKey_Hk(void* dc, Game::itemDef_s* item, Game::MenuEventHandlerSet* handler);

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
