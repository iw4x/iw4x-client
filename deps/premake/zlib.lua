-- zlib.
--
local targets = ...

targets:static_library ("zlib",
{
  directory = "zlib",
  language  = "C",

  files =
  {
    "*.h",
    "*.c"
  },

  defines = {"_CRT_SECURE_NO_DEPRECATE"},

  export =
  {
    includedirs = {"."},
    defines     = {"ZLIB_CONST"}
  }
})
