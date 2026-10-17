#pragma once

#include "Types.hpp"

#include "Context.hpp"

namespace Sentry
{
  struct attachment
  {
    const char* relative_path;
    const char* content_type;
  };

  std::span<const attachment>
  attachments () noexcept;

  // A file that we attach a copy of instead. The copy lives in the database
  // directory and is refreshed as the file changes, so that it is ready when
  // the game crashes. This lets us follow a file that moves (the stats file
  // is per mod, for example) and remove what the player may not want to send.
  //
  struct snapshot
  {
    const char* name;
    std::filesystem::path (*source) ();
    std::string (*filter) (std::string);
    const char* content_type;
  };

  std::span<const snapshot>
  snapshots () noexcept;

  // A file that we generate ourselves, such as the recent console output. It
  // lives next to the snapshot copies.
  //
  struct generated
  {
    const char* name;
    const char* content_type;
  };

  std::span<const generated>
  generated_attachments () noexcept;

  // Write a generated attachment, replacing the previous one. Return false if
  // we couldn't.
  //
  bool
  write_generated (const std::filesystem::path& database,
                   const char* name,
                   std::string_view data);

  void
  attach (const context&,
          const std::filesystem::path& base,
          const std::filesystem::path& database);

  // Bring the snapshot copies up to date. Must be called from the main thread
  // since the sources depend on the game state.
  //
  void
  refresh_snapshots (const context&, const std::filesystem::path& database);
}
