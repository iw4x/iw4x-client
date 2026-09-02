#pragma once

#include "Types.hpp"

#include "Error.hpp"
#include "Device/Id.hpp"

namespace Controller
{
  enum class facility : uint8_t
  {
    runtime,
    discovery,
    transport,
    driver,
    decode,
    sample,
    calibration,
    mapping,
    aim,
    steam,
    engine,
    debug,
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
    device_id device {};
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
  report (diagnostic_sink&,
          severity,
          facility,
          errc,
          device_id,
          std::string message);

  inline void
  report (diagnostic_sink& sink,
          severity level,
          facility origin,
          errc code,
          std::string message)
  {
    report (sink, level, origin, code, no_device, std::move (message));
  }
}
