#include "Report.hpp"

#include "Types.hpp"

#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    // Sentry rejects an attachment over 100 MB and an envelope over 200 MB.
    //
    constexpr std::uintmax_t attachment_limit (100 * 1024 * 1024);
    constexpr std::uintmax_t envelope_limit (200 * 1024 * 1024);

    std::string
    json (sentry_value_t v)
    {
      char* s (sentry_value_to_json (v));
      sentry_value_decref (v);

      std::string r (s != nullptr ? s : "{}");
      sentry_free (s);

      return r;
    }

    void
    set (sentry_value_t o, const char* k, const std::string& v)
    {
      if (!v.empty ())
        sentry_value_set_by_key (o, k, sentry_value_new_string (v.c_str ()));
    }

    bool
    launch (const std::filesystem::path& reporter,
            const std::filesystem::path& envelope)
    {
      // Note that CreateProcessW() may modify the command line, so it has to
      // be a writable buffer.
      //
      std::wstring c (std::format (L"\"{}\" \"{}\"",
                                   reporter.wstring (),
                                   envelope.wstring ()));

      STARTUPINFOW si {};
      si.cb = sizeof (si);

      PROCESS_INFORMATION pi {};

      if (CreateProcessW (reporter.c_str (), c.data (),
                          nullptr, nullptr, FALSE, DETACHED_PROCESS,
                          nullptr, nullptr, &si, &pi) == FALSE)
        return false;

      CloseHandle (pi.hThread);
      CloseHandle (pi.hProcess);

      return true;
    }
  }

  bool
  offer (const options& o,
         const std::string& logger,
         const std::string& message,
         std::span<const std::filesystem::path> attachments)
  {
    std::error_code ec;

    if (!o.configured () || !std::filesystem::exists (o.reporter, ec))
      return false;

    // Decide what to attach first since what we leave out goes into the
    // event.
    //
    std::vector<std::pair<std::filesystem::path, std::uintmax_t>> as;
    std::string skipped;
    std::uintmax_t total (0);

    for (const std::filesystem::path& p: attachments)
    {
      const std::uintmax_t n (std::filesystem::file_size (p, ec));

      if (ec)
        continue;

      if (n > attachment_limit || total + n > envelope_limit)
      {
        if (!skipped.empty ())
          skipped += ", ";

        skipped += p.filename ().string ();
        continue;
      }

      as.emplace_back (p, n);
      total += n;
    }

    sentry_value_t e (sentry_value_new_message_event (SENTRY_LEVEL_ERROR,
                                                      logger.c_str (),
                                                      message.c_str ()));

    set (e, "release", o.release);
    set (e, "environment", o.environment);
    set (e, "dist", o.dist);
    set (e, "platform", "native");

    sentry_value_t t (sentry_value_new_object ());
    set (t, "iw4x.branch", engine::branch ());
    set (t, "iw4x.build", engine::build_configuration ());
    set (t, "iw4x.wine", engine::wine ());
    sentry_value_set_by_key (e, "tags", t);

    sentry_value_t i (sentry_value_new_object ());
    set (i, "os", engine::operating_system ());
    set (i, "wine", engine::wine_version ());
    set (i, "architecture", engine::architecture ());
    set (i, "launch_parameters", engine::launch_parameters ());

    sentry_value_t c (sentry_value_new_object ());
    sentry_value_set_by_key (c, "iw4x", i);
    sentry_value_set_by_key (e, "contexts", c);

    if (!skipped.empty ())
    {
      sentry_value_t x (sentry_value_new_object ());
      set (x, "too_large_to_attach", skipped);
      sentry_value_set_by_key (e, "extra", x);
    }

    const std::string id (sentry_value_as_string (
                            sentry_value_get_by_key (e, "event_id")));

    const std::string ej (json (e));

    const std::filesystem::path f (o.database / "external" / (id + ".envelope"));
    std::filesystem::create_directories (f.parent_path (), ec);

    // See the envelope format at https://develop.sentry.dev/sdk/envelopes/.
    //
    {
      std::ofstream os (f, std::ios::binary | std::ios::trunc);

      sentry_value_t h (sentry_value_new_object ());
      set (h, "dsn", o.dsn);
      set (h, "event_id", id);

      os << json (h) << '\n';
      os << std::format ("{{\"type\":\"event\",\"length\":{}}}\n", ej.size ())
         << ej << '\n';

      for (const auto& [p, n]: as)
      {
        std::ifstream is (p, std::ios::binary);

        if (!is.is_open ())
          continue;

        sentry_value_t ah (sentry_value_new_object ());
        set (ah, "type", "attachment");
        sentry_value_set_by_key (ah, "length",
                                 sentry_value_new_uint64 (n));
        set (ah, "filename", p.filename ().string ());
        set (ah, "content_type",
             p.extension () == ".log" ? "text/plain" : "application/octet-stream");

        os << json (ah) << '\n';

        // Note that inserting an empty stream buffer sets failbit.
        //
        if (n != 0)
          os << is.rdbuf ();

        os << '\n';
      }

      if (!os.good ())
      {
        os.close ();
        std::filesystem::remove (f, ec);
        return false;
      }
    }

    if (!launch (o.reporter, f))
    {
      std::filesystem::remove (f, ec);
      return false;
    }

    return true;
  }
}
