#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Dvar.hpp"

namespace Controller
{
  namespace engine
  {
    class bind_bridge
    {
    public:
      bind_bridge (const context&, const dvars&);

      void
      apply_layout (std::string_view name);

      void
      apply_configured_layout ();

      void
      poll_configured_layout ();

      void
      reapply_layout ();

      void
      note_manual_rebind () noexcept;

      static size_t
      command_keys (int client,
                    bool controller_in_use,
                    const char* command,
                    int (&keys_out)[2]) noexcept;

    private:
      void
      migrate_controller_commands ();

      const context& ctx_;
      const dvars& dvars_;

      std::string applied_;
    };

    const char*
    controller_command_for (const char* command) noexcept;
  }
}
