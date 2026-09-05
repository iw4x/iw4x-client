#include "Stream.hpp"

#include "../Types.hpp"

#include <algorithm>

#include "../Driver/OutputReport.hpp"

namespace Controller
{
  namespace haptic
  {
    stream::
    stream (const context& ctx,
            device_id device,
            connection link,
            transport::hid_device& hid)
      : ctx_ (ctx),
        device_ (device),
        link_ (link),
        hid_ (hid)
    {
      start_reports ();
    }

    void
    stream::
    start_reports ()
    {
      rate_ = driver::haptic_sample_rate;
      block_.resize (driver::haptic_frames_per_report);

      const std::chrono::nanoseconds period
      {
        std::chrono::nanoseconds::rep (1000000000ull *
                                       driver::haptic_frames_per_report /
                                       driver::haptic_sample_rate)
      };

      reports_ = std::make_unique<transport::report_stream> (
        ctx_, device_, hid_, period, driver::ds_haptic_report_size,
        [this] (std::span<std::byte> out) {return produce (out);});
    }

    void
    stream::
    start_audio ()
    {
      rate_ = 0;
      block_.clear ();

      audio_ = std::make_unique<transport::audio_endpoint> (
        ctx_, device_, hid_.path (),
        [this] (std::span<frame> f)
        {
          rate_ = audio_->sample_rate ();
          mixer_.render (f, rate_);
        });
    }

    bool
    stream::
    running () noexcept
    {
      if (reports_ != nullptr)
      {
        if (reports_->running ())
          return true;

        if (link_ != connection::usb || !reports_->failed ())
          return false;

        ctx_.report (severity::info, facility::transport, errc::output_rejected,
                     device_,
                     "the controller would not carry the haptic report stream over "
                     "USB; falling back to its audio endpoint, which sits behind "
                     "the Windows audio pipeline and lags further behind the game");

        reports_.reset ();
        start_audio ();
      }

      return audio_ != nullptr && audio_->running ();
    }

    std::optional<size_t>
    stream::
    produce (std::span<std::byte> out) noexcept
    {
      std::fill (block_.begin (), block_.end (), frame {});
      mixer_.render (block_, rate_);

      return driver::encode_dualsense_haptics (block_, counter_, out);
    }

    std::string
    stream::
    status () const
    {
      std::string s (reports_ != nullptr ? reports_->status ()
                   : audio_ != nullptr   ? audio_->status ()
                                         : std::string ("not started"));

      if (const uint64_t dropped = mixer_.dropped (); dropped != 0)
        s += ", " + std::to_string (dropped) +
             " effect(s) dropped for want of a voice";

      return s;
    }

    void
    stream::
    poll_diagnostics ()
    {
      const uint64_t dropped (mixer_.dropped ());

      if (dropped == reported_drops_)
        return;

      ctx_.report (severity::warning, facility::driver, errc::output_rejected,
                   device_,
                   "controller haptics dropped " +
                   std::to_string (dropped - reported_drops_) +
                   " effect(s) for want of a voice; effects are being started "
                   "faster than " + std::to_string (mixer::voices) +
                   " can carry them");

      reported_drops_ = dropped;
    }
  }
}
