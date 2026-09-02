#include "Command.hpp"

#include "../Types.hpp"

#include "../Runtime.hpp"
#include "../Haptic/Effect.hpp"
#include "../../Components/Modules/Command.hpp"

namespace Controller
{
  namespace engine
  {
    namespace
    {
      runtime* command_runtime {nullptr};

      void
      status_command ()
      {
        if (command_runtime == nullptr)
          return;

        const context ctx (command_runtime->make_context ());

        ctx.report (severity::info, facility::engine, errc::none,
                    std::to_string (command_runtime->device_count ()) +
                    " device(s) bound; input source is " +
                    (command_runtime->keys ().in_use ()
                     ? "the controller"
                     : "keyboard and mouse"));

        if (const device_id active = command_runtime->active ())
        {
          const input_frame& f (command_runtime->latest ());

          ctx.report (severity::info, facility::engine, errc::none, active,
                      std::string ("active device: ") + to_string (f.family) +
                      " over " + to_string (f.link) + ", sequence " +
                      std::to_string (f.sequence));

          const std::string d (command_runtime->active_diagnostics ());

          if (!d.empty ())
            ctx.report (severity::info, facility::engine, errc::none, active, d);
        }
      }

      void
      haptic_command (const Components::Command::Params* params)
      {
        if (command_runtime == nullptr)
          return;

        const context ctx (command_runtime->make_context ());

        const auto number = [params] (int i, float fallback)
        {
          return params->size () > i
            ? static_cast<float> (std::atof (params->get (i)))
            : fallback;
        };

        const float intensity (number (1, 1.0f));
        const float sharpness (number (2, 0.5f));
        const float duration (number (3, 0.0f));

        const haptic::effect e (
          duration > 0.0f
          ? haptic::continuous (intensity, sharpness, seconds {duration})
          : haptic::transient (intensity, sharpness));

        command_runtime->submit (e);

        ctx.report (severity::info, facility::engine, errc::none,
                    "played a " +
                    std::string (duration > 0.0f ? "continuous" : "transient") +
                    " effect at intensity " + std::to_string (intensity) +
                    ", sharpness " + std::to_string (sharpness));

        const std::string d (command_runtime->active_diagnostics ());

        ctx.report (severity::info, facility::engine, errc::none,
                    d.empty () ? "no device is active to play it on" : d);
      }

      void
      buttons_config_command ()
      {
        if (command_runtime != nullptr)
          command_runtime->binds ().reapply_layout ();
      }

      void
      sticks_config_command ()
      {
        if (command_runtime == nullptr)
          return;

        const context ctx (command_runtime->make_context ());

        ctx.report (severity::info, facility::engine, errc::none,
                    std::string ("stick layout: ") +
                    read (command_runtime->dvars ().sticks_config,
                          "thumbstick_default"));
      }
    }

    void
    register_commands (const context& ctx, runtime& rt)
    {
      command_runtime = &rt;

      Components::Command::Add ("controller_status", &status_command);
      Components::Command::Add ("controller_haptic", &haptic_command);

      Components::Command::Add ("bindgpbuttonsconfigs", &buttons_config_command);
      Components::Command::Add ("bindgpsticksconfigs", &sticks_config_command);

      ctx.report (severity::info, facility::engine, errc::none,
                  "controller commands registered");
    }
  }
}
