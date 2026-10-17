#include "Report.hpp"

#include "Types.hpp"

#include "Guard.hpp"

#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    // Our Sentry server (iw4x/self-hosted) rejects a request over 1 GiB, both
    // in nginx and in Relay, which also bounds every attachment in it. Keep
    // some room for the event and the item headers.
    //
    // Note that this has to stay in sync with the server: what we send over
    // its limit is dropped there, with nothing to show for it here.
    //
    constexpr std::uintmax_t attachments_limit (1023 * 1024 * 1024);

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

    // Start the reporter on the envelope without waiting for it, the same way
    // the crash daemon does.
    //
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

      // We have no use for the handles since we don't wait for it.
      //
      const auto_handle t (pi.hThread);
      const auto_handle p (pi.hProcess);

      return true;
    }

    // Return the path for the envelope of an event, where the crash daemon
    // writes its own.
    //
    std::filesystem::path
    envelope_path (const options& o, const std::string& id)
    {
      return o.database / "external" / (id + ".envelope");
    }

    // Hand a written envelope over to the reporter, which removes it once it
    // is done with it. If we can't, then the envelope is removed as f goes
    // out of scope.
    //
    bool
    hand_over (const options& o, auto_rmfile& f)
    {
      if (!launch (o.reporter, f.path))
        return false;

      f.cancel ();
      return true;
    }

    // Send an envelope the SDK produced, which is to say offer it to the
    // player. See make_transport() for details.
    //
    void
    send (sentry_envelope_t* e, void* s)
    {
      const options& o (*static_cast<const options*> (s));

      const std::unique_ptr<sentry_envelope_t, decltype (&sentry_envelope_free)>
        env (e, &sentry_envelope_free);

      // Drop what carries no event, such as client reports: there is nothing
      // to show the player, so nothing they could agree to.
      //
      const sentry_value_t ev (sentry_envelope_get_event (env.get ()));

      if (sentry_value_is_null (ev))
        return;

      std::error_code ec;
      if (!std::filesystem::exists (o.reporter, ec))
        return;

      // Note that the SDK only writes to a path in the ANSI code page, which
      // won't do for an installation under, say, a user name with non-ASCII
      // characters. So serialize it and write it ourselves.
      //
      size_t n (0);
      const std::unique_ptr<char, decltype (&sentry_free)> d (
        sentry_envelope_serialize (env.get (), &n), &sentry_free);

      if (d == nullptr)
        return;

      auto_rmfile f (envelope_path (
                       o,
                       sentry_value_as_string (
                         sentry_value_get_by_key (ev, "event_id"))));

      std::filesystem::create_directories (f.path.parent_path (), ec);

      {
        std::ofstream os (f.path, std::ios::binary | std::ios::trunc);
        os.write (d.get (), static_cast<std::streamsize> (n));

        if (!os.good ())
          return;
      }

      hand_over (o, f);
    }
  }

  sentry_transport_t*
  make_transport (const options& o)
  {
    sentry_transport_t* t (sentry_transport_new (&send));
    sentry_transport_set_state (t, const_cast<options*> (&o));
    return t;
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

      if (total + n > attachments_limit)
      {
        if (!skipped.empty ())
          skipped += ", ";

        skipped += p.filename ().string ();
        continue;
      }

      as.emplace_back (p, n);
      total += n;
    }

    // Build the event.
    //
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

    // Write the envelope. Until the reporter has it, it is ours to clean up
    // if anything goes wrong.
    //
    // See the envelope format at https://develop.sentry.dev/sdk/envelopes/.
    //
    auto_rmfile f (envelope_path (o, id));
    std::filesystem::create_directories (f.path.parent_path (), ec);

    {
      std::ofstream os (f.path, std::ios::binary | std::ios::trunc);

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
             p.extension () == ".log"
             ? "text/plain"
             : "application/octet-stream");

        os << json (ah) << '\n';

        // Note that inserting an empty stream buffer sets failbit.
        //
        if (n != 0)
          os << is.rdbuf ();

        os << '\n';
      }

      if (!os.good ())
        return false;
    }

    return hand_over (o, f);
  }
}
