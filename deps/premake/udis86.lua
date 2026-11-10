-- udis86 disassembler.
--
-- Note that the instruction tables (itab.c and itab.h), which upstream
-- generates with Python, are pregenerated in extra/udis86/.
--
local targets = ...

targets:static_library ("udis86",
{
  directory = "udis86",
  language  = "C",

  files =
  {
    "libudis86/*.c",
    "../extra/udis86/libudis86/*.c"
  },

  defines = {"HAVE_STRING_H=1"},

  export =
  {
    includedirs =
    {
      ".",
      "libudis86",
      "../extra/udis86",
      "../extra/udis86/libudis86"
    }
  }
})
