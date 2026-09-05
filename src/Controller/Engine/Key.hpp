#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Dvar.hpp"
#include "../Mapping/Key.hpp"
#include "../Sample/Sample.hpp"

namespace Controller
{
  namespace engine
  {
    inline constexpr int local_client {0};

    class key_dispatcher
    {
    public:
      key_dispatcher (const context&, const dvars&);

      void
      dispatch (const canonical_sample&) noexcept;

      void
      release_all () noexcept;

      bool
      in_use () const noexcept {return in_use_;}

      void
      note_other_input () noexcept;

    private:
      enum class key_event : uint8_t
      {
        pressed,
        repeated,
        released,
      };

      void
      set_in_use (bool) noexcept;

      unsigned
      release_delay () const noexcept;

      bool
      defers_release (mapping::engine_key) const noexcept;

      void
      emit_button (mapping::engine_key, key_event, unsigned time) noexcept;

      void
      emit (mapping::engine_key, key_event, unsigned time) noexcept;

      void
      dispatch_apad (unsigned time) noexcept;

      void
      dispatch_buttons (const button_set& current, unsigned time) noexcept;

      bool
      ignore_repeat (mapping::engine_key, int repeats, unsigned time) noexcept;

      void
      reset_scroll (mapping::engine_key, bool down, unsigned time) noexcept;

      void
      menu_key_event (mapping::engine_key, bool down) noexcept;

      bool
      scoreboard_key_event (mapping::engine_key) noexcept;

      const context& ctx_;
      const dvars& dvars_;

      bool in_use_ {false};

      bool reported_deadzone_ {false};

      button_set buttons_;

      static constexpr size_t button_state_count {
        static_cast<size_t> (button::count)};

      button_set deferred_;

      std::array<unsigned, button_state_count> pressed_at_ {};
      std::array<unsigned, button_state_count> released_at_ {};

      static constexpr size_t axis_count {4};

      std::array<std::array<bool, 2>, axis_count> deflected_ {};
      std::array<std::array<bool, 2>, axis_count> was_deflected_ {};

      unsigned next_scroll_ {0};
      unsigned scroll_hold_start_ {0};
      std::optional<mapping::engine_key> scroll_hold_key_;
    };
  }
}
