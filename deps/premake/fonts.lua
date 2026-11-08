-- Fonts embedded into the client as C++ headers.
--
local targets = ...

targets:binless_library ("fonts",
{
  directory = "extra/font",

  export =
  {
    includedirs = {"."}
  }
})
