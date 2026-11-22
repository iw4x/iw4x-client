#include "Hook.hpp"

#include "../Types.hpp"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <exception>
#include <limits>

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

extern "C"
{
  void Controller_PlayerUseEntityStub ();
  void Controller_WriteDeltaMovementStub ();
  void Controller_ReadDeltaMovementStub1 ();
  void Controller_ReadDeltaMovementStub2 ();
  void Controller_LocalizedKeyNamesStub ();

  bool Controller_ShouldUse (const Game::gentity_s* player, unsigned held);

  void Controller_ReadDeltaMovement (Game::msg_t* msg,
                                     int key,
                                     Game::usercmd_s* from,
                                     Game::usercmd_s* to);

  Game::keyname_t* Controller_LocalizedKeyNames ();
}

namespace Controller
{
  namespace engine
  {
    bool
    should_use (const Game::gentity_s* player, unsigned held) noexcept;

    namespace
    {
      constexpr std::uintptr_t command_assignment_address {0x1400EF670};

      constexpr std::uintptr_t menu_set_binding_call_1 {0x14025D226};
      constexpr std::uintptr_t menu_set_binding_call_2 {0x14025D234};
      constexpr std::uintptr_t menu_set_binding_call_3 {0x14025D251};

      constexpr std::uintptr_t cl_key_event_call {0x1401F3C7D};
      constexpr std::uintptr_t cl_key_event_address {0x1400EE7C0};

      // Note that x64 inlines IN_MouseMove() into IN_Frame(), where we jump
      // to the epilogue once we return.
      //
      constexpr std::uintptr_t in_frame_mouse_move_call {0x1402A1148};
      constexpr std::uintptr_t in_frame_mouse_move_jump {0x1402A114D};

      constexpr std::uintptr_t ui_bypass_mouse_call {0x14026F306};

      // Note that x64 moves the cursor to the previous and the next item in
      // one function, presumably the two x86 ones folded together.
      //
      constexpr std::uintptr_t menu_cursor_focus_call {0x140263EBD};
      constexpr std::uintptr_t menu_focus_first_call {0x14025BCBE};
      constexpr std::uintptr_t menu_set_focus_call {0x14025BE2D};
      constexpr std::uintptr_t menu_set_focus_by_dvar_call {0x14025BF4A};
      constexpr std::uintptr_t item_set_focus_address {0x140260F00};

      constexpr std::uintptr_t cl_mouse_move_call {0x1400F7BC4};
      constexpr std::uintptr_t cl_remote_control_move_call {0x1400F7B75};
      constexpr std::uintptr_t cl_remote_control_move_address {0x1400F7380};
      constexpr std::uintptr_t cl_location_selection_call {0x1400F7B92};
      constexpr std::uintptr_t cl_location_selection_address {0x1400F7CE0};

      constexpr std::uintptr_t write_delta_movement_patch {0x140204572};
      constexpr std::uintptr_t write_delta_field_width_1 {0x1402046E0};
      constexpr std::uintptr_t write_delta_field_width_2 {0x140204794};

      constexpr std::uintptr_t read_delta_movement_patch_1 {0x140203A9A};
      constexpr std::uintptr_t read_delta_movement_patch_2 {0x140203B9E};

      constexpr std::uintptr_t cg_register_dvars_call {0x1400FBC1F};
      constexpr std::uintptr_t cg_register_dvars_address {0x1400D7520};

      constexpr std::uintptr_t key_write_bindings_call {0x1401F3FA8};
      constexpr std::uintptr_t key_write_bindings_address {0x1400EFCB0};

      constexpr std::uintptr_t player_use_entity_patch {0x14018BE41};

      // The instructions that reach the key name table, along with their
      // lengths. Note that x64 inlines Key_KeynumToString() into
      // Key_WriteBindings(), hence the reference there.
      //
      constexpr struct {std::uintptr_t place; std::size_t length;} keyname_table_refs[] {
        {0x1400EF88E, 7}, // Key_KeynumToStringBuf
        {0x1400EFB5E, 7}, // Key_StringToKeynum
        {0x1400EFB65, 7},
        {0x1400EFD01, 7}, // Key_WriteBindings
        {0x1400EFD6E, 8},
      };

      constexpr std::uintptr_t localized_keyname_patch {0x1400EF895};

      runtime* the_runtime {nullptr};

      constexpr size_t controller_key_count {mapping::engine_key_count};

      // Note that x64 reaches the key name table through 32-bit
      // displacements, so we have to place ours near the game.
      //
      constexpr size_t combined_key_name_count {Game::KEY_NAME_COUNT + controller_key_count + 1};

      keyname_t* combined_key_names {nullptr};
      keyname_t combined_glyphs_xbox[Game::LOCALIZED_KEY_NAME_COUNT + controller_key_count + 1] {};
      keyname_t combined_glyphs_playstation[Game::LOCALIZED_KEY_NAME_COUNT + controller_key_count + 1] {};

      bool
      driving () noexcept
      {
        return the_runtime != nullptr && the_runtime->driving ();
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

      void
      menu_set_binding (int client, int keynum, int binding) noexcept
      {
        if (mapping::is_controller_key (keynum))
        {
          if (the_runtime != nullptr)
            the_runtime->binds ().note_manual_rebind ();

          if (const char* const c = Game::Key_GetCmdForBinding (binding))
            binding = Game::Key_GetBindingForCmd (controller_command_for (c));
        }

        Game::Key_SetBinding (client, keynum, binding);
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

      void
      run_frame () noexcept
      {
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

      // On x86 we hook the tail call of IN_Frame() to IN_MouseMove() where it
      // activates the mouse. x64 inlines IN_MouseMove() at the point where
      // that path meets the ones that deactivate it, so here we run our frame
      // only when the mouse is active, which should come down to the same.
      //
      // Note also that on x86 we hook the CL_MouseEvent() call in
      // IN_MouseMove(), which RawMouse replaces. x64 has that call in the
      // inlined IN_MouseMove() that we replace altogether.
      //
      void
      in_frame_trampoline () noexcept
      {
        Components::RawMouse::IN_MouseMove ();

        if (Game::s_wmv->mouseActive)
          run_frame ();
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

        const Game::ScreenPlacement* const p (Game::ScrPlace_GetViewPlacement (0));

        // The test places the item's text rectangle on screen and the cursor
        // as a full screen point, so place the rectangle the same way and
        // turn its centre back into such a point.
        //
        const Game::rectDef_s& r (item->textRect[0]);

        float x (r.x), y (r.y - r.h), w (r.w), h (r.h);
        Game::ScrPlace_ApplyRect (p, &x, &y, &w, &h, r.horzAlign, r.vertAlign);

        constexpr int full_screen (4);

        const float x0 (Game::ScrPlace_ApplyX (p, 0.0f, full_screen));
        const float y0 (Game::ScrPlace_ApplyY (p, 0.0f, full_screen));
        const float sx (Game::ScrPlace_ApplyX (p, 1.0f, full_screen) - x0);
        const float sy (Game::ScrPlace_ApplyY (p, 1.0f, full_screen) - y0);

        if (sx == 0.0f || sy == 0.0f)
          return;

        point[0] = (x + w * 0.5f - x0) / sx;
        point[1] = (y + h * 0.5f - y0) / sy;
      }

      int
      item_set_focus (Game::UiContext* dc,
                      Game::itemDef_s* item,
                      float x,
                      float y,
                      int send_script) noexcept
      {
        float point[2] {x, y};
        menu_cursor_focus_point (item, point);

        return Utils::Hook::Call<int (Game::UiContext*, Game::itemDef_s*, float, float, int)> (
          item_set_focus_address) (dc, item, point[0], point[1], send_script);
      }

      void
      cl_mouse_move (int client, usercmd_s* cmd, float frame_time) noexcept
      {
        if (driving ())
          the_runtime->view ().apply_move (client, *cmd, frame_time);
        else
          Game::CL_MouseMove (client, cmd, frame_time);
      }

      void
      cl_remote_control_move (int client, usercmd_s* cmd) noexcept
      {
        Utils::Hook::Call<void (int, usercmd_s*)> (
          cl_remote_control_move_address) (client, cmd);

        if (driving ())
          the_runtime->view ().apply_remote_move (client, *cmd);
      }

      bool
      cl_location_selection (int client, usercmd_s* cmd) noexcept
      {
        if (!Utils::Hook::Call<bool (int, usercmd_s*)> (
              cl_location_selection_address) (client, cmd))
          return false;

        if (driving ())
          the_runtime->view ().apply_location_selection (client);

        return true;
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

      keyname_t*
      localized_key_names () noexcept
      {
        return active_glyph_family () == mapping::glyph_family::playstation
          ? combined_glyphs_playstation
          : combined_glyphs_xbox;
      }

      void
      key_write_bindings (int client, Game::fileHandle_t file) noexcept
      {
        Utils::Hook::Call<void (int, Game::fileHandle_t)> (
          key_write_bindings_address) (client, file);

        const PlayerKeyState& ks (playerKeys[client]);

        for (const mapping::engine_key k : mapping::keys ())
        {
          const int keynum (static_cast<int> (k));

          if (keynum <= Game::K_LAST_KEY)
            continue;

          const char* const binding (key_binding (ks, keynum));

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
        combined_key_names = static_cast<keyname_t*> (
          Utils::Hook::AllocateNear (keyname_table_refs[0].place,
                                     sizeof (keyname_t) * combined_key_name_count));

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

        for (const auto& r: keyname_table_refs)
          Utils::Hook::Displace (r.place,
                                 r.length,
                                 reinterpret_cast<std::uintptr_t> (Game::keyNames),
                                 reinterpret_cast<std::uintptr_t> (combined_key_names));

        Utils::Hook (localized_keyname_patch, Controller_LocalizedKeyNamesStub, HOOK_CALL).install ()->quick ();
        Utils::Hook::Nop (localized_keyname_patch + 5, 2);
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

        Game::fileHandle_t file (0);
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

      // Release the texture of an image load_loose_image() made, and free the
      // image itself.
      //
      void
      free_loose_image (Game::GfxImage* image) noexcept
      {
        Game::Image_Release (image);
        Utils::Memory::GetAllocator ()->free (image);
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
      // The small image is loaded here, on the main thread, and read by the
      // text renderer on the render thread, so it is published with release
      // and read with acquire. The copy itself is only ever touched by the
      // render thread, see text_glyph_material() for details.
      //
      struct text_glyph
      {
        const char* material;
        const char* image;

        std::atomic<Game::GfxImage*> small_image;
        bool tried;

        Game::Material copy;
        Game::MaterialTextureDef textures[
          std::numeric_limits<decltype (Game::Material::textureCount)>::max ()];
      };

      text_glyph text_glyphs[] {
        {"button_x", "button_x_small", nullptr, false, {}, {}},
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

      // Load the small image once the button material is there to take its
      // format from. The image is ours and outlives the zones.
      //
      void
      update_text_glyph (text_glyph& g) noexcept
      {
        if (g.tried)
          return;

        const Game::Material* const base (loaded_material (g.material));

        if (base == nullptr || base->textureCount == 0 ||
            base->textureTable[0].u.image == nullptr)
          return;

        g.tried = true;
        g.small_image.store (
          load_loose_image (*base->textureTable[0].u.image, g.image),
          std::memory_order_release);
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

    void
    release_glyph_images () noexcept
    {
      // Put the zone's own image back wherever ours is shown before we free
      // it, since the zone may well outlive the device. Note that the up
      // image is always there when the pressed one is.
      //
      for (stick& s: sticks)
      {
        if (s.down != nullptr)
        {
          if (Game::Material* const m = loaded_material (s.material);
              m != nullptr && m->textureCount != 0 &&
              m->textureTable[0].u.image == s.down)
            m->textureTable[0].u.image = s.up;

          free_loose_image (s.down);
          s.down = nullptr;
        }

        s.tried = false;
      }

      for (text_glyph& g: text_glyphs)
      {
        if (Game::GfxImage* const i = g.small_image.exchange (nullptr))
          free_loose_image (i);

        g.tried = false;
      }
    }

    // Note that we make the copy here, from the material the text renderer
    // is about to draw, rather than keep one made from the material as we
    // saw it last. The copy shares the base's technique set and textures, so
    // one kept across a zone reload would hand the render thread whatever is
    // left of them. And telling a reloaded material from the old one is not
    // reliable either since the zone may well end up at the same address.
    //
    // Overwriting the copy while the render thread may still have it batched
    // is fine: we only do so from the render thread itself and, until the
    // zone changes, with the same contents.
    //
    Game::Material*
    text_glyph_material (const char* name, const Game::Material* base) noexcept
    {
      for (text_glyph& g: text_glyphs)
      {
        if (std::strcmp (g.material, name) != 0)
          continue;

        Game::GfxImage* const image (g.small_image.load (std::memory_order_acquire));

        if (image == nullptr || base == nullptr || base->textureCount == 0 ||
            base->info.name == nullptr || std::strcmp (base->info.name, name) != 0)
          return nullptr;

        std::copy_n (base->textureTable, base->textureCount, g.textures);
        g.textures[0].u.image = image;

        g.copy = *base;
        g.copy.textureTable = g.textures;

        return &g.copy;
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
      Utils::Hook (write_delta_movement_patch, Controller_WriteDeltaMovementStub, HOOK_JUMP).install ()->quick ();
      Utils::Hook::Set<BYTE> (write_delta_field_width_1, 16);
      Utils::Hook::Set<BYTE> (write_delta_field_width_2, 16);

      // Note that x64 inlines MSG_ReadDeltaUsercmdKey() twice, the same as
      // x86 does, which is at least one thing that didn't change.
      //
      Utils::Hook (read_delta_movement_patch_1, Controller_ReadDeltaMovementStub1, HOOK_JUMP).install ()->quick ();
      Utils::Hook (read_delta_movement_patch_2, Controller_ReadDeltaMovementStub2, HOOK_JUMP).install ()->quick ();
      Utils::Hook (cg_register_dvars_call, cg_register_dvars, HOOK_CALL).install ()->quick ();
      Utils::Hook (key_write_bindings_call, key_write_bindings, HOOK_CALL).install ()->quick ();
      Utils::Hook (player_use_entity_patch, Controller_PlayerUseEntityStub, HOOK_JUMP).install ()->quick ();
    }

    void
    install (runtime& rt)
    {
      the_runtime = &rt;

      const context ctx (rt.make_context ());

      install_protocol ();

      build_key_name_tables ();

      // Let the player keep the aim assist they choose.
      //
      // On x86 we change the registration of these dvars. On x64 we register
      // them ourselves (see Components::Dvar), with acceleration already
      // enabled.
      //
      for (const Game::dvar_t* d: {Game::aim_accel_turnrate_enabled,
                                   Game::aim_slowdown_enabled,
                                   Game::aim_lockon_enabled})
        const_cast<Game::dvar_t*> (d)->flags = Game::DVAR_ARCHIVE;

      Utils::Hook (command_assignment_address, command_assignment, HOOK_JUMP).install ()->quick ();
      Utils::Hook (menu_set_binding_call_1, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_2, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_3, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_key_event_call, cl_key_event, HOOK_CALL).install ()->quick ();
      Utils::Hook (in_frame_mouse_move_call, in_frame_trampoline, HOOK_CALL).install ()->quick ();
      Utils::Hook::Set<std::uint16_t> (in_frame_mouse_move_jump, 0xC0EB); // jmp 0x1402A110F
      Utils::Hook (ui_bypass_mouse_call, ui_bypass_mouse_input, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_cursor_focus_call, item_set_focus, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_focus_first_call, item_set_focus, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_focus_call, item_set_focus, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_focus_by_dvar_call, item_set_focus, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_mouse_move_call, cl_mouse_move, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_remote_control_move_call, cl_remote_control_move, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_location_selection_call, cl_location_selection, HOOK_CALL).install ()->quick ();

      runtime* const r (&rt);

      Components::Scheduler::Once ([r] { r->engine_ready (); },
        Components::Scheduler::Pipeline::MAIN);

      ctx.report (severity::info, facility::engine, errc::none,
        "controller engine hooks installed");
    }
  }
}

bool
Controller_ShouldUse (const Game::gentity_s* player, unsigned held)
{
  return Controller::engine::should_use (player, held);
}

void
Controller_ReadDeltaMovement (Game::msg_t* msg,
                              int key,
                              Game::usercmd_s* from,
                              Game::usercmd_s* to)
{
  Controller::engine::read_delta_movement (msg, key, from, to);
}

Game::keyname_t*
Controller_LocalizedKeyNames ()
{
  return Controller::engine::localized_key_names ();
}
