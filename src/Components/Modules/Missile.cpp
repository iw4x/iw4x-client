#include "Missile.hpp"

namespace Components
{
	constexpr auto CONTENTS_SOLID = 1;
	constexpr auto NO_PASS_ENTITY = -1;
	constexpr auto LIFT_SEARCH_HEIGHT = 16.0f; // same as G_ExplodeMissile

	bool Missile::LiftOutOfSolid(const float* origin, const Game::gentity_s* inflictor, float* out)
	{
		// These are supplied by the G_ExplodeMissile call below and normally
		// cannot be missing. Still, both are dereferenced in this function and
		// there is no sensible correction we can produce without either one.
		//
		if (!origin || !inflictor)
			return false;

		// First see if there is anything to fix. Normally the explosion is
		// already in open space and we want to leave its origin exactly where
		// the game put it.
		//
		// The troublesome case is a stuck charge. Its resting point can be
		// slightly inside the surface so that the model sits against it. That
		// same point later becomes the origin of the radius damage traces.
		//
		if (!Game::G_PointContents(origin, NO_PASS_ENTITY, CONTENTS_SOLID))
			return false;

		// So origin is inside solid. We cannot learn where the surface is by
		// tracing out from that point in the same way as radius damage does:
		// starting inside the solid is what got us here ;)
		//
		// Instead, take a point 16 units above it and trace back down. For the
		// floor case we are fixing, start should now be in open space and the
		// trace should encounter the floor before reaching origin.
		//
		const Game::vec3_t start = { origin[0], origin[1], origin[2] + LIFT_SEARCH_HEIGHT };

		Game::trace_t trace{};

		// Use bounds_origin to make this a point trace and ignore the inflictor
		// itself since we are trying to find the piece of solid geometry containing
		// its origin.
		//
		Game::G_TraceCapsule(&trace, start, origin, Game::bounds_origin, inflictor->s.number, CONTENTS_SOLID);

		// From there, there are two ways this search can fail.
		//
		// If startsolid is set, then moving 16 units up did not get us out of
		// the solid. In that case the trace has no outside starting point from
		// which we can recover the surface.
		//
		// If fraction is 1, then the trace reached origin without encountering
		// solid. That is inconsistent with the contents test above from our
		// point of view, but it still leaves us without the surface point we
		// came here to find. Presumably this can happen with some of the less
		// straightforward collision geometry.
		//
		// In either case, keep the position supplied by the game.
		//
		if (trace.startsolid || trace.fraction >= 1.0f)
			return false;

		// Good, so start was outside and the trace hit solid on the way to
		// origin. trace.fraction is the position of that hit along the
		// start->origin segment. Reconstruct it here and return it as the point
		// from which radius damage should start.
		//
		// Note that, with the start used above, x and y happen to remain
		// unchanged and only z moves. Spell out the interpolation for all three
		// components anyway so this remains the trace hit point.
		//
		for (auto i = 0; i < 3; ++i)
			out[i] = ((origin[i] - start[i]) * trace.fraction) + start[i];

		return true;
	}

	void Missile::G_RadiusDamage_Hk(const float* origin,
		                              Game::gentity_s* inflictor,
																	Game::gentity_s* attacker,
																	float damage,
																	float minDamage,
																	float radius,
																	float coneDot,
																	const float* coneDir,
																	Game::gentity_s* ignore,
																	int meansOfDeath,
																	int hitLoc)
	{
		Game::vec3_t corrected;

		// G_ExplodeMissile is about to use origin as the start point for radius
		// damage. See if this is one of the stuck missiles whose resting point
		// ended up inside solid and, if so, recover the surface point first.
		//
		// Also keep corrected separate from origin. LiftOutOfSolid() only writes a
		// useful value after it has found the surface, so a failed attempt leaves
		// the original call completely untouched.
		//
		if (LiftOutOfSolid(origin, inflictor, corrected))
		{
			Logger::Debug("Explosion at ({}, {}, {}) was inside solid, damaging from ({}, {}, {})",
				origin[0], origin[1], origin[2], corrected[0], corrected[1], corrected[2]);

			// Notice that we are changing the local damage origin, not the
			// missile position. G_ExplodeMissile has already used the latter for
			// the explosion itself and it's only the point from which G_RadiusDamage
			// begins tracing that is wrong.
			//
			origin = corrected;
		}

		// Carry on with the original call. If there was no buried origin then
		// this is byte-for-byte the same set of arguments G_ExplodeMissile was
		// going to pass. If there was one, origin is the recovered surface point
		// and everything else retains the game's value.
		//
		Game::G_RadiusDamage(origin, inflictor, attacker, damage, minDamage, radius, coneDot, coneDir, ignore, meansOfDeath, hitLoc);
	}

	Missile::Missile()
	{
		// The problem here is a rather subtle consequence of how stuck missiles
		// are positioned.
		//
		// When a charge sticks to the floor, the game leaves its origin slightly
		// inside the surface. This is intentional as far as placement is
		// concerned: the charge sits flush with the floor instead of floating a
		// little above it. G_ExplodeMissile later takes that same origin and
		// passes it to G_RadiusDamage.
		//
		// Now, G_RadiusDamage finds targets from that point using traces whose
		// contents mask includes CONTENTS_SOLID. Starting one of those traces
		// inside the floor has the unfortunate result that the floor can block
		// the trace as it comes out. From the player's point of view the charge
		// explodes exactly where expected, yet somebody standing next to it can
		// receive no damage.
		//
		// There is a useful clue in G_MissileImpact. For an explosion that
		// happens directly on impact, that code still has the surface normal and
		// moves the explosion point out from the surface before doing radius
		// damage. So the impact path never hands G_RadiusDamage the buried point
		// in the first place.
		//
		// The stuck-missile path is later and has lost that surface normal. What
		// it does still have is the buried resting point and the knowledge that
		// G_ExplodeMissile itself considers 16 units enough when looking back
		// from a stuck missile. LiftOutOfSolid() uses those two facts to recover
		// the floor: If the origin is in solid, approach it from 16 units above and
		// use the downward trace to recover the surface. If this produces a surface
		// point, use that point for radius damage.
		//
		Utils::Hook(0x477EF5, G_RadiusDamage_Hk, HOOK_CALL).install()->quick();
	}
}
