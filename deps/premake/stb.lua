-- stb_truetype (v1.24), the TrueType rasterizer from the stb single-file
-- libraries.
--
local targets = ...

targets:binless_library ("stb",
{
  directory = "extra/stb",

  export =
  {
    includedirs = {"."}
  }
})
