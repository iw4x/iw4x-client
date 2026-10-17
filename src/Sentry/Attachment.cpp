#include "Attachment.hpp"

#include "Types.hpp"

#include "Redact.hpp"

#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    constexpr attachment table[] =
    {
      {"zone-conversion.log", "text/plain"},
    };

    constexpr snapshot snapshot_table[] =
    {
      {CLIENT_CONFIG,             engine::config_path, redact_lines, "text/plain"},
      {BASEGAME_LEGACY ".stat",   engine::stats_path,  nullptr,      "application/octet-stream"},
    };

    // The source and modification time of the last copy we made of each
    // snapshot.
    //
    struct taken
    {
      std::filesystem::path source;
      std::filesystem::file_time_type time;
    };

    taken copies[std::size (snapshot_table)];

    constexpr generated generated_table[] =
    {
      {"console.log", "text/plain"},
    };

    std::filesystem::path
    prepared_path (const std::filesystem::path& database, const char* name)
    {
      return database / "attachments" / name;
    }

    std::filesystem::path
    snapshot_path (const std::filesystem::path& database, const snapshot& s)
    {
      return prepared_path (database, s.name);
    }
  }

  std::span<const generated>
  generated_attachments () noexcept
  {
    return generated_table;
  }

  bool
  write_generated (const std::filesystem::path& database,
                   const char* name,
                   std::string_view data)
  {
    const std::filesystem::path p (prepared_path (database, name));

    std::error_code ec;
    std::filesystem::create_directories (p.parent_path (), ec);

    std::ofstream os (p, std::ios::binary | std::ios::trunc);
    os.write (data.data (), static_cast<std::streamsize> (data.size ()));

    return os.good ();
  }

  std::span<const snapshot>
  snapshots () noexcept
  {
    return snapshot_table;
  }

  void
  refresh_snapshots (const context& ctx, const std::filesystem::path& database)
  {
    const std::span<const snapshot> ss (snapshots ());

    for (size_t i (0); i != ss.size (); ++i)
    {
      const snapshot& s (ss[i]);
      const std::filesystem::path f (s.source ());
      const std::filesystem::path t (snapshot_path (database, s));

      std::error_code ec;
      const std::filesystem::file_time_type m (
        std::filesystem::last_write_time (f, ec));

      // If the file is gone, then so should be its copy. Otherwise we would
      // attach a stale one, possibly left over from a previous run.
      //
      if (ec)
      {
        std::filesystem::remove (t, ec);
        copies[i] = taken ();
        continue;
      }

      if (copies[i].source == f && copies[i].time == m)
        continue;

      // Note that if we fail to read the file (the game may be writing it),
      // then we keep the old copy and try again on the next refresh.
      //
      std::ifstream is (f, std::ios::binary);
      std::string d ((std::istreambuf_iterator<char> (is)),
                     std::istreambuf_iterator<char> ());

      if (!is.good () && !is.eof ())
        continue;

      if (s.filter != nullptr)
        d = s.filter (std::move (d));

      std::filesystem::create_directories (t.parent_path (), ec);

      std::ofstream os (t, std::ios::binary | std::ios::trunc);
      os.write (d.data (), static_cast<std::streamsize> (d.size ()));

      if (!os.good ())
      {
        ctx.report (severity::warning, facility::attachment,
                    errc::attachment_missing,
                    std::format ("unable to write {}", t.string ()));
        continue;
      }

      copies[i] = taken {f, m};
    }
  }

  std::span<const attachment>
  attachments () noexcept
  {
    return table;
  }

  void
  attach (const context& ctx,
          const std::filesystem::path& base,
          const std::filesystem::path& database)
  {
    refresh_snapshots (ctx, database);

    for (const snapshot& s: snapshots ())
    {
      const std::filesystem::path p (snapshot_path (database, s));

      sentry_attachment_t* h (sentry_attach_filew (p.c_str ()));

      if (h == nullptr)
      {
        ctx.report (severity::warning, facility::attachment,
                    errc::attachment_missing,
                    std::format ("unable to attach {}", s.name));
        continue;
      }

      sentry_attachment_set_content_type (h, s.content_type);
    }

    for (const generated& g: generated_attachments ())
    {
      const std::filesystem::path p (prepared_path (database, g.name));

      sentry_attachment_t* h (sentry_attach_filew (p.c_str ()));

      if (h == nullptr)
      {
        ctx.report (severity::warning, facility::attachment,
                    errc::attachment_missing,
                    std::format ("unable to attach {}", g.name));
        continue;
      }

      sentry_attachment_set_content_type (h, g.content_type);
    }

    for (const attachment& a: attachments ())
    {
      const std::filesystem::path p (base / a.relative_path);

      sentry_attachment_t* h (sentry_attach_filew (p.c_str ()));

      if (h == nullptr)
      {
        ctx.report (severity::warning, facility::attachment,
                    errc::attachment_missing,
                    std::format ("unable to attach {}", a.relative_path));
        continue;
      }

      sentry_attachment_set_content_type (h, a.content_type);
    }
  }
}
