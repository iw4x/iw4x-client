-- SQLite from its build2 package, which is what the launcher uses through
-- ODB.
--
-- We build the amalgamation of the package, which carries its warning
-- fixes, against the upstream header. The package's own headers are
-- symlinks, which don't survive every checkout.
--
-- Note that we can't add upstream/ itself to the include path: it holds a
-- file named version, which would shadow the standard <version> header. So
-- the client includes <upstream/sqlite3.h>.
--
local targets = ...

targets:static_library ("sqlite",
{
  directory = "sqlite",
  language  = "C",

  files =
  {
    "upstream/sqlite3.h",
    "libsqlite3/libsqlite3/sqlite3.c"
  },

  defines =
  {
    "SQLITE_DQS=0",
    "SQLITE_OMIT_LOAD_EXTENSION",
    "SQLITE_OMIT_DEPRECATED",
    "SQLITE_DEFAULT_MEMSTATUS=0"
  },

  export =
  {
    includedirs = {"."}
  }
})
