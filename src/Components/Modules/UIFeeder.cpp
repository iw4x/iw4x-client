#include "UIFeeder.hpp"
#include "Events.hpp"
#include "Maps.hpp"

extern "C"
{
  Components::UIFeeder::Container UIFeeder_Current;
}

namespace Components
{
  std::unordered_map<float, UIFeeder::Callbacks> UIFeeder::Feeders;

  Dvar::Var UIFeeder::UIMapLong;
  Dvar::Var UIFeeder::UIMapName;
  Dvar::Var UIFeeder::UIMapDesc;

  void UIFeeder::Add(float feeder, GetItemCount_t itemCountCb, GetItemText_t itemTextCb, Select_t selectCb)
  {
    Feeders[feeder] = { itemCountCb, itemTextCb, selectCb };
  }

  const char* UIFeeder::GetItemText()
  {
    if (Feeders.contains(UIFeeder_Current.feeder))
    {
      return Feeders[UIFeeder_Current.feeder].getItemText(UIFeeder_Current.index, UIFeeder_Current.column);
    }

    return nullptr;
  }

  unsigned int UIFeeder::GetItemCount()
  {
    if (Feeders.contains(UIFeeder_Current.feeder))
    {
      return Feeders[UIFeeder_Current.feeder].getItemCount();
    }

    return 0;
  }

  bool UIFeeder::SetItemSelection()
  {
    if (Feeders.contains(UIFeeder_Current.feeder))
    {
      Feeders[UIFeeder_Current.feeder].select(UIFeeder_Current.index);
      return true;
    }

    return false;
  }

  bool UIFeeder::CheckFeeder()
  {
    if (UIFeeder_Current.feeder == 15.0f) return false;
    return Feeders.contains(UIFeeder_Current.feeder);
  }

  bool UIFeeder::HandleClick(Game::itemDef_s* item)
  {
    static Game::itemDef_s* lastItem = nullptr;
    static int lastIndex = -1;
    static int lastTime = 0;

    const auto index = item->typeData.listBox->mousePos;
    const auto now = Game::Sys_Milliseconds();

    // We only treat a click as a double click if it lands on the row that is
    // still selected in the same list within 300ms. In this case we return
    // true and the caller runs the list's double click script. Any other
    // click selects the row under the cursor.
    //
    // Note that a list can change between the two clicks (the server list,
    // for example, re-sorts as servers respond), which moves the selected
    // row away from the cursor. The second click then selects the row that
    // is now under the cursor.
    //
    if (item == lastItem && index == lastIndex && index == item->cursorPos[0] && now - lastTime < 300)
    {
      lastItem = nullptr;
      return true;
    }

    lastItem = item;
    lastIndex = index;
    lastTime = now;

    if (item->cursorPos[0] != index)
    {
      item->cursorPos[0] = index;
      UIFeeder_Current.index = index;

      SetItemSelection();
    }

    return false;
  }

  // Note that on x86 we go in at the start of these three with stubs that
  // continue in the game's function when we don't take the feeder. On x64
  // we detour them instead.
  //
  int (*UIFeeder::UI_FeederCount_Original)(int localClientNum, float feeder);
  const char* (*UIFeeder::UI_FeederItemText_Original)(int localClientNum, Game::itemDef_s* item, float feeder, int index, int column, float* s0, float* t0, float* s1, float* t1, Game::Material** handle);
  void (*UIFeeder::UI_FeederSelection_Original)(int localClientNum, float feeder, int index);

  int UIFeeder::UI_FeederCount_Hk(const int localClientNum, const float feeder)
  {
    UIFeeder_Current.feeder = feeder;

    if (const auto count = GetItemCount())
    {
      return static_cast<int>(count);
    }

    return UI_FeederCount_Original(localClientNum, feeder);
  }

  const char* UIFeeder::UI_FeederItemText_Hk(const int localClientNum, Game::itemDef_s* item, const float feeder, const int index, const int column, float* s0, float* t0, float* s1, float* t1, Game::Material** handle)
  {
    UIFeeder_Current.feeder = feeder;
    UIFeeder_Current.index = index;
    UIFeeder_Current.column = column;

    if (const auto* text = GetItemText())
    {
      *handle = nullptr;
      return text;
    }

    return UI_FeederItemText_Original(localClientNum, item, feeder, index, column, s0, t0, s1, t1, handle);
  }

  void UIFeeder::UI_FeederSelection_Hk(const int localClientNum, const float feeder, const int index)
  {
    UIFeeder_Current.feeder = feeder;
    UIFeeder_Current.index = index;

    if (SetItemSelection())
    {
      return;
    }

    UI_FeederSelection_Original(localClientNum, feeder, index);
  }

  // Our feeders select on click, not when the mouse moves over a row.
  //
  void UIFeeder::MouseSelect_Hk(const int localClientNum, const float feeder, const int index)
  {
    UIFeeder_Current.feeder = feeder;

    if (CheckFeeder())
    {
      return;
    }

    Utils::Hook::Call<void(int, float, int)>(0x14026CD30)(localClientNum, feeder, index); // UI_FeederSelection
  }

  // A click on one of our feeders selects the row, and only a second click
  // on it runs the list's double click script.
  //
  void UIFeeder::HandleKey_Hk(void* dc, Game::itemDef_s* item, Game::MenuEventHandlerSet* handler)
  {
    UIFeeder_Current.feeder = item->special;

    if (CheckFeeder() && !HandleClick(item))
    {
      return;
    }

    Utils::Hook::Call<void(void*, Game::itemDef_s*, Game::MenuEventHandlerSet*)>(0x14025D320)(dc, item, handler); // Item_RunScript
  }

  void UIFeeder::Select(float feeder, unsigned int index)
  {
    if (Game::uiContext->openMenuCount > 0)
    {
      auto* menu = Game::uiContext->menuStack[Game::uiContext->openMenuCount - 1];

      if (menu && menu->items)
      {
        for (int i = 0; i < menu->itemCount; ++i)
        {
          auto* item = menu->items[i];
          if (item && item->type == 6 && item->special == feeder)
          {
            item->cursorPos[0] = static_cast<int>(index);
            break;
          }
        }
      }
    }
  }

  unsigned int UIFeeder::GetMapCount()
  {
    return static_cast<unsigned int>(Maps::GetCustomMaps().size());
  }

  const char* UIFeeder::GetMapText(unsigned int index, int /*column*/)
  {
    const auto& maps = Maps::GetCustomMaps();
    if (index < maps.size())
    {
      return maps.at(index).data();
    }

#ifdef _DEBUG
    if (IsDebuggerPresent())
    {
      __debugbreak();
    }
#endif

    return "";
  }

  void UIFeeder::SelectMap(unsigned int index)
  {
    const auto& maps = Maps::GetCustomMaps();

    if (index < maps.size())
    {
      std::string mapName = maps[index];

      std::string longName = mapName;
      std::string description = "(Missing arena file!)";

      const auto& arenaPath = Maps::GetArenaPath(mapName);

      if (Utils::IO::FileExists(arenaPath))
      {
        const auto& arena = Maps::ParseCustomMapArena(Utils::IO::ReadFile(arenaPath));

        if (arena.contains("longname"))
        {
          longName = arena.at("longname");
        }

        if (arena.contains("map"))
        {
          mapName = arena.at("map");
        }

        if (arena.contains("description"))
        {
          description = arena.at("description");
        }
      }

      UIMapName.set(Localization::Get(mapName.data()));
      UIMapLong.set(Localization::Get(longName.data()));
      UIMapDesc.set(Localization::Get(description.data()));
    }
  }

  void UIFeeder::ApplyMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
  {
    const auto* mapname = Dvar::Var("ui_map_name").get<const char*>();

    if (*mapname)
    {
      Game::Dvar_SetString(*Game::ui_mapname, mapname);
      Utils::Hook::Call<void(const char*)>(0x14010BD90)(mapname); // Party_SetDisplayMapName
    }
  }

  void UIFeeder::ApplyInitialMap([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
  {
    Maps::ScanCustomMaps();
    Select(60.0f, 0); // Will select nothing if there's no map

    if (GetMapCount() > 0)
    {
      SelectMap(0);
    }
  }

  int UIFeeder::CheckSelection(int feeder)
  {
    // Changelog and server info (player list) should not have the hover effect
    if (feeder == 62 || feeder == 13) return 0;
    return 1;
  }

  UIFeeder::UIFeeder()
  {
    // UIFeeder.asm has its own copy of these offsets.
    //
    AssertOffset(UIFeeder::Container, feeder, 0x0);
    AssertOffset(UIFeeder::Container, index, 0x4);
    AssertOffset(UIFeeder::Container, column, 0x8);

    // We and UIFeeder.asm read these.
    //
    AssertOffset(Game::itemDef_s, type, 0xC4);
    AssertOffset(Game::itemDef_s, dataType, 0xC8);
    AssertOffset(Game::itemDef_s, special, 0x180);
    AssertOffset(Game::itemDef_s, cursorPos, 0x184);
    AssertOffset(Game::itemDef_s, typeData, 0x188);
    AssertOffset(Game::listBoxDef_s, mousePos, 0x0);
    AssertOffset(Game::menuDef_t, itemCount, 0xBC);
    AssertOffset(Game::menuDef_t, items, 0x168);
    AssertOffset(Game::UiContext, menuStack, 0x1440);
    AssertOffset(Game::UiContext, openMenuCount, 0x14C0);

    if (Dedicated::IsEnabled()) return;

    Events::OnDvarInit([]
    {
      UIMapLong = Dvar::Register<const char*>("ui_map_long", "", Game::DVAR_NONE, "");
      UIMapName = Dvar::Register<const char*>("ui_map_name", "", Game::DVAR_NONE, "");
      UIMapDesc = Dvar::Register<const char*>("ui_map_desc", "", Game::DVAR_NONE, "");
    });

    // Get feeder item count
    UI_FeederCount_Original = Utils::Hook::Detour(0x14026BC90, 6, &UI_FeederCount_Hk);

    // Get feeder item text
    UI_FeederItemText_Original = Utils::Hook::Detour(0x14026C3D0, 5, &UI_FeederItemText_Hk);

    // Select feeder item
    UI_FeederSelection_Original = Utils::Hook::Detour(0x14026CD30, 6, &UI_FeederSelection_Hk);

    // Mouse enter check
    Utils::Hook(0x14025E6B1, UIFeeder_MouseEnterStub, HOOK_JUMP).install()->quick();

    // Handle key event
    Utils::Hook(0x14025E276, HandleKey_Hk, HOOK_CALL).install()->quick();

    // Mouse select check
    Utils::Hook(0x14025E66E, MouseSelect_Hk, HOOK_CALL).install()->quick();

    // Play mouse over sound check
    Utils::Hook(0x14025E69F, UIFeeder_PlaySoundStub, HOOK_CALL).install()->quick();

    // Disable blinking
    //
    // Note that x64 inlines the check into Item_ListBox_Paint().
    //
    Utils::Hook(0x14025F614, UIFeeder_CheckSelectionStub, HOOK_JUMP).install()->quick();

    // some thing overwriting feeder 2's data
    Utils::Hook::Set<BYTE>(0x14026E427, 0xEB);

    // Use feeder 5 for maps, as feeder 4 selects on mouse over
    UIFeeder::Add(60.0f, UIFeeder::GetMapCount, UIFeeder::GetMapText, UIFeeder::SelectMap);
    UIScript::Add("ApplyMap", UIFeeder::ApplyMap);
    UIScript::Add("ApplyInitialMap", UIFeeder::ApplyInitialMap);

    // Fix feeder focus
    Utils::Hook::Nop(0x14025F1F8, 6); // Flag 2 check (WINDOW_HASFOCUS)
    Utils::Hook::Nop(0x14025F20B, 6); // Flag 2000 check (WINDOW_LB_THUMB?)
  }

  UIFeeder::~UIFeeder()
  {
    UIFeeder::Feeders.clear();
  }
}

bool UIFeeder_CheckFeeder()
{
  return Components::UIFeeder::CheckFeeder();
}

int UIFeeder_CheckSelection(int feeder)
{
  return Components::UIFeeder::CheckSelection(feeder);
}
