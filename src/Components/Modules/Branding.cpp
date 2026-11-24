#include "Branding.hpp"

#include <version.hpp>

namespace Components
{
  Dvar::Var Branding::CGDrawVersion;
  Dvar::Var Branding::CGDrawVersionX;
  Dvar::Var Branding::CGDrawVersionY;

#ifdef _DEBUG
  constexpr auto* BUILD_TYPE = "IW4x_DEV MP";
#else
  constexpr auto* BUILD_TYPE = "IW4x MP";
#endif

  void Branding::CG_DrawVersion()
  {
    // Default values
    constexpr auto fontScale = 0.25f;
    constexpr auto maxChars = std::numeric_limits<int>::max();
    // Default colours
    constexpr Game::vec4_t shadowColor = {0.0f, 0.0f, 0.0f, 0.69f};
    constexpr Game::vec4_t color = {0.4f, 0.69f, 1.0f, 0.69f};

    auto* const placement = Game::ScrPlace_GetUnsafeFullPlacement();
    auto* const font = Game::UI_GetFontHandle(placement, 0, 0.583f);

    const auto width = Game::UI_TextWidth((*Game::version)->current.string, 0, font, fontScale);
    const auto height = Game::UI_TextHeight(font, fontScale);

    Game::UI_DrawText(placement, (*Game::version)->current.string, maxChars, font, 1.0f - (CGDrawVersionX.get<float>() + static_cast<float>(width)),
      1.0f - (CGDrawVersionY.get<float>() + static_cast<float>(height)), 3, 3, fontScale, shadowColor, 0);
    Game::UI_DrawText(placement, (*Game::version)->current.string, maxChars, font, (0.0f - static_cast<float>(width)) - CGDrawVersionX.get<float>(),
      (0.0f - static_cast<float>(height)) - CGDrawVersionY.get<float>(), 3, 3, fontScale, color, 0);
  }

  void Branding::CL_DrawDebugOverlays_Hk(int localClientNum)
  {
    Game::CL_DrawDebugOverlays(localClientNum);

    if (Game::CL_GetLocalClientConnectionState(localClientNum) == Game::CA_ACTIVE &&
        CGDrawVersion.get<bool>())
    {
      CG_DrawVersion();
    }
  }

  const char* Branding::GetBuildNumber()
  {
#ifdef EXPERIMENTAL_BUILD
    return REVISION_STR "-develop latest " __DATE__ " " __TIME__;
#else
    return REVISION_STR " latest " __DATE__ " " __TIME__;
#endif
  }

  const char* Branding::GetVersionString()
  {
    // IW4x is technically a beta
    return Utils::String::VA("%s %s build %s %s",
      BUILD_TYPE, "(Beta)", GetBuildNumber(), reinterpret_cast<const char*>(0x140389578));
  }

  void Branding::Dvar_SetVersionString(const Game::dvar_t* dvar, [[maybe_unused]] const char* value)
  {
    Game::Dvar_SetString(dvar, GetVersionString());
  }

  // Branding this might be a good idea in case this LSP logging ever gets turned on for some reason
  void Branding::MSG_WriteVersionStringHeader(Game::msg_t* msg, [[maybe_unused]] const char* string)
  {
    Game::MSG_WriteString(msg, GetVersionString());
  }

  Game::dvar_t* Branding::Dvar_RegisterUIBuildLocation(const char* dvarName, [[maybe_unused]] float x,
    [[maybe_unused]] float y, float min, float max, [[maybe_unused]] int flags, const char* description)
  {
    return Game::Dvar_RegisterVec2(dvarName, -60.0f, 472.0f, min, max, Game::DVAR_ROM, description);
  }

  void Branding::RegisterBrandingDvars()
  {
#ifdef _DEBUG
    constexpr auto value = true;
#else
    constexpr auto value = false;
#endif
    Branding::CGDrawVersion = Dvar::Register<bool>("cg_drawVersion", value,
      Game::DVAR_NONE, "Draw the game version");
    Branding::CGDrawVersionX = Dvar::Register<float>("cg_drawVersionX", 10.0f,
      0.0f, 512.0f, Game::DVAR_NONE, "X offset for the version string");
    Branding::CGDrawVersionY = Dvar::Register<float>("cg_drawVersionY", 455.0f,
      0.0f, 512.0f, Game::DVAR_NONE, "Y offset for the version string");
  }

  Branding::Branding()
  {
    RegisterBrandingDvars();

    // UI version string
    //
    // Note that on x64 this is the "%s.%i" format that the UI builds its
    // version from, reached through a 'lea'. So we place our string in its
    // stead, which the format then leaves as is since it has no conversions.
    //
    Utils::Hook::PlaceString(0x14026F26E, 7, 0x1403A290C, "IW4x " REVISION_STR);

    // Short version dvar
    //
    Utils::Hook::PlaceString(0x1401F54FF, 7, 0x140389580, REVISION_STR);

    // Com_Init_Try_Block_Function
    //
    // Note that on x86 we also replace the date in this version line. x64
    // doesn't print one there, it prints the platform ("win64") instead,
    // which seems fine to keep. The build number comes from GetBuildNumber()
    // below.
    //
    Utils::Hook::PlaceString(0x1401F54A5, 7, 0x140387AA8, BUILD_TYPE);
#ifdef EXPERIMENTAL_BUILD
    Utils::Hook::PlaceString(0x1401F5497, 7, 0x140389580, REVISION_STR "-develop");
#else
    Utils::Hook::PlaceString(0x1401F5497, 7, 0x140389580, REVISION_STR);
#endif

    Utils::Hook(0x1401E6960, GetBuildNumber, HOOK_JUMP).install()->quick();

    // Version string color
    //
    constexpr Game::vec4_t buildLocColor = {1.0f, 1.0f, 1.0f, 0.8f};
    Utils::Hook::PlaceData(0x14026F28C, 7, 0x1403A4E30, buildLocColor, sizeof(buildLocColor));

    // Place ui version string to bottom right corner (ui_buildlocation)
    //
    Utils::Hook(0x14026FA90, Dvar_RegisterUIBuildLocation, HOOK_CALL).install()->quick();

    Utils::Hook(0x1401F54ED, Dvar_SetVersionString, HOOK_CALL).install()->quick();

    Utils::Hook(0x1401B15BE, MSG_WriteVersionStringHeader, HOOK_CALL).install()->quick();

    // Draw the version after the debug overlays while connected, which is
    // where x86 draws it (from CG_DrawFullScreenDebugOverlays()).
    //
    // Note that on x64 this is the very UI_PaintMenus() call that Dvar
    // already turns into a call of our Game::CL_DrawDebugOverlays(), and so
    // our hook calls that in turn. This only works because Branding registers
    // after Dvar and so installs last, which is a bit fragile. But the
    // alternative, one hook for both, would tie Dvar's debug info and our
    // version together, which feels worse.
    //
    Utils::Hook(0x140101D0C, CL_DrawDebugOverlays_Hk, HOOK_CALL).install()->quick();

    // Console title
    //
    if (Dedicated::IsEnabled())
    {
#ifdef EXPERIMENTAL_BUILD
      Utils::Hook::PlaceString(0x1402A7BD6, 7, 0x1403A8A20, "IW4x " REVISION_STR "-develop: Dedicated");
#else
      Utils::Hook::PlaceString(0x1402A7BD6, 7, 0x1403A8A20, "IW4x " REVISION_STR ": Dedicated");
#endif
    }
    else
    {
      Utils::Hook::PlaceString(0x1402A7BD6, 7, 0x1403A8A20, "IW4x (" REVISION_STR "): Console");
    }
  }
}
