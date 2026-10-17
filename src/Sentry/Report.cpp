#include "Report.hpp"

#include "Types.hpp"

#include "Engine/State.hpp"

namespace Sentry
{
  namespace
  {
    // Relay (Sentry's ingestion service) by default rejects an envelope over
    // 200 MiB, which also bounds every attachment in it. Keep some room for
    // the event and the item headers.
    //
    // Note that sentry.io also rejects a compressed request over 40 MB, which
    // fastfiles, being compressed already, will mostly hit first. A
    // self-hosted Relay can raise all of this (see its limits section).
    //
    constexpr std::uintmax_t attachments_limit (199 * 1024 * 1024);

    // Close a Windows handle when it goes out of scope.
    //
    class auto_handle
    {
    public:
      explicit
      auto_handle (HANDLE h = nullptr) noexcept: h_ (h) {}

      auto_handle (auto_handle&& x) noexcept: h_ (x.release ()) {}

      auto_handle&
      operator= (auto_handle&& x) noexcept
      {
        reset (x.release ());
        return *this;
      }

      auto_handle (const auto_handle&) = delete;
      auto_handle& operator= (const auto_handle&) = delete;

      ~auto_handle () noexcept {reset ();}

      HANDLE
      get () const noexcept {return h_;}

      HANDLE
      release () noexcept
      {
        HANDLE r (h_);
        h_ = nullptr;
        return r;
      }

      void
      reset (HANDLE h = nullptr) noexcept
      {
        if (h_ != nullptr)
          CloseHandle (h_);

        h_ = h;
      }

    private:
      HANDLE h_;
    };

    // Remove a file when it goes out of scope, unless cancelled.
    //
    class auto_rmfile
    {
    public:
      explicit
      auto_rmfile (std::filesystem::path p) noexcept: path_ (std::move (p)) {}

      auto_rmfile (auto_rmfile&& x) noexcept: path_ (std::move (x.path_))
      {
        x.cancel ();
      }

      auto_rmfile&
      operator= (auto_rmfile&& x) noexcept
      {
        if (this != &x)
        {
          remove ();
          path_ = std::move (x.path_);
          x.cancel ();
        }

        return *this;
      }

      auto_rmfile (const auto_rmfile&) = delete;
      auto_rmfile& operator= (const auto_rmfile&) = delete;

      ~auto_rmfile () noexcept {remove ();}

      const std::filesystem::path&
      path () const noexcept {return path_;}

      void
      cancel () noexcept {path_.clear ();}

    private:
      void
      remove () noexcept
      {
        if (!path_.empty ())
        {
          std::error_code ec;
          std::filesystem::remove (path_, ec);
        }
      }

      std::filesystem::path path_;
    };

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

    // Write the envelope where the crash daemon writes its own. Until the
    // reporter has it, it is ours to clean up if anything goes wrong.
    //
    // See the envelope format at https://develop.sentry.dev/sdk/envelopes/.
    //
    auto_rmfile f (o.database / "external" / (id + ".envelope"));
    std::filesystem::create_directories (f.path ().parent_path (), ec);

    {
      std::ofstream os (f.path (), std::ios::binary | std::ios::trunc);

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
        return false;
    }

    // Hand the envelope over. From here on the reporter removes it once it is
    // done with it.
    //
    if (!launch (o.reporter, f.path ()))
      return false;

    f.cancel ();
    return true;
  }
}
