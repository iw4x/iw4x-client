#include "Feedback.hpp"

#include "../Types.hpp"

#include <cstring>

#include "Key.hpp"
#include "../Mapping/Key.hpp"

namespace Controller
{
  namespace engine
  {
    namespace
    {
      using driver::adaptive_trigger_request;
      using driver::trigger_effect;

      enum class trigger_role : uint8_t
      {
        none,
        firing,
        aiming,
        primary_offhand,
        secondary_offhand,
      };

      trigger_role
      role_for (mapping::engine_key k) noexcept
      {
        const char* const b (
          playerKeys[local_client].keys[static_cast<int> (k)].binding);

        if (b == nullptr)
          return trigger_role::none;

        if (std::strcmp (b, "+attack") == 0)
          return trigger_role::firing;

        if (std::strcmp (b, "+speed_throw") == 0 ||
            std::strcmp (b, "+toggleads_throw") == 0)
          return trigger_role::aiming;

        if (std::strcmp (b, "+frag") == 0)
          return trigger_role::primary_offhand;

        if (std::strcmp (b, "+smoke") == 0)
          return trigger_role::secondary_offhand;

        return trigger_role::none;
      }

      adaptive_trigger_request
      released (trigger_side side) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::off;
        return r;
      }

      adaptive_trigger_request
      continuous (trigger_side side, uint8_t strength) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::feedback;
        r.start_position = 1;
        r.strength = strength;
        return r;
      }

      adaptive_trigger_request
      section (trigger_side side, uint8_t start, uint8_t end) noexcept
      {
        adaptive_trigger_request r;
        r.side = side;
        r.effect = trigger_effect::weapon;
        r.start_position = start;
        r.end_position = end;
        return r;
      }

      constexpr uint8_t slight {3};
      constexpr uint8_t heavy {7};

      constexpr uint8_t light_break_start {3};
      constexpr uint8_t light_break_end {5};
      constexpr uint8_t hard_break_start {5};
      constexpr uint8_t hard_break_end {8};

      adaptive_trigger_request
      firing_feedback (trigger_side side, const playerState_s& ps) noexcept
      {
        const int index (BG_GetViewModelWeaponIndex (&ps));

        if (index == 0)
          return released (side);

        const WeaponDef* const w (
          BG_GetWeaponDef (static_cast<unsigned> (index)));

        if (w == nullptr)
          return released (side);

        switch (w->weapClass)
        {
          case Game::WEAPCLASS_MG:
          case Game::WEAPCLASS_RIFLE:
          case Game::WEAPCLASS_TURRET:
            return continuous (side, heavy);

          case Game::WEAPCLASS_SMG:
            return continuous (side, slight);

          case Game::WEAPCLASS_PISTOL:
            return section (side, light_break_start, light_break_end);

          case Game::WEAPCLASS_SPREAD:
          case Game::WEAPCLASS_SNIPER:
          case Game::WEAPCLASS_ROCKETLAUNCHER:
            return section (side, hard_break_start, hard_break_end);

          default:
            return released (side);
        }
      }

      adaptive_trigger_request
      offhand_feedback (trigger_side side,
                        const playerState_s& ps,
                        bool primary) noexcept
      {
        const int held (primary ? ps.weapCommon.offhandPrimary
                                : ps.weapCommon.offhandSecondary);

        return held != Game::OFFHAND_CLASS_NONE
          ? section (side, light_break_start, light_break_end)
          : released (side);
      }
    }

    bool
    evaluate_trigger_feedback (const dvars& d,
                               int client,
                               adaptive_trigger_request& left,
                               adaptive_trigger_request& right) noexcept
    {
      if (!read (d.adaptive_triggers, true))
        return false;

      Game::cg_s* const cg (Game::CL_GetLocalClientGlobals (client));

      if (cg == nullptr || cg->snap == nullptr)
        return false;

      const playerState_s& ps (cg->snap->ps);

      const trigger_role l (role_for (mapping::engine_key::button_ltrig));
      const trigger_role r (role_for (mapping::engine_key::button_rtrig));

      const auto effect_for = [&ps] (trigger_side side, trigger_role role)
      {
        switch (role)
        {
          case trigger_role::firing:
            return firing_feedback (side, ps);

          case trigger_role::aiming:
          case trigger_role::none:
            return released (side);

          case trigger_role::primary_offhand:
            return offhand_feedback (side, ps, true);

          case trigger_role::secondary_offhand:
            return offhand_feedback (side, ps, false);
        }

        return released (side);
      };

      left = effect_for (trigger_side::left, l);
      right = effect_for (trigger_side::right, r);

      if (l == trigger_role::firing && r == trigger_role::firing)
      {
        const int index (BG_GetViewModelWeaponIndex (&ps));

        if (index != 0)
        {
          const PlayerEquippedWeaponState* const e (
            BG_GetEquippedWeaponState (const_cast<playerState_s*> (&ps),
                                       static_cast<unsigned> (index)));

          if (e != nullptr && e->dualWielding)
          {
            right = left;
            right.side = trigger_side::right;
          }
        }
      }

      return true;
    }
  }
}
