#include "Breadcrumb.hpp"

#include "Types.hpp"

namespace Sentry
{
  namespace
  {
    constexpr trail_kind kinds[] =
    {
      {"default", "lifecycle"},
      {"default", "state"},
      {"error",   "error"},
      {"debug",   "engine"},
    };

    static_assert (std::size (kinds) == static_cast<size_t> (trail::engine) + 1,
                   "every trail must name a breadcrumb type and category");

    const char*
    name_of (const sentry_level_t l) noexcept
    {
      switch (l)
      {
      case SENTRY_LEVEL_FATAL:   return "fatal";
      case SENTRY_LEVEL_ERROR:   return "error";
      case SENTRY_LEVEL_WARNING: return "warning";
      case SENTRY_LEVEL_DEBUG:   return "debug";
      case SENTRY_LEVEL_TRACE:   return "debug";
      default:                   return "info";
      }
    }
  }

  const trail_kind&
  kind_of (const trail t) noexcept
  {
    return kinds[static_cast<size_t> (t)];
  }

  void
  record (const trail t, const sentry_level_t l, const std::string& m)
  {
    const trail_kind& k (kind_of (t));

    sentry_value_t c (sentry_value_new_breadcrumb (k.type, m.c_str ()));

    sentry_value_set_by_key (c, "category",
                             sentry_value_new_string (k.category));
    sentry_value_set_by_key (c, "level", sentry_value_new_string (name_of (l)));

    sentry_add_breadcrumb (c);
  }
}
