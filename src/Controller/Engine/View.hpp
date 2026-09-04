#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Dvar.hpp"
#include "../Aim/Assist.hpp"
#include "../Aim/Calibration.hpp"
#include "../Mapping/StickLayout.hpp"
#include "../Sample/Sample.hpp"

namespace Controller
{
  namespace engine
  {
    class view_driver
    {
    public:
      view_driver (const context&, const dvars&);

      void
      observe (const canonical_sample&) noexcept;

      void
      idle () noexcept;

      void
      apply_move (int client, usercmd_s& cmd, float frame_time) noexcept;

      void
      apply_remote_move (int client, usercmd_s& cmd) noexcept;

      void
      apply_location_selection (int client) noexcept;

    private:
      bool
      ensure_processor ();

      bool
      view_active (int client) const noexcept;

      void
      apply_lock_on (int client,
                     const AimInput&,
                     aim::aim_frame_output&) noexcept;

      const context& ctx_;
      const dvars& dvars_;

      mapping::resolved_axes axes_ {};

      std::optional<aim::aim_calibration> calibration_;
      std::optional<aim::aim_processor> processor_;

      size_t tuning_signature_ {0};
      bool have_signature_ {false};
      bool reported_invalid_ {false};

      dvar_t* cursor_speed_ {nullptr};
    };
  }
}
