-- Tracy profiler client.
--
local targets = ...

local profile = _OPTIONS["profile"] ~= nil

targets:static_library ("tracy",
{
  directory = "tracy",
  language  = "C++",
  files     = {"public/TracyClient.cpp"},

  export =
  {
    includedirs = {"public"},

    -- Note that these are exported since Tracy.hpp, which our sources
    -- include, must see the same configuration as the client.
    --
    defines = profile and
    {
      "TRACY_ENABLE",
      "TRACY_ON_DEMAND",
      "TRACY_DELAYED_INIT",
      "TRACY_MANUAL_LIFETIME",
      "TRACY_NO_CALLSTACK",
      "TRACY_NO_SAMPLING",
      "TRACY_NO_SYSTEM_TRACING"
    } or {},

    links = profile and {"secur32", "ws2_32"} or {}
  }
})
