#include "Hook.hpp"

#include "../Types.hpp"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <exception>

#include "../Mapping/Glyph.hpp"
#include "../Mapping/Key.hpp"
#include "../Runtime.hpp"
#include "Bind.hpp"
#include "Engine.hpp"
#include "Key.hpp"
#include "NetMove.hpp"
#include "View.hpp"


#include "../../Components/Modules/Logger.hpp"
#include "../../Components/Modules/RawMouse.hpp"
#include "../../Components/Modules/Scheduler.hpp"

namespace Controller
{
  namespace engine
  {
    bool
    should_use (const Game::gentity_s* player, unsigned held) noexcept;

    namespace
    {
      constexpr auto in_frame_mouse_move_call {0x475E9E};

      constexpr auto command_assignment_address {0x5A7890};

      constexpr auto menu_set_binding_call_1 {0x47D473};
      constexpr auto menu_set_binding_call_2 {0x47D485};
      constexpr auto menu_set_binding_call_3 {0x47D49D};

      constexpr auto cl_key_event_call {0x43D179};
      constexpr auto cl_key_event_address {0x4F6480};

      constexpr auto cl_mouse_event_call {0x64C507};
      constexpr auto cl_mouse_event_address {0x4D7C50};

      constexpr auto ui_bypass_mouse_call {0x48E527};

      constexpr auto menu_prev_cursor_focus_call {0x639F91};
      constexpr auto menu_next_cursor_focus_call {0x63A03F};
      constexpr auto menu_focus_first_call {0x63BF8B};
      constexpr auto menu_set_focus_call {0x63C0E7};
      constexpr auto menu_set_focus_by_dvar_call {0x63C1D3};
      constexpr auto ui_placement_address {0x48E980};
      constexpr auto scr_place_apply_x_address {0x4B9620};
      constexpr auto scr_place_apply_y_address {0x4F1430};

      constexpr auto cl_mouse_move_call {0x5A6DAE};
      constexpr auto cl_remote_control_move_call {0x5A6D4E};
      constexpr auto cl_location_selection_call {0x5A6D72};

      constexpr auto cl_remote_control_move_address {0x5A6BA0};
      constexpr auto cl_location_selection_address {0x5A67A0};

      constexpr auto write_delta_movement_patch {0x60E38D};
      constexpr auto write_delta_movement_return {0x60E40E};
      constexpr auto write_delta_field_width_1 {0x60E501};
      constexpr auto write_delta_field_width_2 {0x60E5CD};

      constexpr auto read_delta_movement_patch_1 {0x492127};
      constexpr auto read_delta_movement_return_1 {0x4921BF};
      constexpr auto read_delta_movement_patch_2 {0x492009};
      constexpr auto read_delta_movement_return_2 {0x492085};

      constexpr auto cg_register_dvars_call {0x4059FE};
      constexpr auto cg_register_dvars_address {0x4F8DC0};

      constexpr auto key_write_bindings_call {0x60B223};
      constexpr auto key_write_bindings_address {0x4A5A20};

      constexpr auto player_use_entity_patch {0x5FE396};

      constexpr auto aim_accel_enabled_flags {0x43F8E0};
      constexpr auto aim_accel_enabled_default {0x43F8E2};
      constexpr auto aim_slowdown_enabled_flags {0x43F945};
      constexpr auto aim_lockon_enabled_flags {0x43FC76};

      constexpr auto keyname_table_slot_1 {0x4A780A};
      constexpr auto keyname_table_slot_2 {0x4A7810};
      constexpr auto keyname_table_slot_3 {0x435C9F};
      constexpr auto localized_keyname_call {0x435C97};

      runtime* the_runtime {nullptr};

      constexpr size_t controller_key_count {mapping::engine_key_count};

      keyname_t combined_key_names[Game::KEY_NAME_COUNT + controller_key_count + 1] {};
      keyname_t combined_glyphs_xbox[Game::LOCALIZED_KEY_NAME_COUNT + controller_key_count + 1] {};
      keyname_t combined_glyphs_playstation[Game::LOCALIZED_KEY_NAME_COUNT + controller_key_count + 1] {};

      bool
      driving () noexcept
      {
        return the_runtime != nullptr && the_runtime->driving ();
      }

      void
      in_frame_trampoline () noexcept
      {
        Components::RawMouse::IN_MouseMove ();

        try
        {
          the_runtime->frame ();
        }
        catch (const std::exception& e)
        {
          Components::Logger::PrintError (
            Game::CON_CHANNEL_ERROR,
            "controller: input frame failed: {}\n",
            e.what ());
        }
        catch (...)
        {
          Components::Logger::PrintError (Game::CON_CHANNEL_ERROR,
                                          "controller: input frame failed\n");
        }
      }

      __declspec (naked) void
      player_use_entity_stub ()
      {
        __asm
        {
          cmp eax, [ecx + 0x10]
          jl skip

          push eax
          pushad

          push eax
          push edi
          call should_use
          add esp, 0x8

          mov [esp + 0x20], eax

          popad
          pop eax

          test al, al
          jz skip

          push 0x5FE39B
          ret

        skip:
          push 0x5FE3AF
          ret
        }
      }

      int
      command_assignment (int client,
                          const char* command,
                          int (*keys_out)[2]) noexcept
      {
        int found[2];
        const size_t count (bind_bridge::command_keys (
          client,
          the_runtime != nullptr && the_runtime->keys ().in_use (),
          command,
          found));

        (*keys_out)[0] = found[0];
        (*keys_out)[1] = found[1];

        return static_cast<int> (count);
      }

      __declspec (naked) void
      command_assignment_stub ()
      {
        __asm
        {
          push eax
          pushad

          push [esp + 0x20 + 0x4 + 0x8]
          push [esp + 0x20 + 0x4 + 0x8]
          push eax
          call command_assignment
          add esp, 0xC

          mov [esp + 0x20], eax

          popad
          pop eax
          ret
        }
      }

      void
      menu_set_binding (int client, int keynum, const char* binding) noexcept
      {
        if (mapping::is_controller_key (keynum))
        {
          if (the_runtime != nullptr)
            the_runtime->binds ().note_manual_rebind ();

          if (binding != nullptr)
            binding = controller_command_for (binding);
        }

        Key_SetBinding (client, keynum, binding);
      }

      void
      cl_key_event (int client, int key, int down, unsigned time) noexcept
      {
        if (the_runtime != nullptr && down != 0 &&
            !mapping::is_controller_key (key))
          the_runtime->keys ().note_other_input ();

        Utils::Hook::Call<void (int, int, int, unsigned)> (
          cl_key_event_address) (client, key, down, time);
      }

      int
      cl_mouse_event (int x, int y, int dx, int dy) noexcept
      {
        note_mouse_move (dx, dy);

        return Utils::Hook::Call<int (int, int, int, int)> (
          cl_mouse_event_address) (x, y, dx, dy);
      }

      bool
      ui_bypass_mouse_input () noexcept
      {
        static Game::dvar_t* bypass {nullptr};

        if (bypass == nullptr)
          bypass = Dvar_FindVar ("cl_bypassMouseInput");

        return read (bypass, false) ||
          (the_runtime != nullptr && the_runtime->keys ().in_use ());
      }

      // Moving through a menu with the arrow keys, and the focusFirst,
      // setFocus and setFocusByDvar scripts, only give a text item focus when
      // the point they test is over it: the cursor, or the item's unplaced
      // rectangle. That works with a mouse at hand but not with the cursor
      // hidden for a controller: the open choice list of a setting neither
      // highlights the current value nor moves off its invisible close
      // button. Test the item at its own centre instead while a controller
      // drives the menu.
      //
      void
      menu_cursor_focus_point (const Game::itemDef_s* item, float* point) noexcept
      {
        if (the_runtime == nullptr || !the_runtime->keys ().in_use () ||
            item == nullptr || item->type != Game::ITEM_TYPE_TEXT)
          return;

        using placement_t = const Game::ScreenPlacement* (*) (int);
        using apply_t = float (*) (const Game::ScreenPlacement*, float, int);

        const auto ui_placement (reinterpret_cast<placement_t> (ui_placement_address));
        const auto apply_x (reinterpret_cast<apply_t> (scr_place_apply_x_address));
        const auto apply_y (reinterpret_cast<apply_t> (scr_place_apply_y_address));

        const Game::ScreenPlacement* const p (ui_placement (0));

        // The test places the item's text rectangle on screen and the cursor
        // as a full screen point, so place the rectangle the same way and
        // turn its centre back into such a point.
        //
        const Game::rectDef_s& r (item->textRect[0]);

        float x (r.x), y (r.y - r.h), w (r.w), h (r.h);
        Game::ScrPlace_ApplyRect (p, &x, &y, &w, &h, r.horzAlign, r.vertAlign);

        constexpr int full_screen (4);

        const float x0 (apply_x (p, 0.0f, full_screen));
        const float y0 (apply_y (p, 0.0f, full_screen));
        const float sx (apply_x (p, 1.0f, full_screen) - x0);
        const float sy (apply_y (p, 1.0f, full_screen) - y0);

        if (sx == 0.0f || sy == 0.0f)
          return;

        point[0] = (x + w * 0.5f - x0) / sx;
        point[1] = (y + h * 0.5f - y0) / sy;
      }

      __declspec (naked) void
      item_set_focus_stub ()
      {
        __asm
        {
          pushad

          lea eax, [esp + 0x20 + 0x4]
          push eax
          push ebx
          call menu_cursor_focus_point
          add esp, 0x8

          popad

          push 0x6392B0
          retn
        }
      }

      void
      cl_mouse_move (int client, usercmd_s* cmd, float frame_time) noexcept
      {
        if (driving ())
          the_runtime->view ().apply_move (client, *cmd, frame_time);
        else
          Game::CL_MouseMove (client, cmd, frame_time);
      }

      __declspec (naked) void
      cl_mouse_move_stub ()
      {
        __asm
        {
          pushad

          push [esp + 0x20 + 0x4]
          push ebx
          push eax
          call cl_mouse_move
          add esp, 0xC

          popad
          ret
        }
      }

      void
      cl_remote_control_move (int client, usercmd_s* cmd) noexcept
      {
        if (driving ())
          the_runtime->view ().apply_remote_move (client, *cmd);
      }

      __declspec (naked) void
      cl_remote_control_move_stub ()
      {
        __asm
        {
          push edi
          push eax

          call cl_remote_control_move_address
          call cl_remote_control_move

          add esp, 0x8
          ret
        }
      }

      bool
      cl_location_selection (int client, usercmd_s*) noexcept
      {
        if (driving ())
          the_runtime->view ().apply_location_selection (client);

        return true;
      }

      __declspec (naked) void
      cl_location_selection_stub ()
      {
        __asm
        {
          push esi
          push eax

          call cl_location_selection_address

          test al, al
          jz done

          call cl_location_selection

        done:
          add esp, 0x8
          ret
        }
      }

      __declspec (naked) void
      write_delta_movement_stub ()
      {
        __asm
        {
          add esp, 0xC

          mov dl, byte ptr [edi + 0x1A]
          mov dh, byte ptr [edi + 0x1B]
          mov [esp + 0x30], dx

          mov dl, byte ptr [ebp + 0x1A]
          mov dh, byte ptr [ebp + 0x1B]
          mov [esp + 0x2C], dx

          push 0x60E40E
          retn
        }
      }

      void
      read_delta_movement (Game::msg_t* msg,
                           int key,
                           usercmd_s* from,
                           usercmd_s* to) noexcept
      {
        move_delta d {from->forwardmove, from->rightmove};

        if (Game::MSG_ReadBit (msg))
          d = unpack_move (static_cast<uint16_t> (Game::MSG_ReadBits (msg, 16)),
                           key);

        to->forwardmove = d.forward;
        to->rightmove = d.right;
      }

      __declspec (naked) void
      read_delta_movement_stub_1 ()
      {
        __asm
        {
          push ebx
          push ebp
          push edi
          push esi
          call read_delta_movement
          add esp, 0x10

          push 0x4921BF
          ret
        }
      }

      __declspec (naked) void
      read_delta_movement_stub_2 ()
      {
        __asm
        {
          push ebx
          push ebp
          push edi
          push esi
          call read_delta_movement
          add esp, 0x10

          push 3
          push esi
          push 0x492085
          ret
        }
      }

      keyname_t*
      localized_key_names () noexcept
      {
        return active_glyph_family () == mapping::glyph_family::playstation
          ? combined_glyphs_playstation
          : combined_glyphs_xbox;
      }

      __declspec (naked) void
      localized_key_names_stub ()
      {
        __asm
        {
          push eax
          pushad

          call localized_key_names
          mov [esp + 0x20], eax

          popad
          pop eax

          test edi, edi
          ret
        }
      }

      void
      key_write_bindings (int client, int file) noexcept
      {
        Utils::Hook::Call<void (int, int)> (key_write_bindings_address) (client,
                                                                         file);

        const PlayerKeyState& ks (playerKeys[client]);

        for (const mapping::engine_key k : mapping::keys ())
        {
          const int keynum (static_cast<int> (k));

          if (keynum <= Game::K_LAST_KEY)
            continue;

          const char* const binding (ks.keys[keynum].binding);

          if (binding == nullptr || binding[0] == '\0')
            continue;

          Game::FS_Printf (file,
                           "bind %s \"%s\"\n",
                           mapping::key_name (k),
                           binding);
        }
      }

      void
      cg_register_dvars ()
      {
        Utils::Hook::Call<void ()> (cg_register_dvars_address) ();

        register_dvars ();
      }

      void
      build_key_name_tables ()
      {
        std::memcpy (combined_key_names,
                     Game::keyNames,
                     sizeof (keyname_t) * Game::KEY_NAME_COUNT);
        std::memcpy (combined_glyphs_xbox,
                     Game::localizedKeyNames,
                     sizeof (keyname_t) * Game::LOCALIZED_KEY_NAME_COUNT);
        std::memcpy (combined_glyphs_playstation,
                     Game::localizedKeyNames,
                     sizeof (keyname_t) * Game::LOCALIZED_KEY_NAME_COUNT);

        size_t names (Game::KEY_NAME_COUNT);
        size_t glyphs (Game::LOCALIZED_KEY_NAME_COUNT);

        for (const mapping::engine_key k : mapping::keys ())
        {
          const int keynum (static_cast<int> (k));

          const char* const name (mapping::key_name (k));

          combined_key_names[names++] = {name, keynum};

          const char* const xbox (
            mapping::glyph_for (k, mapping::glyph_family::xbox));

          const char* const playstation (
            mapping::glyph_for (k, mapping::glyph_family::playstation));

          combined_glyphs_xbox[glyphs] = {xbox != nullptr ? xbox : name, keynum};
          combined_glyphs_playstation[glyphs] =
            {playstation != nullptr ? playstation : name, keynum};
          ++glyphs;
        }

        combined_key_names[names] = {nullptr, 0};
        combined_glyphs_xbox[glyphs] = {nullptr, 0};
        combined_glyphs_playstation[glyphs] = {nullptr, 0};

        Utils::Hook::Set<keyname_t*> (keyname_table_slot_1, combined_key_names);
        Utils::Hook::Set<keyname_t*> (keyname_table_slot_2, combined_key_names);
        Utils::Hook::Set<keyname_t*> (keyname_table_slot_3, combined_key_names);
        Utils::Hook (localized_keyname_call, localized_key_names_stub, HOOK_CALL).install ()->quick ();
      }
    }

    void
    note_mouse_move (int dx, int dy) noexcept
    {
      if (the_runtime != nullptr && (dx != 0 || dy != 0))
        the_runtime->keys ().note_other_input ();
    }

    namespace
    {
      // A new image like the one given, filled from a loose .iwi the way the
      // engine fills an image whose pixels are not in a zone, or nullptr if
      // there is no such file.
      //
      Game::GfxImage*
      load_loose_image (const Game::GfxImage& like, const char* name) noexcept
      {
        // The engine reports an image it cannot open as an error, so look
        // for the file first, the way the loader below opens it: these
        // images are optional.
        //
        const std::string path ("images/" + std::string (name) + ".iwi");

        int file (0);
        Game::FS_FOpenFileReadDatabase (path.c_str (), &file);

        if (file == 0)
          return nullptr;

        Game::FS_FCloseFile (file);

        auto* const image (Utils::Memory::GetAllocator ()->allocate<Game::GfxImage> ());
        *image = like;
        image->texture = {};
        image->cardMemory = {};
        image->delayLoadPixels = false;
        image->name = name;

        if (!Game::Image_LoadFromFileWithReader (
              image, reinterpret_cast<Game::Reader_t> (Game::FS_FOpenFileReadDatabase)))
        {
          Utils::Memory::GetAllocator ()->free (image);
          return nullptr;
        }

        return image;
      }

      // The material named as its zone has it now, or nullptr while no
      // loaded zone does. Zones come and go, with a map or with fs_game, so
      // this is looked up again every frame rather than kept.
      //
      Game::Material*
      loaded_material (const char* name) noexcept
      {
        const Game::XAssetEntry* const e (
          Game::DB_FindXAssetEntry (Game::ASSET_TYPE_MATERIAL, name));

        return e != nullptr ? e->asset.header.material : nullptr;
      }

      struct stick
      {
        const char* material;
        const char* pressed;

        // The zone's own image of the material as last seen, and the pressed
        // image loaded for it. The pressed image is ours and outlives zones.
        //
        Game::GfxImage* up;
        Game::GfxImage* down;
        bool tried;
      };

      stick sticks[] {
        {"button_lstick", "button_lstick2", nullptr, nullptr, false},
        {"button_rstick", "button_rstick2", nullptr, nullptr, false},
        {"button_ps3_l3", "button_ps3_l3_alt", nullptr, nullptr, false},
        {"button_ps3_r3", "button_ps3_r3_alt", nullptr, nullptr, false},
      };

      // The console draws the buttons in hint text from its HUD font, a
      // smaller glyph than the button images the HUD itself shows. Where a
      // small image is there, text gets a copy of the button material that
      // uses it.
      //
      // The copy is written here, on the main thread, and read by the text
      // renderer, so it is published with release and read with acquire.
      // A copy made from a material that has since gone is not freed, as the
      // renderer may still be drawing with it; this only happens when a
      // zone with the material is reloaded.
      //
      struct text_glyph
      {
        const char* material;
        const char* image;

        Game::GfxImage* small_image;
        bool tried;

        // What the copy was made from, to tell when the zone has reloaded it.
        //
        const Game::Material* base;
        const Game::MaterialTextureDef* base_textures;

        std::atomic<Game::Material*> m;
      };

      text_glyph text_glyphs[] {
        {"button_x", "button_x_small", nullptr, false, nullptr, nullptr, nullptr},
      };

      void
      update_stick (stick& s, bool pressed) noexcept
      {
        Game::Material* const m (loaded_material (s.material));

        if (m == nullptr || m->textureCount == 0 || m->textureTable[0].u.image == nullptr)
          return;

        Game::GfxImage* const shown (m->textureTable[0].u.image);

        // Anything other than our two images is the zone's own, loaded
        // afresh: take it as the up image.
        //
        if (shown != s.down && shown != s.up)
          s.up = shown;

        if (s.down == nullptr)
        {
          if (s.tried)
            return;

          s.tried = true;
          s.down = load_loose_image (*s.up, s.pressed);

          if (s.down == nullptr)
          {
            Components::Logger::Print ("controller: no {} image to animate {}\n",
                                       s.pressed, s.material);
            return;
          }
        }

        m->textureTable[0].u.image = pressed ? s.down : s.up;
      }

      void
      update_text_glyph (text_glyph& g) noexcept
      {
        const Game::Material* const base (loaded_material (g.material));

        if (base == nullptr || base->textureCount == 0 ||
            base->textureTable[0].u.image == nullptr)
        {
          g.m.store (nullptr, std::memory_order_release);
          g.base = nullptr;
          return;
        }

        if (base == g.base && base->textureTable == g.base_textures &&
            g.m.load (std::memory_order_relaxed) != nullptr)
          return;

        if (g.small_image == nullptr)
        {
          if (g.tried)
            return;

          g.tried = true;
          g.small_image = load_loose_image (*base->textureTable[0].u.image, g.image);

          if (g.small_image == nullptr)
            return;
        }

        auto* const textures (Utils::Memory::GetAllocator ()->allocateArray<Game::MaterialTextureDef> (
          base->textureCount));

        std::copy_n (base->textureTable, base->textureCount, textures);
        textures[0].u.image = g.small_image;

        auto* const m (Utils::Memory::GetAllocator ()->allocate<Game::Material> ());
        *m = *base;
        m->textureTable = textures;

        g.base = base;
        g.base_textures = base->textureTable;
        g.m.store (m, std::memory_order_release);
      }
    }

    void
    update_glyph_images () noexcept
    {
      // The console holds each stick frame for about this long.
      //
      constexpr int frame_time {400};

      const bool pressed ((Sys_Milliseconds () / frame_time) % 2 != 0);

      for (stick& s: sticks)
        update_stick (s, pressed);

      for (text_glyph& g: text_glyphs)
        update_text_glyph (g);
    }

    Game::Material*
    text_glyph_material (const char* name) noexcept
    {
      for (const text_glyph& g: text_glyphs)
      {
        if (std::strcmp (g.material, name) == 0)
          return g.m.load (std::memory_order_acquire);
      }

      return nullptr;
    }

    mapping::glyph_family
    active_glyph_family () noexcept
    {
      static Game::dvar_t* style {nullptr};

      if (style == nullptr)
        style = Dvar_FindVar ("gpad_style");

      std::optional<mapping::glyph_family> chosen;

      switch (read (style, 0))
      {
        case 1: chosen = mapping::glyph_family::playstation; break;
        case 2: chosen = mapping::glyph_family::xbox; break;
        default: break;
      }

      return chosen ? *chosen : controller_glyph_family ();
    }

    mapping::glyph_family
    controller_glyph_family () noexcept
    {
      const Controller::family device (
        the_runtime != nullptr && the_runtime->active () != no_device
        ? the_runtime->latest ().family
        : Controller::family::unknown);

      return mapping::glyph_family_for (device, std::nullopt);
    }

    bool
    should_use (const Game::gentity_s* player, unsigned held) noexcept
    {
      if ((player->client->buttons & Game::CMD_BUTTON_USE_RELOAD) == 0)
        return true;

      const int hold (read (registered_dvars ().use_hold_time, 250));

      return hold <= 0 || held >= static_cast<unsigned> (hold);
    }

    void
    install_protocol ()
    {
      Utils::Hook (write_delta_movement_patch, write_delta_movement_stub, HOOK_JUMP).install ()->quick ();
      Utils::Hook::Set<BYTE> (write_delta_field_width_1, 16);
      Utils::Hook::Set<BYTE> (write_delta_field_width_2, 16);
      Utils::Hook (read_delta_movement_patch_1, read_delta_movement_stub_1, HOOK_JUMP).install ()->quick ();
      Utils::Hook (read_delta_movement_patch_2, read_delta_movement_stub_2, HOOK_JUMP).install ()->quick ();
      Utils::Hook (cg_register_dvars_call, cg_register_dvars, HOOK_CALL).install ()->quick ();
      Utils::Hook (key_write_bindings_call, key_write_bindings, HOOK_CALL).install ()->quick ();
      Utils::Hook (player_use_entity_patch, player_use_entity_stub, HOOK_JUMP).install ()->quick ();
    }

    void
    install (runtime& rt)
    {
      the_runtime = &rt;

      const context ctx (rt.make_context ());

      install_protocol ();

      build_key_name_tables ();

      Utils::Hook::Set<BYTE> (aim_accel_enabled_flags, Game::DVAR_ARCHIVE);
      Utils::Hook::Set<BYTE> (aim_accel_enabled_default, 1);
      Utils::Hook::Set<BYTE> (aim_slowdown_enabled_flags, Game::DVAR_ARCHIVE);
      Utils::Hook::Set<BYTE> (aim_lockon_enabled_flags, Game::DVAR_ARCHIVE);

      Utils::Hook (command_assignment_address, command_assignment_stub, HOOK_JUMP).install ()->quick ();
      Utils::Hook (menu_set_binding_call_1, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_2, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_3, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (in_frame_mouse_move_call, in_frame_trampoline, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_key_event_call, cl_key_event, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_mouse_event_call, cl_mouse_event, HOOK_CALL).install ()->quick ();
      Utils::Hook (ui_bypass_mouse_call, ui_bypass_mouse_input, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_prev_cursor_focus_call, item_set_focus_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_next_cursor_focus_call, item_set_focus_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_focus_first_call, item_set_focus_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_focus_call, item_set_focus_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_focus_by_dvar_call, item_set_focus_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_mouse_move_call, cl_mouse_move_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_remote_control_move_call, cl_remote_control_move_stub, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_location_selection_call, cl_location_selection_stub, HOOK_CALL).install ()->quick ();

      runtime* const r (&rt);

      Components::Scheduler::Once ([r] { r->engine_ready (); },
        Components::Scheduler::Pipeline::MAIN);

      ctx.report (severity::info, facility::engine, errc::none,
        "controller engine hooks installed");
    }
  }
}
