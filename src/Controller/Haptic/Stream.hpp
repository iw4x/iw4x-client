#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Device/Id.hpp"
#include "../Transport/Audio.hpp"
#include "../Transport/Hid.hpp"
#include "../Transport/ReportStream.hpp"

#include "Effect.hpp"
#include "Mixer.hpp"

namespace Controller
{
  namespace haptic
  {
    class stream
    {
    public:
      stream (const context&,
              device_id,
              connection link,
              transport::hid_device&);

      void
      poll_diagnostics ();

      stream (const stream&) = delete;
      stream& operator= (const stream&) = delete;

      bool
      running () const noexcept
      {
        return audio_ != nullptr ? audio_->running ()
             : reports_ != nullptr && reports_->running ();
      }

      bool
      play (const haptic::effect& e) noexcept {return mixer_.play (e);}

      void
      stop (uint32_t tag) noexcept {mixer_.stop (tag);}

      void
      set_rumble (float low_frequency, float high_frequency) noexcept
      {
        if (mixer_.has_effects ())
        {
          mixer_.set_rumble (0.0f, 0.0f);
          return;
        }

        mixer_.set_rumble (low_frequency, high_frequency);
      }

      void
      silence () noexcept {mixer_.stop_all ();}

      std::string
      status () const;

    private:
      const context& ctx_;
      device_id device_;

      uint64_t reported_drops_ {0};

      std::optional<size_t>
      produce (std::span<std::byte>) noexcept;

      mixer mixer_;

      uint32_t rate_ {0};

      std::vector<frame> block_;
      uint8_t counter_ {0};

      std::unique_ptr<transport::audio_endpoint> audio_;
      std::unique_ptr<transport::report_stream> reports_;
    };
  }
}
