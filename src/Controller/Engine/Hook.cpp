#include "Hook.hpp"

#include "../Types.hpp"

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

      constexpr auto key_write_bindings_call {0x60B223};
      constexpr auto key_write_bindings_address {0x4A5A20};

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
        static Game::dvar_t* style {nullptr};

        if (style == nullptr)
          style = Dvar_FindVar ("gpad_style");

        return read (style, false) ? combined_glyphs_playstation
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

    void
		install_protocol ()
    {
      Utils::Hook (write_delta_movement_patch, write_delta_movement_stub, HOOK_JUMP).install ()->quick ();
      Utils::Hook::Set<BYTE> (write_delta_field_width_1, 16);
      Utils::Hook::Set<BYTE> (write_delta_field_width_2, 16);
      Utils::Hook (read_delta_movement_patch_1, read_delta_movement_stub_1, HOOK_JUMP).install ()->quick ();
      Utils::Hook (read_delta_movement_patch_2, read_delta_movement_stub_2, HOOK_JUMP).install ()->quick ();
    }

    void
		install (runtime& rt)
    {
      the_runtime = &rt;

      const context ctx (rt.make_context ());

      install_protocol ();

      build_key_name_tables ();

      Utils::Hook::Set<BYTE> (aim_accel_enabled_flags, Game::DVAR_ARCHIVE);
      Utils::Hook::Set<BYTE> (aim_accel_enabled_default, 0);
      Utils::Hook::Set<BYTE> (aim_slowdown_enabled_flags, Game::DVAR_ARCHIVE);
      Utils::Hook::Set<BYTE> (aim_lockon_enabled_flags, Game::DVAR_ARCHIVE);

      Utils::Hook (command_assignment_address, command_assignment_stub, HOOK_JUMP).install ()->quick ();
      Utils::Hook (menu_set_binding_call_1, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_2, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (menu_set_binding_call_3, menu_set_binding, HOOK_CALL).install ()->quick ();
      Utils::Hook (key_write_bindings_call, key_write_bindings, HOOK_CALL).install ()->quick ();
      Utils::Hook (in_frame_mouse_move_call, in_frame_trampoline, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_key_event_call, cl_key_event, HOOK_CALL).install ()->quick ();
      Utils::Hook (cl_mouse_event_call, cl_mouse_event, HOOK_CALL).install ()->quick ();
      Utils::Hook (ui_bypass_mouse_call, ui_bypass_mouse_input, HOOK_CALL).install ()->quick ();
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
