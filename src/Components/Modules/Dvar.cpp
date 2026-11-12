#include "Dvar.hpp"
#include "Friends.hpp"
#include "TextRenderer.hpp"

namespace Components
{
  Dvar::Var Dvar::Name;

  Dvar::Var::Var(const std::string& dvarName)
    : dvar_(Game::Dvar_FindVar(dvarName.data()))
  {
    // If the dvar can't be found it will be registered as an empty string dvar
    if (!this->dvar_)
    {
      this->dvar_ = const_cast<Game::dvar_t*>(Game::Dvar_SetFromStringByNameFromSource(dvarName.data(), "", Game::DVAR_SOURCE_INTERNAL));
    }
  }

  template <> Game::dvar_t* Dvar::Var::get()
  {
    return this->dvar_;
  }

  template <> const char* Dvar::Var::get()
  {
    if (!this->dvar_)
    {
      return "";
    }

    if (this->dvar_->type == Game::DVAR_TYPE_STRING || this->dvar_->type == Game::DVAR_TYPE_ENUM)
    {
      if (this->dvar_->current.string)
      {
        return this->dvar_->current.string;
      }
    }

    return "";
  }

  template <> int Dvar::Var::get()
  {
    if (!this->dvar_)
    {
      return 0;
    }

    if (this->dvar_->type == Game::DVAR_TYPE_INT || this->dvar_->type == Game::DVAR_TYPE_ENUM)
    {
      return this->dvar_->current.integer;
    }

    return 0;
  }

  template <> unsigned int Dvar::Var::get()
  {
    if (!this->dvar_)
    {
      return 0;
    }

    if (this->dvar_->type == Game::DVAR_TYPE_INT)
    {
      return this->dvar_->current.unsignedInt;
    }

    return 0;
  }

  template <> float Dvar::Var::get()
  {
    if (!this->dvar_)
    {
      return 0.f;
    }

    if (this->dvar_->type == Game::DVAR_TYPE_FLOAT)
    {
      return this->dvar_->current.value;
    }

    return 0.f;
  }

  template <> bool Dvar::Var::get()
  {
    if (!this->dvar_)
    {
      return false;
    }

    if (this->dvar_->type == Game::DVAR_TYPE_BOOL)
    {
      return this->dvar_->current.enabled;
    }

    return false;
  }

  template <> std::string Dvar::Var::get()
  {
    return this->get<const char*>();
  }

  void Dvar::Var::set(const char* string)
  {
    assert(string);
    assert(this->dvar_->type == Game::DVAR_TYPE_STRING);

    if (this->dvar_)
    {
      Game::Dvar_SetString(this->dvar_, string);
    }
  }

  void Dvar::Var::set(const std::string& string)
  {
    this->set(string.data());
  }

  void Dvar::Var::set(int integer)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_INT);

    if (this->dvar_)
    {
      Game::Dvar_SetInt(this->dvar_, integer);
    }
  }

  void Dvar::Var::set(float value)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

    if (this->dvar_)
    {
      Game::Dvar_SetFloat(this->dvar_, value);
    }
  }

  void Dvar::Var::set(bool enabled)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

    if (this->dvar_)
    {
      Game::Dvar_SetBool(this->dvar_, enabled);
    }
  }

  void Dvar::Var::setRaw(int integer)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_INT);

    if (this->dvar_)
    {
      this->dvar_->current.integer = integer;
      this->dvar_->latched.integer = integer;
    }
  }

  void Dvar::Var::setRaw(float value)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

    if (this->dvar_)
    {
      this->dvar_->current.value = value;
      this->dvar_->latched.value = value;
    }
  }

  void Dvar::Var::setRaw(bool enabled)
  {
    assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

    if (this->dvar_)
    {
      this->dvar_->current.enabled = enabled;
      this->dvar_->latched.enabled = enabled;
    }
  }

  template<> Dvar::Var Dvar::Register(const char* dvarName, bool value, std::uint16_t flag, const char* description)
  {
    return Game::Dvar_RegisterBool(dvarName, value, flag, description);
  }

  template<> Dvar::Var Dvar::Register(const char* dvarName, const char* value, std::uint16_t flag, const char* description)
  {
    return Game::Dvar_RegisterString(dvarName, value, flag, description);
  }

  template<> Dvar::Var Dvar::Register(const char* dvarName, int value, int min, int max, std::uint16_t flag, const char* description)
  {
    return Game::Dvar_RegisterInt(dvarName, value, min, max, flag, description);
  }

  template<> Dvar::Var Dvar::Register(const char* dvarName, float value, float min, float max, std::uint16_t flag, const char* description)
  {
    return Game::Dvar_RegisterFloat(dvarName, value, min, max, flag, description);
  }

  const Game::dvar_t* Dvar::Dvar_RegisterName(const char* dvarName, const char* /*value*/, std::uint16_t flags, const char* description)
  {
    // Name watcher
    if (!Dedicated::IsEnabled())
    {
      Scheduler::Loop([]
      {
        static std::string lastValidName = "Unknown Soldier";
        auto name = Name.get<std::string>();

        // Don't perform any checks if name didn't change
        if (name == lastValidName) return;

        Utils::String::Trim(name);
        auto saneName = TextRenderer::StripAllTextIcons(TextRenderer::StripColors(name));
        if (saneName.size() < 3 || (saneName[0] == '[' && saneName[1] == '{'))
        {
          Logger::PrintError(Game::CON_CHANNEL_ERROR, "Username '{}' is invalid. It must at least be 3 characters long and not appear empty!\n", name);
          Name.set(lastValidName);
        }
        else
        {
          lastValidName = name;
          Friends::UpdateName();
        }
      }, Scheduler::Pipeline::CLIENT, 3s); // Don't need to do this every frame
    }

    Name = Register<const char*>(dvarName, "Unknown Soldier", flags | Game::DVAR_ARCHIVE, description);
    return Name.get<Game::dvar_t*>();
  }

  const Game::dvar_t* Dvar::Dvar_RegisterSVNetworkFps(const char* dvarName, int value, int min, int /*max*/, std::uint16_t /*flags*/, const char* description)
  {
    // bump limit up to 1000
    return Game::Dvar_RegisterInt(dvarName, Dedicated::IsEnabled() ? 1000 : value, min, 1000, Game::DVAR_NONE, description);
  }

  const Game::dvar_t* Dvar::Dvar_RegisterDrawFps(const char* dvarName, const char** valueList, int defaultIndex, std::uint16_t flags, const char* description)
  {
    return Game::Dvar_RegisterEnum(dvarName, valueList, defaultIndex, flags | Game::DVAR_ARCHIVE, description);
  }

  const Game::dvar_t* Dvar::Dvar_RegisterFov(const char* dvarName, float value, float min, float /*max*/, std::uint16_t flags, const char* description)
  {
    // Allow the same maximum as single player.
    //
    return Game::Dvar_RegisterFloat(dvarName, value, min, 160.0f, flags, description);
  }

  const Game::dvar_t* Dvar::Dvar_RegisterVolume(const char* dvarName, float /*value*/, float min, float /*max*/, std::uint16_t flags, const char* description)
  {
    return Game::Dvar_RegisterFloat(dvarName, 1.0f, min, 1.0f, flags, description);
  }

  void Dvar::SetFromStringByNameSafeExternal(const char* dvarName, const char* string)
  {
    static std::array exceptions =
    {
      "ui_showEndOfGame",
      "systemlink",
      "splitscreen",
      "onlinegame",
      "party_maxplayers",
      "xblive_privateserver",
      "xblive_rankedmatch",
      "ui_mptype",
    };

    for (const auto& entry : exceptions)
    {
      if (!_stricmp(dvarName, entry))
      {
        Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_INTERNAL);
        return;
      }
    }

    SetFromStringByNameExternal(dvarName, string);
  }

  void Dvar::SetFromStringByNameExternal(const char* dvarName, const char* string)
  {
    Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_EXTERNAL);
  }

  bool Dvar::AreArchiveDvarsUnprotected()
  {
    static std::optional<bool> flag;

    if (!flag.has_value())
    {
      flag.emplace(Flags::HasFlag("unprotect-dvars"));
    }

    return flag.value();
  }

  bool Dvar::IsSettingDvarsDisabled()
  {
    static std::optional<bool> flag;

    if (!flag.has_value())
    {
      flag.emplace(Flags::HasFlag("protect-dvars"));
    }

    return flag.value();
  }

  void Dvar::DvarSetFromStringByName_Stub(const char* dvarName, const char* value)
  {
    if (IsSettingDvarsDisabled())
    {
      Logger::Debug("Not allowing server to set '{}'", dvarName);
      return;
    }

    // Save the dvar original value if it has the archive flag
    const auto* dvar = Game::Dvar_FindVar(dvarName);
    if (dvar && dvar->flags & Game::DVAR_ARCHIVE)
    {
      if (!AreArchiveDvarsUnprotected())
      {
        Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to override saved dvar '{}'\n", dvar->name);
        return;
      }

      Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Server is overriding saved dvar '{}'\n", dvarName);
    }

    if (dvar && std::strcmp(dvar->name, "com_errorResolveCommand") == 0)
    {
      Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to set '{}'\n", dvar->name);
      return;
    }

    Game::Dvar_SetFromStringByName(dvarName, value);
  }

  const char* Dvar::Dvar_EnumToString_Stub(const Game::dvar_t* dvar)
  {
    assert(dvar);
    assert(dvar->name);
    assert(dvar->type == Game::DVAR_TYPE_ENUM);
    assert(dvar->domain.enumeration.strings);
    assert((dvar->current.integer >= 0 && dvar->current.integer < dvar->domain.enumeration.stringCount) || dvar->current.integer == 0);

    // Fix nullptr crash
    if (!dvar || dvar->domain.enumeration.stringCount == 0)
    {
      return "";
    }

    return dvar->domain.enumeration.strings[dvar->current.integer];
  }

  Dvar::Dvar()
  {
    // Set flags of cg_drawFPS to archive.
    //
    // Note that the registration passes its flags with xor r9d, r9d, which
    // leaves no immediate to patch.
    //
    Utils::Hook(0x1400D77C3, Dvar_RegisterDrawFps, HOOK_CALL).install()->quick();

    // un-cheat camera_thirdPersonCrosshairOffset and add archive flags
    Utils::Hook::Xor<std::uint8_t>(0x14008B3E1, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

    // un-cheat cg_fov and add archive flags
    Utils::Hook::Xor<std::uint8_t>(0x1400D7637, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

    // un-cheat cg_fovscale and add archive flags
    Utils::Hook::Xor<std::uint8_t>(0x1400D7680, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

    // un-cheat cg_fovMin and add archive flags
    Utils::Hook::Xor<std::uint8_t>(0x1400D76B7, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

    // un-cheat cg_drawGun
    Utils::Hook::Set<std::uint8_t>(0x1400D754C, Game::DVAR_NONE);

    // re-cheat cg_draw2D to prevent stripping the thermal scope's mask overlay via cg_draw2D 0
    Utils::Hook::Or<std::uint8_t>(0x1400D7702, Game::DVAR_CHEAT);

    // un-cheat cg_overheadNamesFarScale
    Utils::Hook::Set<std::uint8_t>(0x1400D92E8, Game::DVAR_NONE);

    // un-cheat cg_overheadNamesSize
    Utils::Hook::Set<std::uint8_t>(0x1400D931D, Game::DVAR_NONE);

    // un-cheat cg_overheadRankSize
    Utils::Hook::Set<std::uint8_t>(0x1400D9376, Game::DVAR_NONE);

    // un-cheat cg_overheadIconSize
    Utils::Hook::Set<std::uint8_t>(0x1400D9352, Game::DVAR_NONE);

    // un-cheat cg_overheadTitleSize
    Utils::Hook::Set<std::uint8_t>(0x1400D93B6, Game::DVAR_NONE);

    // un-cheat cg_overheadNamesGlow
    Utils::Hook::Set<std::uint8_t>(0x1400D93E3, Game::DVAR_NONE);

    // remove archive flags for cg_hudChatPosition
    Utils::Hook::Xor<std::uint8_t>(0x1400D8350, Game::DVAR_ARCHIVE);

    // remove archive flags for sv_hostname
    Utils::Hook::Xor<std::uint32_t>(0x140238976, Game::DVAR_ARCHIVE);

    // remove write protection from fs_game
    Utils::Hook::Xor<std::uint32_t>(0x1402763B2, Game::DVAR_INIT);

    // Raise cg_fov maximum to 160.
    //
    // Note that the maximum is a RIP-relative load which can't reach a
    // constant of ours, so we register the dvar ourselves.
    //
    Utils::Hook(0x1400D763B, Dvar_RegisterFov, HOOK_CALL).install()->quick();

    // Set profileMenuOption_volume default and maximum to 1.
    //
    Utils::Hook(0x1400F110D, Dvar_RegisterVolume, HOOK_CALL).install()->quick();

    // un-cheat ui_showList and ui_debugMode
    //
    // Note that their flags are lea r8d, [rbx+4] with ebx zero, so we clear
    // the displacement.
    //
    Utils::Hook::Xor<std::uint8_t>(0x14026FADA, Game::DVAR_CHEAT);
    Utils::Hook::Xor<std::uint8_t>(0x14026FD5B, Game::DVAR_CHEAT);

    // un-cheat player_breath_fire_delay
    Utils::Hook::Xor<std::uint32_t>(0x14008BDBF, Game::DVAR_CHEAT);

    // un-cheat player_breath_gasp_scale
    Utils::Hook::Xor<std::uint32_t>(0x14008BDF1, Game::DVAR_CHEAT);

    // un-cheat player_breath_gasp_lerp
    Utils::Hook::Xor<std::uint32_t>(0x14008BE5D, Game::DVAR_CHEAT);

    // un-cheat player_breath_gasp_time
    Utils::Hook::Xor<std::uint32_t>(0x14008BD8F, Game::DVAR_CHEAT);

    // Hook dvar 'name' registration
    Utils::Hook(0x1400FB5E5, Dvar_RegisterName, HOOK_CALL).install()->quick();

    // Hook dvar 'sv_network_fps' registration
    Utils::Hook(0x140238D83, Dvar_RegisterSVNetworkFps, HOOK_CALL).install()->quick();

    // un-cheat safeArea_* and add archive flags
    Utils::Hook::Xor<std::uint32_t>(0x1400F2C6B, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_horizontal
    Utils::Hook::Xor<std::uint32_t>(0x1400F2C9B, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_vertical
    Utils::Hook::Xor<std::uint8_t>(0x1400F2C06, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_horizontal
    Utils::Hook::Xor<std::uint8_t>(0x1400F2C3B, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_vertical

    // Don't allow setting cheat protected dvars via menus
    Utils::Hook(0x140261A75, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
    Utils::Hook(0x140261C5D, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
    Utils::Hook(0x140261FFE, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

    // Script_SetDvar
    Utils::Hook(0x14025BFDA, SetFromStringByNameSafeExternal, HOOK_CALL).install()->quick();

    // Item_HandleKey (yes/no, dvar enum, slider and multi items) and the
    // slider cursor.
    //
    // Note that x64 inlines the per-type key handlers, which share these
    // calls.
    //
    Utils::Hook(0x14025D97A, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
    Utils::Hook(0x14025DAD5, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
    Utils::Hook(0x140267894, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

    // Hook Dvar_SetFromStringByName inside the inlined CG_SetClientDvarFromServer so we can protect dvars
    Utils::Hook(0x1400E6EDC, DvarSetFromStringByName_Stub, HOOK_CALL).install()->quick();

    // Fix crash
    Utils::Hook(0x1402834B0, Dvar_EnumToString_Stub, HOOK_JUMP).install()->quick();

    // Restore the jump dvars that x64 compiles out, with the x86 defaults
    // and limits, and replace the jump movement that reads them.
    //
    // Note that we keep jump_height and jump_slowdownEnable free of the
    // cheat flag, as x86 did.
    //
    constexpr std::uint16_t jumpFlags = Game::DVAR_CHEAT | Game::DVAR_CODINFO | Game::DVAR_INTERNAL;
    constexpr std::uint16_t jumpFreeFlags = Game::DVAR_CODINFO | Game::DVAR_INTERNAL;

    Game::jump_height = Game::Dvar_RegisterFloat("jump_height", 39.0f, 0.0f, 1000.0f, jumpFreeFlags, "The maximum height of a player's jump");
    Game::jump_stepSize = Game::Dvar_RegisterFloat("jump_stepSize", 18.0f, 0.0f, 64.0f, jumpFlags, "The maximum step up to the top of a jump arc");
    Game::jump_slowdownEnable = Game::Dvar_RegisterBool("jump_slowdownEnable", true, jumpFreeFlags, "Slow player movement after jumping");
    Game::jump_ladderPushVel = Game::Dvar_RegisterFloat("jump_ladderPushVel", 128.0f, 0.0f, 1024.0f, jumpFlags, "The velocity of a jump off of a ladder");
    Game::jump_spreadAdd = Game::Dvar_RegisterFloat("jump_spreadAdd", 64.0f, 0.0f, 512.0f, jumpFlags, "The amount of spread scale to add as a side effect of jumping");

    Utils::Hook(0x140087240, Game::Jump_ApplySlowdown, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400872E0, Game::Jump_Check, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140087550, Game::Jump_ClampVelocity, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140087600, Game::Jump_GetStepHeight, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140087660, Game::Jump_IsPlayerAboveMax, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140087680, Game::Jump_GetLandFactor, HOOK_JUMP).install()->quick();
    Utils::Hook(0x1400876E0, Game::Jump_Start, HOOK_JUMP).install()->quick();

    // Restore the aim assist dvars that x64 compiles out, with the x86
    // defaults and limits, and replace the aim assist that reads them.
    //
    // Note that, as x86 did, we raise the perk_extendedMeleeRange maximum
    // and say that aim_lockon_strength is for yaw.
    //
    constexpr std::uint16_t aimSavedFlags = Game::DVAR_CHEAT | Game::DVAR_SAVED;

    Game::aim_aimAssistRangeScale = Game::Dvar_RegisterFloat("aim_aimAssistRangeScale", 1.0f, 0.0f, 2.0f, aimSavedFlags, "Scales the weapon's aim assist range");
    Game::aim_autoAimRangeScale = Game::Dvar_RegisterFloat("aim_autoAimRangeScale", 1.0f, 0.0f, 2.0f, aimSavedFlags, "Scales the weapon's auto aim range");
    Game::aim_input_graph_enabled = Game::Dvar_RegisterBool("aim_input_graph_enabled", true, Game::DVAR_CHEAT, "Use graph for adjusting view input");
    Game::aim_input_graph_debug = Game::Dvar_RegisterBool("aim_input_graph_debug", false, Game::DVAR_CHEAT, "Debug the view input graphs");
    Game::aim_input_graph_index = Game::Dvar_RegisterInt("aim_input_graph_index", 3, 0, 3, Game::DVAR_CHEAT, "Which input graph to use");
    Game::aim_turnrate_pitch = Game::Dvar_RegisterFloat("aim_turnrate_pitch", 90.0f, 0.0f, 1080.0f, Game::DVAR_CHEAT, "The vertical turn rate for aim assist when firing from the hip");
    Game::aim_turnrate_pitch_ads = Game::Dvar_RegisterFloat("aim_turnrate_pitch_ads", 55.0f, 0.0f, 1080.0f, Game::DVAR_CHEAT, "The turn rate up and down for aim assist when aiming down the sight");
    Game::aim_turnrate_yaw = Game::Dvar_RegisterFloat("aim_turnrate_yaw", 260.0f, 0.0f, 1080.0f, Game::DVAR_CHEAT, "The horizontal turn rate for aim assist when firing from the hip");
    Game::aim_turnrate_yaw_ads = Game::Dvar_RegisterFloat("aim_turnrate_yaw_ads", 90.0f, 0.0f, 1080.0f, Game::DVAR_CHEAT, "The horizontal turn rate for aim assist when aiming down the sight");
    Game::aim_accel_turnrate_enabled = Game::Dvar_RegisterBool("aim_accel_turnrate_enabled", true, Game::DVAR_CHEAT, "Enable/disable acceleration of the turnrates");
    Game::aim_accel_turnrate_debug = Game::Dvar_RegisterBool("aim_accel_turnrate_debug", false, Game::DVAR_CHEAT, "Turn on debugging info for the acceleration");
    Game::aim_accel_turnrate_lerp = Game::Dvar_RegisterFloat("aim_accel_turnrate_lerp", 1200.0f, 0.0f, 4000.0f, Game::DVAR_CHEAT, "The acceleration of the turnrates");
    Game::aim_slowdown_enabled = Game::Dvar_RegisterBool("aim_slowdown_enabled", true, Game::DVAR_CHEAT, "Slowdown the turn rate when the cross hair passes over a target");
    Game::aim_slowdown_debug = Game::Dvar_RegisterBool("aim_slowdown_debug", false, Game::DVAR_CHEAT, "Turn on debugging info for aim slowdown");
    Game::aim_slowdown_region_width = Game::Dvar_RegisterFloat("aim_slowdown_region_width", 90.0f, 0.0f, 640.0f, Game::DVAR_CHEAT, "The screen width of the aim slowdown region");
    Game::aim_slowdown_region_height = Game::Dvar_RegisterFloat("aim_slowdown_region_height", 90.0f, 0.0f, 480.0f, Game::DVAR_CHEAT, "The screen height of the aim assist slowdown region");
    Game::aim_slowdown_pitch_scale = Game::Dvar_RegisterFloat("aim_slowdown_pitch_scale", 0.4f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The vertical aim assist slowdown ratio from the hip");
    Game::aim_slowdown_pitch_scale_ads = Game::Dvar_RegisterFloat("aim_slowdown_pitch_scale_ads", 0.5f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The vertical aim assist slowdown ratio when aiming down the sight");
    Game::aim_slowdown_yaw_scale = Game::Dvar_RegisterFloat("aim_slowdown_yaw_scale", 0.4f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The horizontal aim assist slowdown ratio from the hip");
    Game::aim_slowdown_yaw_scale_ads = Game::Dvar_RegisterFloat("aim_slowdown_yaw_scale_ads", 0.5f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The horizontal aim assist slowdown ratio when aiming down the sight");
    Game::aim_autoaim_enabled = Game::Dvar_RegisterBool("aim_autoaim_enabled", false, Game::DVAR_CHEAT, "Turn on auto aim");
    Game::aim_autoaim_debug = Game::Dvar_RegisterBool("aim_autoaim_debug", false, Game::DVAR_CHEAT, "Turn on auto aim debugging");
    Game::aim_autoaim_lerp = Game::Dvar_RegisterFloat("aim_autoaim_lerp", 40.0f, 0.0f, 100.0f, Game::DVAR_CHEAT, "The rate in degrees per second that the auto aim will converge to its target");
    Game::aim_autoaim_region_width = Game::Dvar_RegisterFloat("aim_autoaim_region_width", 160.0f, 0.0f, 640.0f, Game::DVAR_CHEAT, "The width of the auto aim region in virtual screen coordinates (0 - 640)");
    Game::aim_autoaim_region_height = Game::Dvar_RegisterFloat("aim_autoaim_region_height", 120.0f, 0.0f, 480.0f, Game::DVAR_CHEAT, "The height of the auto aim region in virtual screen coordinates (0 - 480)");
    Game::aim_automelee_enabled = Game::Dvar_RegisterBool("aim_automelee_enabled", true, Game::DVAR_CHEAT, "Turn on auto melee");
    Game::aim_automelee_debug = Game::Dvar_RegisterBool("aim_automelee_debug", false, Game::DVAR_CHEAT, "Turn on auto melee debugging");
    Game::aim_automelee_lerp = Game::Dvar_RegisterFloat("aim_automelee_lerp", 40.0f, 0.0f, 100.0f, Game::DVAR_CHEAT, "The rate in degrees per second that the auto melee will converge to its target");
    Game::aim_automelee_region_width = Game::Dvar_RegisterFloat("aim_automelee_region_width", 320.0f, 0.0f, 640.0f, Game::DVAR_CHEAT, "The width of the auto melee region in virtual screen coordinates (0 - 640)");
    Game::aim_automelee_region_height = Game::Dvar_RegisterFloat("aim_automelee_region_height", 240.0f, 0.0f, 480.0f, Game::DVAR_CHEAT, "The height of the auto melee region in virtual screen coordinates (0 - 480)");
    Game::aim_automelee_range = Game::Dvar_RegisterFloat("aim_automelee_range", 128.0f, 0.0f, 255.0f, Game::DVAR_CHEAT, "The range of the auto melee");
    Game::aim_lockon_enabled = Game::Dvar_RegisterBool("aim_lockon_enabled", true, Game::DVAR_CHEAT, "Aim lock on helps the player to stay on target");
    Game::aim_lockon_debug = Game::Dvar_RegisterBool("aim_lockon_debug", false, Game::DVAR_CHEAT, "Turn on debugging info for aim lock on");
    Game::aim_lockon_deflection = Game::Dvar_RegisterFloat("aim_lockon_deflection", 0.05f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The amount of stick deflection for the lockon to activate");
    Game::aim_lockon_strength = Game::Dvar_RegisterFloat("aim_lockon_strength", 0.6f, 0.0f, 1.0f, Game::DVAR_CHEAT, "The amount of aim assistance given by the target lock on (yaw)");
    Game::aim_lockon_region_width = Game::Dvar_RegisterFloat("aim_lockon_region_width", 90.0f, 0.0f, 640.0f, Game::DVAR_CHEAT, "The width of the auto aim region in virtual screen coordinates(0-640)");
    Game::aim_lockon_region_height = Game::Dvar_RegisterFloat("aim_lockon_region_height", 90.0f, 0.0f, 480.0f, Game::DVAR_CHEAT, "The height of the auto aim region in virtual screen coordinates(0-480)");
    Game::aim_scale_view_axis = Game::Dvar_RegisterBool("aim_scale_view_axis", true, Game::DVAR_CHEAT, "Scale the influence of each input axis so that the major axis has more influence on the control");
    Game::perk_extendedMeleeRange = Game::Dvar_RegisterFloat("perk_extendedMeleeRange", 176.0f, 0.0f, 10000.0f, Game::DVAR_CHEAT, "The range of the auto melee with the extened perk");

    Utils::Hook(0x140080FE0, Game::AimAssist_UpdateMouseInput, HOOK_JUMP).install()->quick();
    Utils::Hook(0x140080160, Game::AimAssist_ApplyAutoMelee, HOOK_JUMP).install()->quick();

    // Restore the upper right debug info that x64 compiles out, drawn after
    // the UI menus as on x86.
    //
    Game::cg_debugInfoCornerOffset = Game::Dvar_RegisterVec2("cg_debugInfoCornerOffset", 0.0f, -30.0f, -200.0f, 640.0f, Game::DVAR_CHEAT, "Offset from top-right corner, for cg_drawFPS, etc");

    Utils::Hook(0x140101D0C, Game::CL_DrawDebugOverlays, HOOK_CALL).install()->quick();
  }
}
