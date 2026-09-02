#pragma once

#include "../Types.hpp"

#include "Engine.hpp"
#include "../Context.hpp"
#include "Dvar.hpp"
#include "../Driver/Output.hpp"

namespace Controller
{
  namespace engine
  {
    bool
    evaluate_trigger_feedback (const dvars&,
                               int client,
                               driver::adaptive_trigger_request& left,
                               driver::adaptive_trigger_request& right) noexcept;
  }
}
