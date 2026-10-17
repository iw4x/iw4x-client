-- SQLite from its build2 package, which is what the launcher uses through
-- odb. We build the package's copy of the amalgamation, which carries its
-- warning fixes, against the upstream headers (the package's own headers are
-- symlinks, which don't survive every checkout).
--
-- Note that we can't put upstream/ itself on the include path: it holds a
-- file named version, which would shadow the standard <version> header. So
-- the header is included as <upstream/sqlite3.h>.
--
sqlite = {
  source = path.join(dependencies.basePath, "sqlite"),
}

function sqlite.import()
  links {"sqlite"}
  sqlite.includes()
end

function sqlite.includes()
  includedirs {
    sqlite.source
  }
end

function sqlite.project()
  project "sqlite"
    language "C"

    sqlite.includes()

    files {
      path.join(sqlite.source, "upstream/sqlite3.h"),
      path.join(sqlite.source, "libsqlite3/libsqlite3/sqlite3.c"),
    }

    defines {
      "SQLITE_DQS=0",
      "SQLITE_OMIT_LOAD_EXTENSION",
      "SQLITE_OMIT_DEPRECATED",
      "SQLITE_DEFAULT_MEMSTATUS=0",
    }

    warnings "Off"
    kind "StaticLib"
end

table.insert(dependencies, sqlite)
