-- Companion protocol headers (libcompanion/protocol.hxx, etc).
--
local targets = ...

targets:binless_library ("companion",
{
  directory = "companion",

  export =
  {
    includedirs = {"libcompanion"}
  }
})
