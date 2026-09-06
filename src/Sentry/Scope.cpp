#include "Scope.hpp"

#include "Types.hpp"

#include "Breadcrumb.hpp"
#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    constexpr field table[] =
    {
      {"iw4x.version",  placement::tag, nullptr, engine::client_version,      false},
      {"iw4x.branch",   placement::tag, nullptr, engine::branch,              false},
      {"iw4x.build",    placement::tag, nullptr, engine::build_configuration, false},
      {"iw4x.role",     placement::tag, nullptr, engine::role,                true },
      {"iw4x.wine",     placement::tag, nullptr, engine::wine,                false},
      {"game.mod",      placement::tag, nullptr, engine::mod,                 true },
      {"game.map",      placement::tag, nullptr, engine::map,                 true },
      {"game.gametype", placement::tag, nullptr, engine::gametype,            true },
      {"game.state",    placement::tag, nullptr, engine::connection,          true },

      {"version",           placement::context, "iw4x", engine::client_version,      false},
      {"branch",            placement::context, "iw4x", engine::branch,              false},
      {"build",             placement::context, "iw4x", engine::build_configuration, false},
      {"role",              placement::context, "iw4x", engine::role,                false},
      {"os",                placement::context, "iw4x", engine::operating_system,    false},
      {"architecture",      placement::context, "iw4x", engine::architecture,        false},
      {"install_path",      placement::context, "iw4x", engine::install_path,        false},
      {"launch_parameters", placement::context, "iw4x", engine::launch_parameters,   false},

      {"map",      placement::context, "game", engine::map,        false},
      {"gametype", placement::context, "game", engine::gametype,   false},
      {"mod",      placement::context, "game", engine::mod,        false},
      {"state",    placement::context, "game", engine::connection, false},

      {"name",    placement::context, "server", engine::server_name,    false},
      {"address", placement::context, "server", engine::server_address, false},
      {"version", placement::context, "server", engine::server_version, false},
    };

    bool
    grouped (const field& a, const field& b) noexcept
    {
      if (a.where != placement::context || b.where != placement::context)
        return false;

      if (a.group == nullptr || b.group == nullptr)
        return a.group == b.group;

      return std::strcmp (a.group, b.group) == 0;
    }
  }

  std::span<const field>
  fields () noexcept
  {
    return table;
  }

  scope::
  scope ()
    : cached_ (fields ().size ())
  {
  }

  void
  scope::
  publish ()
  {
    const std::span<const field> fs (fields ());
    const size_t n (fs.size ());

    std::vector<std::string> vs (n);
    std::vector<bool> dirty (n, false);

    for (size_t i (0); i != n; ++i)
    {
      vs[i] = fs[i].read ();
      dirty[i] = vs[i] != cached_[i];
    }

    for (size_t i (0); i != n; ++i)
    {
      const field& f (fs[i]);

      if (f.where != placement::tag || !dirty[i])
        continue;

      if (vs[i].empty ())
        sentry_remove_tag (f.key);
      else
        sentry_set_tag (f.key, vs[i].c_str ());

      if (f.trail && primed_ && !vs[i].empty ())
        record (trail::state, SENTRY_LEVEL_INFO,
                std::format ("{} is {}", f.key, vs[i]));
    }

    for (size_t i (0); i != n; ++i)
    {
      const field& f (fs[i]);

      if (f.where != placement::context)
        continue;

      bool seen (false);

      for (size_t j (0); j != i && !seen; ++j)
        seen = grouped (fs[j], f);

      if (seen)
        continue;

      bool changed (false);
      size_t members (0);

      for (size_t j (i); j != n; ++j)
      {
        if (!grouped (fs[j], f))
          continue;

        changed = changed || dirty[j];
        members += vs[j].empty () ? 0 : 1;
      }

      if (!changed)
        continue;

      if (members == 0)
      {
        sentry_remove_context (f.group);
        continue;
      }

      sentry_value_t o (sentry_value_new_object ());

      for (size_t j (i); j != n; ++j)
      {
        if (!grouped (fs[j], f) || vs[j].empty ())
          continue;

        sentry_value_set_by_key (o, fs[j].key,
                                 sentry_value_new_string (vs[j].c_str ()));
      }

      sentry_set_context (f.group, o);
    }

    cached_ = std::move (vs);
    primed_ = true;
  }
}
