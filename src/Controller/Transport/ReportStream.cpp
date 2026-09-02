#include "ReportStream.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace transport
  {
    report_stream::
    report_stream (const context& ctx,
                   device_id device,
                   hid_device& hid,
                   std::chrono::nanoseconds period,
                   size_t capacity,
                   producer p)
      : hid_ (hid),
        produce_ (std::move (p)),
        period_ (period),
        capacity_ (capacity),
        thread_ ([this, ctx, device] (std::stop_token t)
                 {run (std::move (t), ctx, device);})
    {
    }

    std::string
    report_stream::
    status () const
    {
      std::lock_guard lock (status_mutex_);
      return status_;
    }

    void
    report_stream::
    note (std::string s) const
    {
      std::lock_guard lock (status_mutex_);
      status_ = std::move (s);
    }

    void
    report_stream::
    run (std::stop_token stop, context ctx, device_id device) noexcept
    {
      if (period_ <= std::chrono::nanoseconds::zero () || capacity_ == 0)
      {
        note ("a report stream was asked for with no cadence to keep");
        return;
      }

      std::vector<std::byte> report (capacity_);

      bool started (false);

      auto due (std::chrono::steady_clock::now ());

      while (!stop.stop_requested ())
      {
        const std::optional<size_t> size (produce_ (report));

        if (!size)
        {
          note (started ? "the report stream ended"
                        : "no report could be produced for this device");
          break;
        }

        if (!hid_.write (std::span<const std::byte> (report.data (), *size)))
        {
          note (started ? "the device stopped accepting the report stream"
                        : "the device refused the report");

          if (!started)
            ctx.report (severity::warning, facility::transport,
                        errc::output_rejected, device,
                        "the controller did not accept the output report this "
                        "stream is built on, so it cannot be driven this way");
          break;
        }

        if (!started)
        {
          started = true;
          running_.store (true, std::memory_order_release);

          note ("streaming reports every " +
                std::to_string (
                  std::chrono::duration_cast<std::chrono::microseconds> (
                    period_).count ()) + " us");
        }

        due += period_;

        const auto now (std::chrono::steady_clock::now ());

        if (due > now)
          std::this_thread::sleep_until (due);
        else
        {
          due = now;
        }
      }

      running_.store (false, std::memory_order_release);
    }
  }
}
