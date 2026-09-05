#include "Runtime.hpp"

#include "Types.hpp"

#include <cassert>
#include <system_error>

#include "Trace.hpp"
#include "Calibration/Normalize.hpp"
#include "Engine/Command.hpp"

namespace Controller
{
  namespace
  {
    std::filesystem::path
    calibration_directory ()
    {
      std::error_code ec;
      const std::filesystem::path here (std::filesystem::current_path (ec));

      return (ec ? std::filesystem::path () : here) / "players" / "controller";
    }
  }

  runtime::
  runtime (bool developer)
    : developer_ (developer),
      ctx_ (sink_, developer),
      xinput_ (ctx_),
      devices_ (ctx_),
      discovery_ (ctx_, devices_, xinput_),
      drivers_ (ctx_, xinput_),
      calibration_ (ctx_, calibration_directory ()),
      keys_ (ctx_, dvars_),
      binds_ (ctx_, dvars_),
      view_ (ctx_, dvars_)
  {
    report (sink_, severity::info, facility::runtime, errc::none,
            "controller runtime initialized");
  }

  runtime::
  ~runtime () = default;

  context
  runtime::
  make_context () noexcept
  {
    return context (sink_, developer_);
  }

  void
  runtime::
  engine_ready ()
  {
    if (engine_ready_)
      return;

    dvars_ = engine::register_dvars (ctx_);
    engine::register_commands (ctx_, *this);

    binds_.apply_configured_layout ();

    discovery_.scan_now ();

    engine_ready_ = true;
  }

  const calibration::profile&
  runtime::
  profile_for (Controller::family f)
  {
    std::optional<calibration::profile>& slot (profiles_[static_cast<size_t> (f)]);

    if (!slot)
    {
      std::optional<calibration::profile> stored (calibration_.load (f, std::nullopt));
      slot = stored ? std::move (*stored) : calibration::default_profile (f);
    }

    return *slot;
  }

  bool
  runtime::
  advance (driver::driver& d, const device_connection& dc, input_frame& out)
  {
    raw_sample raw;
    canonical_sample canonical;

    const timestamp acquired (clock::now ());

    if (!d.poll (raw, canonical))
      return false;

    calibration::apply (profile_for (dc.identity.family), raw, canonical);

    out = input_frame {dc.id,
                       dc.identity.family,
                       dc.link,
                       ++sequence_,
                       latency_span {acquired, timestamp {}},
                       canonical};
    return true;
  }

  void
  runtime::
  submit (const driver::output_request& r)
  {
    if (active_ != no_device)
      drivers_.submit (active_, r);
  }

  void
  runtime::
  stop_haptic (uint32_t tag)
  {
    if (active_ != no_device)
      drivers_.stop_haptic (active_, tag);
  }

  std::string
  runtime::
  active_diagnostics () const
  {
    return active_ != no_device ? drivers_.diagnostics (active_) : std::string ();
  }

  void
  runtime::
  apply_output_policy ()
  {
    driver::output_policy p;

    p.rumble = engine::read (dvars_.enabled, true) &&
               engine::read (dvars_.rumble, true);
    p.haptics = engine::read (dvars_.haptics, true)
      ? driver::haptic_mode::waveform
      : driver::haptic_mode::emulated;
    p.output_interval =
      static_cast<unsigned> (std::max (0, engine::read (dvars_.output_interval, 4)));

    drivers_.configure (p);
  }

  void
  runtime::
  apply_light_bar ()
  {
    const bool ps (latest_.family == Controller::family::dualshock4 ||
                   latest_.family == Controller::family::dualsense ||
                   latest_.family == Controller::family::dualsense_edge);

    if (!ps || !latest_.state.caps.has (capability::light_bar))
      return;

    const bool on (engine::read (dvars_.light_bar, true));

    if (!on)
    {
      lit_device_ = no_device;
      lit_colour_ = 0;
      return;
    }

    const auto clamp8 = [] (int v) -> uint8_t
    {
      return static_cast<uint8_t> (v < 0 ? 0 : (v > 255 ? 255 : v));
    };

    const uint8_t r (clamp8 (engine::read (dvars_.light_bar_r, 196)));
    const uint8_t g (clamp8 (engine::read (dvars_.light_bar_g, 151)));
    const uint8_t b (clamp8 (engine::read (dvars_.light_bar_b, 54)));

    const uint32_t colour (static_cast<uint32_t> (r) << 16 |
                           static_cast<uint32_t> (g) << 8 | b);

    if (active_ == lit_device_ && colour == lit_colour_)
      return;

    drivers_.submit (active_, driver::light_bar_request {r, g, b});

    lit_device_ = active_;
    lit_colour_ = colour;
  }

  void
  runtime::
  apply_trigger_feedback ()
  {
    if (!latest_.state.caps.has (capability::adaptive_triggers))
      return;

    driver::adaptive_trigger_request left {};
    driver::adaptive_trigger_request right {};

    if (!engine::evaluate_trigger_feedback (dvars_, engine::local_client,
                                            left, right))
    {
      felt_device_ = no_device;
      return;
    }

    const auto same = [] (const driver::adaptive_trigger_request& a,
                          const driver::adaptive_trigger_request& b) noexcept
    {
      return a.effect == b.effect &&
             a.start_position == b.start_position &&
             a.end_position == b.end_position &&
             a.strength == b.strength;
    };

    const bool known (active_ == felt_device_);

    if (!known || !same (left, felt_left_))
    {
      drivers_.submit (active_, left);
      felt_left_ = left;
    }

    if (!known || !same (right, felt_right_))
    {
      drivers_.submit (active_, right);
      felt_right_ = right;
    }

    felt_device_ = active_;
  }

  void
  runtime::
  frame ()
  {
    CONTROLLER_ZONE ("controller::frame");

    if (!engine_ready_)
    {
      CONTROLLER_FRAME_MARK ();
      return;
    }

    binds_.poll_configured_layout ();

    discovery_.scan ();
    drivers_.reconcile (devices_);

    engine::publish_present (dvars_, drivers_.size () != 0);

    apply_output_policy ();

    if (!engine::read (dvars_.enabled, true) || drivers_.size () == 0)
    {
      if (had_device_)
      {
        keys_.release_all ();
        view_.idle ();
        active_ = no_device;
        had_device_ = false;

        lit_device_ = no_device;
        lit_colour_ = 0;
        felt_device_ = no_device;
      }

      CONTROLLER_FRAME_MARK ();
      return;
    }

    input_frame candidate;
    bool have_candidate (false);

    drivers_.for_each ([this, &candidate, &have_candidate]
                       (driver::driver& d, const device_connection& dc)
    {
      input_frame f;

      if (!advance (d, dc, f))
        return;

      if (!have_candidate || dc.id == active_)
      {
        candidate = std::move (f);
        have_candidate = true;
      }
    });

    if (have_candidate)
    {
      latest_ = std::move (candidate);
      latest_.timing.consumed = clock::now ();

      assert (latest_.sequence > last_published_);
      last_published_ = latest_.sequence;

      if (active_ != latest_.device)
        keys_.release_all ();

      active_ = latest_.device;
      had_device_ = true;

      keys_.dispatch (latest_.state);

      view_.observe (latest_.state);

      apply_light_bar ();
      apply_trigger_feedback ();
    }

    CONTROLLER_FRAME_MARK ();
  }
}
