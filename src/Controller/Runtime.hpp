#pragma once

#include "Types.hpp"

#include "Clock.hpp"
#include "Context.hpp"
#include "Diagnostic.hpp"

#include "Device/Id.hpp"
#include "Device/Registry.hpp"
#include "Device/Discovery.hpp"
#include "Driver/Set.hpp"
#include "Transport/XInputModule.hpp"
#include "Sample/Frame.hpp"
#include "Calibration/Store.hpp"
#include "Engine/Dvar.hpp"
#include "Engine/Key.hpp"
#include "Engine/Bind.hpp"
#include "Engine/View.hpp"
#include "Engine/Feedback.hpp"

namespace Controller
{
  class runtime
  {
  public:
    explicit
    runtime (bool developer);

    ~runtime ();

    runtime (const runtime&) = delete;
    runtime& operator= (const runtime&) = delete;
    runtime (runtime&&) = delete;
    runtime& operator= (runtime&&) = delete;

    context
    make_context () noexcept;

    diagnostic_sink&
    diagnostics () noexcept {return sink_;}

    void
    engine_ready ();

    void
    frame ();

    device_id
    active () const noexcept {return active_;}

    const input_frame&
    latest () const noexcept {return latest_;}

    engine::key_dispatcher&
    keys () noexcept {return keys_;}

    engine::bind_bridge&
    binds () noexcept {return binds_;}

    engine::view_driver&
    view () noexcept {return view_;}

    bool
    driving () const noexcept
    {
      return active_ != no_device && keys_.in_use () &&
             engine::read (dvars_.enabled, true);
    }

    const engine::dvars&
    dvars () const noexcept {return dvars_;}

    bool
    submit (const driver::output_request&);

    void
    stop_haptic (uint32_t tag);

    std::string
    active_diagnostics () const;

    size_t
    device_count () const noexcept {return drivers_.size ();}

  private:
    bool
    advance (driver::driver&, const device_connection&, input_frame& out);

    const calibration::profile&
    profile_for (Controller::family);

    void
    apply_output_policy ();

    void
    apply_light_bar ();

    void
    apply_trigger_feedback ();

    logging_sink sink_;
    bool developer_;

    context ctx_;
    transport::xinput_module xinput_;
    registry devices_;
    discovery discovery_;
    driver::set drivers_;
    calibration::store calibration_;

    engine::dvars dvars_ {};
    engine::key_dispatcher keys_;
    engine::bind_bridge binds_;
    engine::view_driver view_;

    bool engine_ready_ {false};

    std::array<std::optional<calibration::profile>, 5> profiles_;

    static_assert (static_cast<size_t> (Controller::family::dualsense_edge) == 4,
                   "profiles_ is indexed by family and must cover every one");

    device_id active_ {};
    input_frame latest_ {};

    uint64_t sequence_ {0};
    uint64_t last_published_ {0};

    bool had_device_ {false};

    device_id lit_device_ {};
    uint32_t lit_colour_ {0};

    device_id felt_device_ {};
    driver::adaptive_trigger_request felt_left_ {};
    driver::adaptive_trigger_request felt_right_ {};
  };
}
