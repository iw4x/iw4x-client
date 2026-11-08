-- JSON for Modern C++ (nlohmann/json), included as json.hpp.
--
local targets = ...

targets:binless_library ("json",
{
  directory = "json",

  export =
  {
    includedirs = {"single_include/nlohmann"}
  }
})
