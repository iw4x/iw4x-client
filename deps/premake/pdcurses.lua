-- PDCurses with its Windows console port.
--
local targets = ...

targets:static_library ("pdcurses",
{
  directory = "pdcurses",
  language  = "C",

  files =
  {
    "pdcurses/*.c",
    "pdcurses/*.h",
    "wincon/*.c",
    "wincon/*.h"
  },

  export =
  {
    includedirs = {"."}
  }
})
