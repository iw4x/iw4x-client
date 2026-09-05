#include "Bind.hpp"

#include "../Types.hpp"

#include <cstring>

#include "Key.hpp"
#include "../Mapping/Binding.hpp"

namespace Controller
{
  namespace engine
  {
    using mapping::engine_key;

    namespace
    {
      constexpr char custom_layout[] {"custom"};
    }

    const char*
    controller_command_for (const char* command) noexcept
    {
      if (command == nullptr)
        return nullptr;

      if (std::strcmp (command, "+activate") == 0 ||
          std::strcmp (command, "+reload") == 0)
        return "+usereload";

      if (std::strcmp (command, "+melee_breath") == 0)
        return "+holdbreath";

      if (std::strcmp (command, "togglescores") == 0)
        return "+scores";

      return command;
    }

    bind_bridge::
    bind_bridge (const context& ctx, const dvars& d)
      : ctx_ (ctx), dvars_ (d)
    {
    }

    void
    bind_bridge::
    apply_layout (std::string_view name)
    {
      mapping::binding_table t;
      mapping::apply_button_layout (t, name);

      size_t bound (0);
      size_t unknown (0);

      t.for_each ([&bound, &unknown] (engine_key k, const std::string& command)
      {
        if (command.empty ())
        {
          ++unknown;
          return;
        }

        Key_SetBinding (local_client, static_cast<int> (k), command.c_str ());
        ++bound;
      });

      ctx_.report (unknown == 0 ? severity::info : severity::warning,
                   facility::mapping,
                   unknown == 0 ? errc::none : errc::binding_invalid,
                   "applied controller layout '" + std::string (name) + "': " +
                   std::to_string (bound) + " keys bound, " +
                   std::to_string (unknown) + " commands unknown to the engine");
    }

    void
    bind_bridge::
    apply_configured_layout ()
    {
      const char* const name (read (dvars_.buttons_config, "buttons_default"));

      applied_ = name;

      migrate_controller_commands ();

      if (std::strcmp (name, custom_layout) != 0)
        apply_layout (name);
    }

    void
    bind_bridge::
    poll_configured_layout ()
    {
      const char* const name (read (dvars_.buttons_config, "buttons_default"));

      if (applied_ == name)
        return;

      apply_configured_layout ();
    }

    void
    bind_bridge::
    migrate_controller_commands ()
    {
      const PlayerKeyState& ks (playerKeys[local_client]);

      size_t migrated (0);

      for (int key (0); key != 256; ++key)
      {
        if (!mapping::is_controller_key (key))
          continue;

        const char* const command (ks.keys[key].binding);

        if (command == nullptr)
          continue;

        const char* const wanted (controller_command_for (command));

        if (wanted == command)
          continue;

        Key_SetBinding (local_client, key, wanted);
        ++migrated;
      }

      if (migrated != 0)
        ctx_.report (severity::info, facility::mapping, errc::none,
                     "migrated " + std::to_string (migrated) +
                     " controller key(s) to the merged controller command");
    }

    void
    bind_bridge::
    reapply_layout ()
    {
      applied_.clear ();

      apply_configured_layout ();
    }

    void
    bind_bridge::
    note_manual_rebind () noexcept
    {
      if (dvars_.buttons_config == nullptr)
        return;

      Dvar_SetString (dvars_.buttons_config, custom_layout);
    }

    size_t
    bind_bridge::
    command_keys (int client,
                  bool controller_in_use,
                  const char* command,
                  int (&keys_out)[2]) noexcept
    {
      keys_out[0] = -1;
      keys_out[1] = -1;

      if (command == nullptr)
        return 0;

      const char* const lookup (controller_in_use ? controller_command_for (command)
                                                  : command);

      size_t count (0);
      const PlayerKeyState& ks (playerKeys[client]);

      for (int key (0); key != 256; ++key)
      {
        if (mapping::is_controller_key (key) != controller_in_use)
          continue;

        const char* const bound (ks.keys[key].binding);

        if (bound == nullptr || std::strcmp (bound, lookup) != 0)
          continue;

        keys_out[count++] = key;

        if (count == 2)
          break;
      }

      return count;
    }
  }
}
