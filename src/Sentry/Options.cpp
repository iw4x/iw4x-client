#include "Options.hpp"

#include "Types.hpp"

#include "Engine/State.hpp"

#include "../Components/Modules/Flags.hpp"

#include <version.hpp>

namespace Sentry
{
  namespace
  {
    const char*
    configured_dsn ()
    {
#ifdef IW4X_SENTRY_DSN
      return IW4X_SENTRY_DSN;
#else
      return "";
#endif
    }

    std::string
    environment ()
    {
      const std::string b (engine::branch ());

      if (b.empty ())
        return "unknown";

      if (b == "main")
        return "production";

      return b;
    }

    sentry_minidump_mode_t
    minidump ()
    {
      if (Components::Flags::HasFlag ("sentryfull"))
        return SENTRY_MINIDUMP_MODE_FULL;

      if (Components::Flags::HasFlag ("sentrystack"))
        return SENTRY_MINIDUMP_MODE_STACK_ONLY;

      return SENTRY_MINIDUMP_MODE_SMART;
    }
  }

  std::filesystem::path
  module_directory ()
  {
    HMODULE m (nullptr);

    if (GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                              | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR> (&module_directory),
                            &m) == FALSE)
      return engine::base_path ();

    wchar_t p[MAX_PATH] {};

    const DWORD n (GetModuleFileNameW (m, p, MAX_PATH));

    if (n == 0 || n >= MAX_PATH)
      return engine::base_path ();

    return std::filesystem::path (p).parent_path ();
  }

  options
  discover ()
  {
    options o;

    o.dsn = configured_dsn ();
    o.release = std::format ("iw4x@{}", REVISION_STR);
    o.environment = environment ();
    o.dist = engine::build_configuration ();

    o.database = engine::base_path () / "sentry";
    o.daemon = module_directory () / "sentry-crash.exe";

    o.minidump = minidump ();
    o.debug = Components::Flags::HasFlag ("sentrydebug");

    return o;
  }
}
