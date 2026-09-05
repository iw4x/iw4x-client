#include "Dvar.hpp"

#include "../Types.hpp"

#include "../../Components/Modules/Dvar.hpp"

namespace Sentry
{
  namespace engine
  {
    namespace
    {
      constexpr status_dvar table[] =
      {
        {
          "sentry_release",
          "Release that crash reports from this build are attributed to",
          [] (const options& o) {return o.release;}
        },
        {
          "sentry_environment",
          "Environment that crash reports from this build are attributed to",
          [] (const options& o) {return o.environment;}
        },
        {
          "sentry_dist",
          "Build variant that crash reports from this build are attributed to",
          [] (const options& o) {return o.dist;}
        },
        {
          "sentry_database",
          "Directory holding crash reports that have not been submitted yet",
          [] (const options& o) {return o.database.string ();}
        },
      };
    }

    std::span<const status_dvar>
    status_dvars () noexcept
    {
      return table;
    }

    void
    publish_status (const options& o)
    {
      static std::vector<std::string> retained;

      const std::span<const status_dvar> ds (status_dvars ());

      retained.reserve (retained.size () + ds.size ());

      for (const status_dvar& d: ds)
      {
        const std::string& v (retained.emplace_back (d.read (o)));

        Components::Dvar::Register<const char*> (d.name, v.c_str (),
                                                 Game::DVAR_ROM, d.description);
      }
    }
  }
}
