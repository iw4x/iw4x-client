-- Discord Rich Presence.
--
local targets = ...

targets:static_library ("discord-rpc",
{
  directory    = "discord-rpc",
  language     = "C++",
  cppdialect   = "C++17",
  dependencies = {"rapidjson"},

  files =
  {
    "src/*.h",
    "src/*.cpp"
  },

  -- Leave out the DLL entry point and the sources of other platforms.
  --
  removefiles =
  {
    "src/dllmain.cpp",
    "src/*_linux.cpp",
    "src/*_unix.cpp",
    "src/*_osx.cpp"
  },

  defines = {"RAPIDJSON_HAS_STDSTRING"},

  export =
  {
    includedirs = {"include"}
  }
})
