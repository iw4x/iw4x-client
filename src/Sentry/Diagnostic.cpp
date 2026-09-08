#include "Diagnostic.hpp"

#include "Types.hpp"

#include "../Components/Modules/Logger.hpp"

namespace Sentry
{
  const char*
  to_string (facility f) noexcept
  {
    switch (f)
    {
      case facility::runtime:    return "runtime";
      case facility::options:    return "options";
      case facility::daemon:     return "daemon";
      case facility::scope:      return "scope";
      case facility::trail:      return "trail";
      case facility::attachment: return "attachment";
      case facility::transport:  return "transport";
      case facility::engine:     return "engine";
      case facility::sdk:        return "sdk";
    }

    return "runtime";
  }

  const char*
  to_string (severity s) noexcept
  {
    switch (s)
    {
      case severity::info:    return "info";
      case severity::warning: return "warning";
      case severity::error:   return "error";
    }

    return "info";
  }

  void
  logging_sink::
  consume (const diagnostic& d)
  {
    std::ostringstream line;
    line << "sentry: " << to_string (d.origin) << ": " << d.message;

    if (d.code != errc::none)
      line << " [" << d.code << ']';

    switch (d.level)
    {
      case severity::info:
        Components::Logger::Print (Game::CON_CHANNEL_SYSTEM, "{}\n", line.str ());
        break;
      case severity::warning:
        Components::Logger::Warning (Game::CON_CHANNEL_SYSTEM, "{}\n", line.str ());
        break;
      case severity::error:
        Components::Logger::PrintError (Game::CON_CHANNEL_ERROR, "{}\n", line.str ());
        break;
    }
  }

  void
  report (diagnostic_sink& sink,
          severity level,
          facility origin,
          errc code,
          std::string message)
  {
    sink.consume (diagnostic {level, origin, code, std::move (message)});
  }
}
