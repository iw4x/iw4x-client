#include "RemoteControl.hpp"
#include "Events.hpp"

namespace Components
{
	Dvar::Var RemoteControl::MRemoteControlSensitivity;

	float RemoteControl::PendingYaw = 0.0f;
	float RemoteControl::PendingPitch = 0.0f;
	float RemoteControl::DeflectionYaw = 0.0f;
	float RemoteControl::DeflectionPitch = 0.0f;
	int RemoteControl::LastTime = 0;

	constexpr auto CL_GetMouseMovementWithSensitivity = 0x5A6170;
	constexpr auto mouseFrameTime = 0xB2BB58u;
	constexpr auto m_yaw = 0xA7FE78u;
	constexpr auto m_pitch = 0xA7FE80u;
	constexpr auto sessionGap = 250;
	constexpr auto referenceFrameRate = 85.0f;
	constexpr auto deflectionResponseSeconds = 0.07f;
	constexpr auto deflectionSlewSeconds = 0.1f;
	constexpr auto deflectionSettle = 0.0005f;
	constexpr auto maxFrameSeconds = 0.1f;

	float RemoteControl::DeflectionLimit(unsigned int dvarAddress)
	{
		const auto* var = *reinterpret_cast<Game::dvar_t**>(dvarAddress);
		if (!var)
			return 0.0f;

		const auto factor = std::fabs(var->current.value);
		if (factor <= 0.0f)
			return 0.0f;

		// Maybe this looks a bit backwards. We are interested in the mouse
		// value before m_yaw/m_pitch is applied, yet the limit we care about
		// lives after that multiplication:
		//
		//   clamp (factor * value, -1, 1)
		//
		// So invert the factor and we get the last input value that still fits.
		// Anything larger would look exactly the same to the code after the
		// clamp, which means we would have no way to account for the excess.
		//
		return 1.0f / factor;
	}

	void RemoteControl::Steer(float* value, float limit, float& pending, float& deflection, float referenceSeconds, float step, float frameSeconds)
	{
		if (limit <= 0.0f)
		{
			pending = 0.0f;
			deflection = 0.0f;
			return;
		}

		// The slightly strange part here is that the mouse gives us movement,
		// yet remoteControlAngles is later treated as something closer to a
		// stick position. So the value eventually becomes a rate.
		//
		// Maybe we could just multiply *value by referenceSeconds / step here.
		// That is in fact the basic correction. The problem is the clamp. Once
		// the corrected value no longer fits, the excess disappears and the
		// correction starts losing movement again.
		//
		// Keep that movement here instead. `pending` is effectively the amount
		// of steering we still owe, with time folded into it.
		//
		pending += *value * referenceSeconds;

		// There is one more problem now. `want` is what we would have put
		// straight into remoteControlAngles before adding the steering dynamics.
		// Doing that makes the mouse correction work, though it leaves us with
		// an infinitely quick virtual stick: one mouse count can move it from
		// center to full deflection in one frame.
		//
		// That doesn't seem to be what we want for the Predator. So take this as
		// the position the mouse is asking for and let the actual deflection
		// move there below.
		//
		const auto want = pending / step;
		const auto target = std::clamp(want, -limit, limit);

		// Note that this uses `target`, not the deflection we eventually send.
		// This is subtle, but mixing the two would make the steering response
		// part of the pending calculation. A slow response would then look like
		// undelivered mouse movement and come back later as accumulated input.
		//
		// `pending` only has to remember what did not fit through the clamp.
		//
		pending -= target * step;

		// What about keeping all of the remainder?
		//
		// Maybe, except then sustained full deflection starts building a queue.
		// Once the mouse stops the camera would keep turning until that queue is
		// drained. What we really want to preserve is the small remainder caused
		// by a mouse count landing in one frame instead of the next.
		//
		// One frame at full deflection is enough for that. Past this point the
		// player is simply asking the remote camera to turn faster than it can.
		//
		const auto carry = limit * step;
		pending = std::clamp(pending, -carry, carry);

		// Now, how should the actual deflection follow `target`?
		//
		// A fixed amount per frame sounds reasonable at first. The trouble is
		// that our target is made from mouse samples. At 250 fps with, say, a
		// 125 Hz mouse, a slow movement may give us a count on one frame and
		// zero on the next. So what looks like a rapidly changing target here
		// can really be one perfectly steady movement of the hand.
		//
		// Moving a fraction of the distance each time seems to behave much
		// better. A frame with no count pulls towards zero only in proportion
		// to how far away we are, and the next count builds from what remains.
		// Over several samples we get the average request without the camera
		// jumping to every individual mouse sample.
		//
		const auto blend = 1.0f - std::exp(-frameSeconds / deflectionResponseSeconds);
		auto move = (target - deflection) * blend;

		// Maybe that would be enough on its own. There is still the large-flick
		// case though. If `target` suddenly jumps from zero to the limit, the
		// exponential step can move quite a lot of deflection in one frame.
		//
		// Put a limit on that change as well. Normally the response above gets
		// there first. This mainly stops a large mouse sample from buying an
		// almost immediate full turn.
		//
		const auto reach = (limit / deflectionSlewSeconds) * frameSeconds;
		move = std::clamp(move, -reach, reach);

		// Reversing direction needs a little care as well. Say we are turning
		// right and `target` suddenly moves left. A sufficiently large `move`
		// could cross zero and spend the rest of the same frame building left
		// deflection.
		//
		// That makes the reversal lose some of the weight we just added. Stop
		// at zero and let the next frame start the turn in the other direction.
		//
		if (deflection * (deflection + move) < 0.0f)
			move = -deflection;

		// The limit comes from a dvar, so it may have changed since the previous
		// frame. Clamp again here and don't assume the old deflection still fits.
		//
		deflection = std::clamp(deflection + move, -limit, limit);

		// One last nuisance with the exponential response is that zero is only
		// approached, never quite reached. For a UI animation that would hardly
		// matter. Here even a tiny value still asks the missile to keep turning.
		//
		// So call the very small tail zero. This has to stay small enough that a
		// real slow movement cannot get caught here on every empty mouse sample.
		//
		if (std::fabs(deflection) < limit * deflectionSettle)
			deflection = 0.0f;

		*value = deflection;
	}

	void RemoteControl::ScaleMouseMovement(float* yaw, float* pitch)
	{
		const auto sensitivity = MRemoteControlSensitivity.get<float>();
		if (sensitivity <= 0.0f)
		{
			PendingYaw = 0.0f;
			PendingPitch = 0.0f;
			DeflectionYaw = 0.0f;
			DeflectionPitch = 0.0f;
			return;
		}

		// So why does remote steering need this in the first place?
		//
		// With normal mouse look, moving the mouse gives us a displacement.
		// Split that movement over twice as many frames and each frame gets less
		// of it, though the accumulated angle remains the same.
		//
		// Remote control is different. The game puts the values in
		// remoteControlAngles and later does roughly:
		//
		//   angle += deflection * steerRate * seconds
		//
		// In other words it treats the mouse value like a stick deflection. That
		// works for a stick since holding it halfway still gives half deflection
		// every frame. A mouse gives less displacement per frame as the frame
		// rate increases, so the game accidentally turns that into a lower rate.
		//
		// Maybe the cleanest fix would be to change the representation itself,
		// but at this point we are dealing with the byte the original game
		// expects. So give each mouse count the same time weight it had at the
		// stock 85 fps and fit that into the current frame instead.
		//
		// And there is a second problem hiding behind the first one. Once the
		// mouse displacement has been turned into a stick deflection, sending
		// that deflection immediately makes the Predator follow the mouse much
		// too directly. We still want the stick to have some weight. Steer()
		// keeps the corrected mouse value as the requested position and lets the
		// position we actually send catch up to it.
		//
		const auto now = Game::cls->realtime;

		if (now - LastTime > sessionGap)
		{
			// Presumably this is another remote camera. We don't have a useful
			// identity for it here, so use the gap between calls.
			//
			// There are now two things that must not survive that gap. `pending`
			// is mouse movement left from the old camera and `deflection` is
			// where its virtual stick had reached. Either one would make the new
			// camera begin with movement from the previous one.
			//
			PendingYaw = 0.0f;
			PendingPitch = 0.0f;
			DeflectionYaw = 0.0f;
			DeflectionPitch = 0.0f;
		}

		LastTime = now;

		const auto referenceSeconds = sensitivity / referenceFrameRate;
		const auto frameSeconds = static_cast<float>(*reinterpret_cast<int*>(mouseFrameTime)) * 0.001f;

		// Could use frameSeconds directly for the whole range. Then a client
		// below 85 fps would get its existing sensitivity changed too. That's
		// outside the problem here, so stop scaling once we reach the reference
		// frame time.
		//
		const auto step = std::min(frameSeconds, 1.0f / referenceFrameRate);

		// The new steering response is timed differently. Here we really do
		// mean "take this many seconds to move the stick", so using the capped
		// `step` would make that response depend on which side of 85 fps we are
		// on.
		//
		// Use the real frame time for it. Well, almost. A long hitch shouldn't
		// count as the player spending half a second moving the stick. Cap that
		// case so one bad frame doesn't skip most of the response.
		//
		const auto elapsedSeconds = std::min(frameSeconds, maxFrameSeconds);

		Steer(yaw, DeflectionLimit(m_yaw), PendingYaw, DeflectionYaw, referenceSeconds, step, elapsedSeconds);
		Steer(pitch, DeflectionLimit(m_pitch), PendingPitch, DeflectionPitch, referenceSeconds, step, elapsedSeconds);
	}

	__declspec(naked) void RemoteControl::CL_GetMouseMovementWithSensitivity_Stub()
	{
		__asm
		{
			// This looks a little odd with the same offset twice. On entry the
			// original arguments are at [esp + 4] and [esp + 8]. Push the second
			// one first and the first one moves to [esp + 8], hence the repeat.
			//
			push [esp + 0x8]
			push [esp + 0x8]
			call CL_GetMouseMovementWithSensitivity
			add esp, 0x8

			// Now we have the exact values the original call would have produced.
			// Save the registers before making our extra C++ call since the code
			// around this call site knows nothing about it.
			//
			pushad

			// And, yes, the offsets move again after each push. PUSHAD accounts
			// for the first 0x20 bytes, then pushing pitch moves yaw by another
			// word.
			//
			push [esp + 0x20 + 0x8]
			push [esp + 0x24 + 0x4]
			call ScaleMouseMovement
			add esp, 0x8

			popad

			ret
		}
	}

	RemoteControl::RemoteControl()
	{
		if (Dedicated::IsEnabled())
			return;

		// Maybe hooking CL_GetMouseMovementWithSensitivity itself would seem
		// simpler. CL_MouseMove uses it too though, and normal mouse look does
		// not have this problem. So hook the one call made by
		// CL_RemoteControlMove and leave the ordinary path alone.
		//
		Utils::Hook(0x5A6C5D, CL_GetMouseMovementWithSensitivity_Stub, HOOK_CALL).install()->quick();

		Events::OnDvarInit([]
		{
			// This comes after the regular mouse sensitivity has already been
			// applied. Zero gives the game its original behavior, and larger
			// values ask for more deflection from the same mouse movement.
			//
			// Past some point the remote steer rate wins anyway. Maybe ten is
			// more range than anyone needs, but there is no useful reason to
			// make the dvar artificially narrow either.
			//
			// Which is exactly why it is cheat protected. A player who can pick
			// their own number here is picking how fast their predator turns,
			// and that is not a preference, that is an advantage. Cheats off
			// pins it at the default, so everyone still gets the frame-rate
			// correction at the stock feel and nobody gets more than that.
			//
			MRemoteControlSensitivity = Dvar::Register<float>("m_remoteControlSensitivity", 1.0f, 0.0f, 10.0f, Game::DVAR_ARCHIVE | Game::DVAR_SAVED | Game::DVAR_CHEAT,
				"Mouse steering sensitivity for remote control (predator, sentry), relative to the stock feel. 0 restores the game's frame-rate dependent steering");
		});
	}
}
