#pragma once

#include "../Types.hpp"

#include "Engine.hpp"
#include "../Haptic/Effect.hpp"

namespace Controller
{
  namespace engine
  {
    bool
    effect_from_rumble (const Game::RumbleInfo&,
                        float scale,
                        bool loop,
                        haptic::effect& out) noexcept;
  }
}
