#include <Utils/Compression.hpp>

#include "Events.hpp"
#include "QuickPatch.hpp"
#include "TextRenderer.hpp"
#include "Toast.hpp"

namespace Components
{
  Game::dvar_t* QuickPatch::g_antilag;

  Dvar::Var QuickPatch::UIMousePitch;

  Dvar::Var QuickPatch::r_customAspectRatio;

  void QuickPatch::UnlockStats()
  {
    if (Dedicated::IsEnabled()) return;

    if (Game::CL_IsCgameInitialized())
    {
      Toast::Show("cardicon_locked", "^1Error", "Not allowed while ingame.", 3000);
      return;
    }

    Command::Execute("setPlayerData prestige 10");
    Command::Execute("setPlayerData experience 2516000");
    Command::Execute("setPlayerData iconUnlocked cardicon_prestige10_02 1");

    // Unlock challenges
    Game::StringTable* challengeTable = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_STRINGTABLE, "mp/allchallengestable.csv").stringTable;

    if (challengeTable)
    {
      for (int i = 0; i < challengeTable->rowCount; ++i)
      {
        // Find challenge
        const char* challenge = Game::TableLookup(challengeTable, i, 0);

        int maxState = 0;
        int maxProgress = 0;

        // Find correct tier and progress
        for (int j = 0; j < 10; ++j)
        {
          int progress = atoi(Game::TableLookup(challengeTable, i, 6 + j * 2));
          if (!progress) break;

          maxState = j + 2;
          maxProgress = progress;
        }

        Command::Execute(Utils::String::VA("setPlayerData challengeState %s %d", challenge, maxState));
        Command::Execute(Utils::String::VA("setPlayerData challengeProgress %s %d", challenge, maxProgress));
      }
    }
  }

  Game::dvar_t* QuickPatch::Dvar_RegisterAspectRatioDvar(const char* dvarName, const char** /*valueList*/, int defaultIndex, unsigned __int16 flags, const char* description)
  {
    static const char* r_aspectRatioEnum[] =
    {
      "auto",
      "standard",
      "wide 16:10",
      "wide 16:9",
      "custom",
      nullptr
    };

    // register custom aspect ratio dvar
    QuickPatch::r_customAspectRatio = Dvar::Register<float>("r_customAspectRatio",
      16.0f / 9.0f, 4.0f / 3.0f, 63.0f / 9.0f, flags,
      "Screen aspect ratio. Divide the width by the height in order to get the aspect ratio value. For example: 16 / 9 = 1,77");

    // register enumeration dvar
    return Game::Dvar_RegisterEnum(dvarName, r_aspectRatioEnum, defaultIndex, flags, description);
  }

  void QuickPatch::SetAspectRatio()
  {
    // set the aspect ratio
    Utils::Hook::Set<float>(0x148CCA798, r_customAspectRatio.get<float>());
  }

  void QuickPatch::FireWeapon_Hk(Game::gentity_s* ent, int gameTime, int eventParm)
  {
    if (!g_antilag->current.enabled)
    {
      gameTime = Game::level->time;
    }

    Utils::Hook::Call<void(Game::gentity_s*, int, int)>(0x1401877D0)(ent, gameTime, eventParm);
  }

  void QuickPatch::FireWeaponMelee_Hk(Game::gentity_s* ent, int gameTime)
  {
    if (!g_antilag->current.enabled)
    {
      gameTime = Game::level->time;
    }

    Utils::Hook::Call<void(Game::gentity_s*, int)>(0x140187DC0)(ent, gameTime);
  }

  void QuickPatch::CL_KeyEvent_OnEscape()
  {
    if (Game::Con_CancelAutoComplete())
    {
      return;
    }

    if (TextRenderer::HandleFontIconAutocompleteKey(0, TextRenderer::FONT_ICON_ACI_CONSOLE, Game::K_ESCAPE))
    {
      return;
    }

    // Close console
    Game::Key_RemoveCatcher(0, ~Game::KEYCATCH_CONSOLE);
  }

  void QuickPatch::R_AddImageToList_Hk(Game::XAssetHeader header, void* data)
  {
    auto* imageList = static_cast<Game::ImageList*>(data);

    assert(imageList->count < ARRAYSIZE(imageList->image));

    if (header.image->texture.basemap)
    {
      imageList->image[imageList->count++] = header.image;
    }
  }

  int QuickPatch::SND_GetAliasOffset(const Game::snd_alias_t* alias)
  {
    // The game looks up the alias's list by its name and walks it without
    // checking that it found one. Note that x64 takes a default asset for
    // none, which we do as well.
    //
    const auto* list = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_SOUND, alias->aliasName).sound;

    if (Game::DB_IsXAssetDefault(Game::ASSET_TYPE_SOUND, alias->aliasName))
    {
      list = nullptr;
    }

    if (!list)
    {
      Game::Com_Error(Game::ERR_DROP, "SND_GetAliasOffset: Could not find sound alias '%s'", alias->aliasName);
      return 0;
    }

    for (auto i = 0u; i < list->count; ++i)
    {
      if (&list->head[i] == alias)
      {
        return static_cast<int>(i);
      }
    }

    return 0;
  }

  void QuickPatch::Sys_SpawnQuitProcess_Hk()
  {
    if (*Game::sys_exitCmdLine[0] == '\0')
    {
      return;
    }

    const std::filesystem::path workingDir = std::filesystem::current_path();
    const std::wstring binary = Utils::String::Convert(*Game::sys_exitCmdLine);
    const std::wstring commandLine = std::format(L"\"{}\" iw4x --pass \"{}\"", (workingDir / binary).wstring(), Utils::GetLaunchParameters());

    SetEnvironmentVariableA("MW2_INSTALL", workingDir.string().data());
    Utils::Library::LaunchProcess(binary, commandLine, workingDir);
  }

  int QuickPatch::FX_ImpactMarkCallback_Hk(void* context, int pointCount, void* points, int triCount, void* tris, const float* origin, const float* texCoordAxis)
  {
    // Skip the mark if there is no room for it.
    //
    // To make room, the game frees the least recently drawn marks until it
    // has the point and triangle groups as well as the mark it needs. But
    // there are marks it never frees this way (of type 3 with the 0x80 flag
    // set, which it caps at 16 per slot instead) and, if only such marks are
    // left while the groups are still short, it goes on to free a null mark
    // and crashes. So we check that the free groups plus those it can free
    // cover this mark and drop the mark otherwise, the same as the game does
    // when it hits that cap.
    //
    // Note that a point group holds one or two points, so the point count
    // is an upper bound of the groups needed, while a triangle group always
    // holds two triangles.
    //
    constexpr auto markCount = 512;
    constexpr auto noHandle = 0xFFFF;

    const auto& system = *Game::fx_marksSystemPool;

    auto pointsNeeded = pointCount;
    auto trisNeeded = (triCount + 1) / 2;
    auto marksNeeded = system.firstFreeMarkHandle == noHandle ? 1 : 0;

    for (auto* group = system.firstFreePointGroup; group && pointsNeeded > 0; group = group->nextFree) --pointsNeeded;
    for (auto* group = system.firstFreeTriGroup; group && trisNeeded > 0; group = group->nextFree) --trisNeeded;

    for (const auto& mark : std::span(system.marks, markCount))
    {
      if (pointsNeeded <= 0 && trisNeeded <= 0 && marksNeeded <= 0)
      {
        break;
      }

      // Skip free marks and those the game never frees to make room.
      //
      if (mark.frameCountDrawn == -1 || (mark.context[0] == 3 && (mark.context[4] & 0x80)))
      {
        continue;
      }

      --marksNeeded;
      for (auto i = mark.points; i != noHandle; i = system.pointGroups[i].item.next) --pointsNeeded;
      for (auto i = mark.tris; i != noHandle; i = system.triGroups[i].item.next) --trisNeeded;
    }

    if (pointsNeeded > 0 || trisNeeded > 0 || marksNeeded > 0)
    {
      return 0;
    }

    return Game::FX_ImpactMarkCallback(context, pointCount, points, triCount, tris, origin, texCoordAxis);
  }

  Game::dvar_t* QuickPatch::Dvar_RegisterScoreboardPingText(const char* dvarName, [[maybe_unused]] bool value, [[maybe_unused]] unsigned __int16 flags, const char* description)
  {
    return Game::Dvar_RegisterBool(dvarName, true, Game::DVAR_CHEAT, description);
  }

#ifdef DEBUG
  Game::dvar_t* QuickPatch::Dvar_RegisterDeveloper(const char* dvarName, [[maybe_unused]] int value, int min, int max, unsigned __int16 flags, const char* description)
  {
    return Game::Dvar_RegisterInt(dvarName, 2, min, max, flags, description);
  }

  Game::dvar_t* QuickPatch::Dvar_RegisterDeveloperScript(const char* dvarName, [[maybe_unused]] bool value, unsigned __int16 flags, const char* description)
  {
    return Game::Dvar_RegisterBool(dvarName, true, flags, description);
  }
#endif

  void QuickPatch::CL_InitRef_Hk(Game::GfxConfiguration* config)
  {
    // CL_InitRef() creates a GfxConfiguration with the fullscreen display
    // mode, which r_displayMode inherits as the default value. We override
    // the field here to force windowed startup on first launch.
    //
    // Note that on x86 the field is a bool that r_fullscreen inherits. x64
    // has the r_displayMode enum in its place, of which 2 is "windowed".
    //
    config->defaultDisplayMode = 2;

    // Call original R_ConfigureRenderer()
    return Utils::Hook::Call<void(Game::GfxConfiguration* config)>(0x1400334B0)(config);
  }

  void QuickPatch::R_EnumDisplayModes_Hk(unsigned int adapterIndex)
  {
    // Call original R_EnumDisplayModes() to let it register and save available display modes in r_mode
    Utils::Hook::Call<void(unsigned int adapterIndex)>(0x1400316D0)(adapterIndex);

    if (Dvar::Var("g_firstLaunch").get<bool>() == false)
    {
      return;
    }

    if (!(*Game::d3d9))
    {
      return;
    }

    // Get the resolution of the monitor that will be used for the game
    HMONITOR adapterMonitor = (*Game::d3d9)->GetAdapterMonitor(adapterIndex);
    MONITORINFO mi{};
    mi.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoA(adapterMonitor, &mi))
    {
      return;
    }

    const int monitor_width  = mi.rcMonitor.right  - mi.rcMonitor.left;
    const int monitor_height = mi.rcMonitor.bottom - mi.rcMonitor.top;

    Game::dvar_t* r_mode = Game::Dvar_FindVar("r_mode");

    if (!r_mode || !r_mode->domain.enumeration.strings || r_mode->domain.enumeration.stringCount <= 0)
    {
      return;
    }

    int mode_index  = r_mode->current.integer;
    int mode_width  = 0;
    int mode_height = 0;

    for (int i = 0; i < r_mode->domain.enumeration.stringCount; i++)
    {
      const char* mode = r_mode->domain.enumeration.strings[i];

      if (std::sscanf(mode, "%ix%i", &mode_width, &mode_height) == 2)
      {
        if (mode_width == monitor_width && mode_height == monitor_height)
        {
          mode_index = i;
          break;
        }
      }
    }

    Game::Dvar_SetInt(r_mode, mode_index);

    // Move the window to the top-left corner for a fullscreen-like appearance
    Game::dvar_t* vid_xpos = Game::Dvar_FindVar("vid_xpos");
    Game::dvar_t* vid_ypos = Game::Dvar_FindVar("vid_ypos");
    if (vid_xpos) Game::Dvar_SetInt(vid_xpos, 0);
    if (vid_ypos) Game::Dvar_SetInt(vid_ypos, 0);
  }

  QuickPatch::QuickPatch()
  {
    // Note that on x86 we also filter the entities of the CODO maps that are
    // meant for Spec Ops, which only the zones that Zones loads have. The x64
    // build doesn't have Zones.
    //

    // Hook escape handling on open console to change behaviour to close the
    // console instead of only canceling autocomplete.
    //
    // Note that x64 compiles out the call to Con_CancelAutoComplete() here,
    // which the Console component puts back with its own stub. We take the
    // place over from it.
    //
    Utils::Hook(0x1400EE95D, QuickPatch_CL_KeyEvent_ConsoleEscape_Stub, HOOK_JUMP).install()->quick();

    // Intermission time dvar
    Game::Dvar_RegisterFloat("scr_intermissionTime", 10, 0, 120, Game::DVAR_NONE, "Time in seconds before match server loads the next map");

    // Note that x64 passes the client events to FireWeapon() and
    // FireWeaponMelee() as tail calls, which we take over as they are.
    //
    g_antilag = Game::Dvar_RegisterBool("g_antilag", true, Game::DVAR_CODINFO, "Perform antilag");
    Utils::Hook(0x140194EBF, FireWeapon_Hk, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140194EDA, FireWeaponMelee_Hk, HOOK_JUMP).install()->quick();

    // Fix out-of-bounds crash in the vehicle effects (0x14029A314)
    //
    // The problem here is that vehicle playerIndex is packed and transmitted
    // as a 5-bit field, which naturally allows for values up to 31 whereas the
    // client array is strictly bounded by MAX_CLIENTS (18).
    //
    // https://github.com/iw4x/iw4x-client/issues/285#issuecomment-3458190361
    //
    // For whatever reason, the original implementation blindly uses this
    // received value as a direct array index, and so we (in some situation)
    // end up with an out-of-bounds memory read/write.
    //
    // NOTE:
    //
    // This is a tentative fix intended to finally address the issue. It may or
    // may not fully resolve the problem depending on underlying conditions not
    // yet accounted for.
    //
    Utils::Hook(0x14029A30D, QuickPatch_VehicleFx_PlayerIndexCheck_Stub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x14029A312, 10);
    Utils::Hook(0x140296DE8, QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x140296DED, 9);
    Utils::Hook(0x1402967AC, QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub, HOOK_CALL).install()->quick();
    Utils::Hook::Nop(0x1402967B1, 4);

    // Fix crash when impact marks run out of room.
    //
    // Note that x64 has a second copy of the caller that passes the callback
    // as well.
    //
    for (const auto place : {0x140151E8C, 0x1401521D5})
    {
      Utils::Hook::Displace(place, 7, 0x140152570, reinterpret_cast<uintptr_t>(Utils::Hook::Relay(place + 7, reinterpret_cast<void*>(QuickPatch::FX_ImpactMarkCallback_Hk))));
    }

    // Add ultrawide support
    Utils::Hook(0x14002FF75, QuickPatch::Dvar_RegisterAspectRatioDvar, HOOK_CALL).install()->quick();
    Utils::Hook(0x140031E8D, QuickPatch_SetAspectRatio_Stub, HOOK_JUMP).install()->quick();

    // Disable fullscreen mode on first launch
    //
    // Note that x64 inlines CL_InitRef() into CL_Vid_Restart_f(), so we hook
    // both of its calls to R_ConfigureRenderer().
    //
    Utils::Hook(0x1400FBD1D, QuickPatch::CL_InitRef_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400FDE44, QuickPatch::CL_InitRef_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x140032772, QuickPatch::R_EnumDisplayModes_Hk, HOOK_CALL).install()->quick();

    // Note that x64 compiles out con_minicon along with the mini console, so
    // we register it as the game's CG_RegisterDvars() does on x86 (see
    // Game::Con_DrawMiniConsole()). As there, we default it to on in debug
    // builds.
    //
    Events::OnDvarInit([]
    {
#ifdef _DEBUG
      constexpr auto value = true;
#else
      constexpr auto value = false;
#endif
      Game::con_minicon = Game::Dvar_RegisterBool("con_minicon", value, Game::DVAR_ARCHIVE, "Display the mini console on screen");
    });

    // Note that x64 has two copies of the imagelist command's walk over the
    // images.
    //
    for (const auto place : {0x1400382FD, 0x14003836C})
    {
      Utils::Hook::Displace(place, 7, 0x140037640, reinterpret_cast<uintptr_t>(Utils::Hook::Relay(place + 7, reinterpret_cast<void*>(QuickPatch::R_AddImageToList_Hk))));
    }

    Utils::Hook::PlaceString(0x1400FE0F0, 7, 0x140377EC8, "iw4x-sp.exe");
    Utils::Hook(0x1402A3DBD, QuickPatch::Sys_SpawnQuitProcess_Hk, HOOK_CALL).install()->quick();

    // Fix crash as nullptr goes unchecked
    //
    AssertSize(Game::snd_alias_t, 0x88);
    Utils::Hook(0x14027F090, QuickPatch::SND_GetAliasOffset, HOOK_JUMP).install()->quick();

    // remove system pre-init stuff (improper quit, disk full)
    //
    // Note that WinMain() goes on only if this returns nonzero, which on x86
    // we leave to whatever eax holds. We return 1.
    //
    Utils::Hook::Set<BYTE>(0x1402A2E00, 0xB8); // mov eax, 1
    Utils::Hook::Set<DWORD>(0x1402A2E01, 1);
    Utils::Hook::Set<BYTE>(0x1402A2E05, 0xC3); // ret

    // Don't delete config files if corrupted
    //
    // Note that on x86 we also clear a flag that the "Invalid external
    // archiving" check sets, which x64 compiles out.
    //
    Utils::Hook::Set<BYTE>(0x1401F3EC6, 0xEB);

    // Don't offer to set optimal settings when the hardware seems to have
    // changed. The game compares it to what the config recorded, so it asks
    // on every first start with a new config (say, of a mod) and its prompt
    // ends up hidden behind the splash screen, which looks like a hang. Note
    // that the game still records the new hardware, we only skip asking.
    //
    // Note that x64 inlines the prompt into the check, so we return false
    // where it starts.
    //
    Utils::Hook::Set<DWORD>(0x1402A4056, 0x2EEBC030); // xor al, al; jmp 0x1402A4088

    // hopefully allow alt-tab during game, used at least in alt-enter handling
    Utils::Hook::Set<BYTE>(0x1402A2B50, 0xB0); // mov al, 1
    Utils::Hook::Set<BYTE>(0x1402A2B51, 1);
    Utils::Hook::Set<BYTE>(0x1402A2B52, 0xC3); // ret

    // fs_basegame
    Utils::Hook::PlaceString(0x14027638F, 7, 0x1403C9110, BASEGAME);

    // window title
    Utils::Hook::PlaceString(0x140032832, 7, 0x1403DEBC0, "IW4x: Multiplayer");

    // sv_hostname
    Utils::Hook::PlaceString(0x14023897A, 7, 0x14039D290, "IW4Host");

    // console logo
    Utils::Hook::PlaceString(0x1402A7CB5, 7, 0x1403A8A40, BASEGAME "/images/logo.bmp");

    // splash logo
    Utils::Hook::PlaceString(0x1402A6E87, 7, 0x1403A8978, BASEGAME "/images/splash.bmp");

    // Numerical ping (cg_scoreboardPingText 1)
    //
    // Note that x64 passes the flags in all of r8d, so we register the dvar
    // ourselves.
    //
    Utils::Hook(0x1400E56B4, QuickPatch::Dvar_RegisterScoreboardPingText, HOOK_CALL).install()->quick();

    // increase font sizes for chat on higher resolutions
    constexpr auto float13 = 13.0f;
    constexpr auto float10 = 10.0f;
    Utils::Hook::PlaceData(0x1400D1322, 9, 0x1403E2CD4, &float13, sizeof(float13));
    Utils::Hook::PlaceData(0x1400D134B, 9, 0x1403E2CD0, &float10, sizeof(float10));

    // Enable commandline arguments
    //
    // Note that x64 inlines the check into Com_Init().
    //
    Utils::Hook::Set<BYTE>(0x1401F501E, 0xEB);

    // Note that x64 drops the connection to a server that sends no list of
    // its iwds, as IW4x servers with sv_pure 0 do, with 'Server has modified
    // files', and to one whose referenced iwds we lack or hold in another
    // version with '<iwd> is different from the server'. x86 has neither
    // check, and an x86 server references its own iwds, which an x64 client
    // never has. So we make the jumps past both errors unconditional.
    //
    Utils::Hook::Set<BYTE>(0x1401002B5, 0xEB); // CL_ParseGamestate
    Utils::Hook::Set<BYTE>(0x1400F88D1, 0xEB); // CL_ParseGamestate (its iwd check)

    // remove limit on IWD file loading
    Utils::Hook::Nop(0x14027338F, 2);

    // Note that x64 also loads an iwd that isn't localized only if it is
    // named iw_NN.iwd with NN at most 23, which x86 doesn't check and which
    // would leave out ours. So we jump over that check to where it loads
    // the iwd.
    //
    Utils::Hook::Set<BYTE>(0x14027357F, 0xEB); // jmp 0x1402735FD
    Utils::Hook::Set<BYTE>(0x140273580, 0x1402735FD - (0x14027357F + 2));

    // Fix stats sleeping
    //
    // Note that on x86 we return from the DNS lookup of the stats server,
    // into which x64 inlines the frame that follows it. We skip to that.
    //
    Utils::Hook::Set<BYTE>(0x14024B307, 0xEB);
    Utils::Hook(0x1401ADD54, 0x1401ADEC8, HOOK_JUMP).install()->quick();

    // Note that on x86 we also remove the 'impure stats' check, default
    // sv_pure to 0, remove activeAction execution, and disable bind
    // protection. x64 compiles out the stats check and has none of the rest.
    //

    // require Windows 6 (Vista)
    Utils::Hook::Set<BYTE>(0x1402A3B1B, 6);
    Utils::Hook::Set<char>(0x1403A8496, '6');

    // Note that on x86 we also disable the 'ignoring asset' notices, which
    // x64 doesn't print.
    //

    // disable migration_dvarErrors
    Utils::Hook::Set<BYTE>(0x1401F552E, 0);

    // allow joining 'developer 1' servers
    Utils::Hook::Set<BYTE>(0x140196156, 0xEB);

    // fs_game fixes
    //
    // Note that on x86 we make G_GetWeaponIndexForName() load the weapons
    // from the fastfiles with fs_game set too, where the game loads them
    // from the raw files. x64 has no raw file loading and always does so in
    // BG_GetWeaponIndexForName().
    //

    // filesystem init default_mp.cfg check
    Utils::Hook::Nop(0x1402765E5, 5);
    Utils::Hook::Nop(0x1402765F6, 5);
    Utils::Hook::Set<WORD>(0x1402765FD, 0xE990);

    // vid_restart when ingame
    Utils::Hook::Nop(0x1400FDB85, 6);

    // Note that on x86 we also filter the 'dvar set' and 'sending splash
    // open' lines out of the log, which x64 compiles out.
    //

    // intro stuff
    Utils::Hook::Nop(0x1401F568B, 5); // Don't show legals
    Utils::Hook::Nop(0x1401F5699, 5); // Don't reset the intro dvar
    Utils::Hook::Displace(0x1401F566F, 7, 0x14038B2E0, 0x14038B2E4); // "cinematic IW_logo\n"
    Utils::Hook::PlaceString(0x140035C6C, 7, 0x1403DED28, "%s/" BASEGAME "/video/%s.bik");
    Utils::Hook::Displace(0x140035CAA, 7, 0x1403DED40, 0x1403DED28);

    // Redirect logs
    //
    // Note that x64 compiles out the console log, which we have in its
    // place.
    //
    Utils::Hook::PlaceString(0x14019D8FD, 7, 0x140386E68, "logs/games_mp.log");
    Game::logFileName = "logs/console_mp.log";

    // Rename config
    for (const auto place : {0x1401E7216, 0x1401F3ECF, 0x1401F3F4C, 0x1401F51A7, 0x1402766A7})
    {
      Utils::Hook::PlaceString(place, 7, 0x14038ABB0, CLIENT_CONFIG);
    }

    // Disable profile system
    //
    // Note that on x86 the patch for GamerProfile_SetPercentCompleteMP is at
    // the fadein menu script handler, which we patch the same.
    //
    // @@ That likely meant 0x450A80 (0x1400F15B0 on x64), which only saves
    //    through GamerProfile_SaveProfile.
    //
    Utils::Hook::Nop(0x1400FBC10, 5); // GamerProfile_RegisterDvars
    Utils::Hook::Nop(0x1401F439A, 5); // GamerProfile_UpdateSystemDvars
    Utils::Hook::Set<BYTE>(0x1400F1190, 0xC3); // GamerProfile_SaveProfile
    Utils::Hook::Set<BYTE>(0x1400F1820, 0xC3); // GamerProfile_UpdateSystemVarsFromProfile
    Utils::Hook::Set<BYTE>(0x1400F0CD0, 0xC3); // GamerProfile_UpdateProfileAndSaveIfNeeded
    Utils::Hook::Set<BYTE>(0x14025AC60, 0xC3); // fadein

    // Profile reading
    //
    // Note that x64 inlines the reading into GamerProfile_LogInProfile() and
    // doesn't print its error. On x86 the reading fails, which marks the
    // profile unread and skips parsing it. We do the same and skip to where
    // it leaves the critical section.
    //
    Utils::Hook::Set<DWORD>(0x1400F0B81, 0x0146C641); // mov byte ptr [r14+1], 1
    Utils::Hook::Set<BYTE>(0x1400F0B85, 1);
    Utils::Hook(0x1400F0B86, 0x1400F0C73, HOOK_JUMP).install()->quick();

    // GamerProfile_RegisterCommands
    //
    // Note that x64 has 10 of the 15 profile commands, which we point to
    // nullsub_1.
    //
    for (const auto& [place, handler] : {std::pair{0x1400F0F6B, 0x1400F0D40}, // profile_menuDvarsSetup
                                         std::pair{0x1400F0F85, 0x1400F0CD0}, // profile_menuDvarsFinish
                                         std::pair{0x1400F0F9F, 0x1400F1870}, // profile_toggleInvertedPitch
                                         std::pair{0x1400F0FB9, 0x1400F2100}, // profile_setViewSensitivity
                                         std::pair{0x1400F0FD3, 0x1400F2060}, // profile_setButtonsConfig
                                         std::pair{0x1400F0FED, 0x1400F20C0}, // profile_setSticksConfig
                                         std::pair{0x1400F1007, 0x1400F03D0}, // profile_toggleAutoAim
                                         std::pair{0x1400F1021, 0x1400F20A0}, // profile_SetHasEverPlayed_MainMenu
                                         std::pair{0x1400F103B, 0x1400F2160}, // profile_setVolume
                                         std::pair{0x1400F1055, 0x1400F21D0}}) // profile_toggleCanSkipOffensiveMissions
    {
      Utils::Hook::Displace(place, 7, handler, 0x140080E30);
    }

    // Patch SV_IsClientUsingOnlineStatsOffline
    Utils::Hook::Set<BYTE>(0x140236F00, 0x33); // xor eax, eax
    Utils::Hook::Set<BYTE>(0x140236F01, 0xC0);
    Utils::Hook::Set<BYTE>(0x140236F02, 0xC3); // ret

    // Fix mouse lag
    Utils::Hook::Nop(0x1402A90C8, 6);
    Scheduler::Loop([]
    {
      SetThreadExecutionState(ES_DISPLAY_REQUIRED);
    }, Scheduler::Pipeline::RENDERER);

    // Fix mouse pitch adjustments
    UIMousePitch = Dvar::Register<bool>("ui_mousePitch", false, Game::DVAR_ARCHIVE, "");
    UIScript::Add("updateui_mousePitch", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
    {
      if (UIMousePitch.get<bool>())
      {
        Game::Dvar_SetFloatByName("m_pitch", -0.022f);
      }
      else
      {
        Game::Dvar_SetFloatByName("m_pitch", 0.022f);
      }
    });

    Command::Add("unlockstats", QuickPatch::UnlockStats);

    // Note that on x86 we also have the dumptechsets command, which writes
    // shaders and techsets in the x86 zone layout for ZoneBuilder. The x64
    // build doesn't have ZoneBuilder.
    //

#ifdef DEBUG_MAT_LOG
    AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, const std::string& /*name*/, bool* /*restrict*/)
    {
      if (type == Game::XAssetType::ASSET_TYPE_GFXWORLD)
      {
        std::string buffer;

        for (unsigned int i = 0; i < asset.gfxWorld->dpvs.staticSurfaceCount; ++i)
        {
          buffer.append(Utils::String::VA("%s\n", asset.gfxWorld->dpvs.surfaces[asset.gfxWorld->dpvs.sortedSurfIndex[i]].material->info.name));
        }

        Utils::IO::WriteFile("userraw/logs/matlog.txt", buffer);
      }
    });
#endif

    // Debug patches
#ifdef DEBUG
    // ui_debugMode 1
    //Utils::Hook::Set<bool>(0x6312E0, true);

    // developer 2
    //
    // Note that x64 has the default value in all of edx, so we register the
    // dvars ourselves.
    //
    Utils::Hook(0x14002FE74, QuickPatch::Dvar_RegisterDeveloper, HOOK_CALL).install()->quick();
    Utils::Hook(0x1400D8EC2, QuickPatch::Dvar_RegisterDeveloper, HOOK_CALL).install()->quick();
    Utils::Hook(0x1401F4A2A, QuickPatch::Dvar_RegisterDeveloper, HOOK_CALL).install()->quick();

    // developer_Script 1
    Utils::Hook(0x1401F4A49, QuickPatch::Dvar_RegisterDeveloperScript, HOOK_CALL).install()->quick();

    // Disable cheat protection for dvars
    //
    // Note that x64 inlines Dvar_IsCheatProtected() into Dvar_SetVariant().
    //
    Utils::Hook::Set<BYTE>(0x1402860CF, 0xEB);
#else
    // Note that on x86 we also remove the missing tag message, which x64
    // compiles out.
    //
#endif

    if (Flags::HasFlag("nointro"))
    {
      Utils::Hook::Set<BYTE>(0x1401F566D, 0xEB);
    }
  }
}

void QuickPatch_SetAspectRatio()
{
  Components::QuickPatch::SetAspectRatio();
}

void QuickPatch_CL_KeyEvent_OnEscape()
{
  Components::QuickPatch::CL_KeyEvent_OnEscape();
}
