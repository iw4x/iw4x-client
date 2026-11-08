-- Mongoose embedded web server.
--
local targets = ...

targets:static_library ("mongoose",
{
  directory = "mongoose",
  language  = "C",

  files =
  {
    "mongoose.c",
    "mongoose.h"
  },

  export =
  {
    includedirs = {"."}
  }
})
