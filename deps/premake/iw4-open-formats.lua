-- IW4 asset formats.
--
local targets = ...

targets:static_library ("iw4-open-formats",
{
  directory    = "iw4-open-formats",
  language     = "C++",
  dependencies = {"libtomcrypt", "libtommath", "rapidjson", "zlib"},

  -- Note that the precompiled header is named the way the sources include
  -- it.
  --
  pchheader = "std_include.hpp",
  pchsource = "src/iw4-of/std_include.cpp",

  files =
  {
    "src/iw4-of/**.hpp",
    "src/iw4-of/**.cpp"
  },

  includedirs = {"src/iw4-of"},

  export =
  {
    includedirs = {"include"}
  }
})
