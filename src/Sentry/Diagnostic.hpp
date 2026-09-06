#pragma once

#include "Types.hpp"

#include "Error.hpp"

namespace Sentry
{
  enum class facility : uint8_t
  {
    runtime,
    options,
    daemon,
    scope,
    trail,
    attachment,
    transport,
    engine,
    sdk,
  };

  const char*
  to_string (facility) noexcept;

  enum class severity : uint8_t
  {
    info,
    warning,
    error,
  };

  const char*
  to_string (severity) noexcept;

  struct diagnostic
  {
    severity level {severity::info};
    facility origin {facility::runtime};
    errc code {errc::none};
    std::string message;
  };

  class diagnostic_sink
  {
  public:
    virtual
    ~diagnostic_sink () = default;

    virtual void
    consume (const diagnostic&) = 0;
  };

  class logging_sink: public diagnostic_sink
  {
  public:
    void
    consume (const diagnostic&) override;
  };

  void
  report (diagnostic_sink&, severity, facility, errc, std::string message);
}
