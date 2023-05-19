#pragma once

namespace Components
{
	class Lean : public Component
	{
	public:
		Lean();

		static Dvar::Var BGLean;

	private:
		static Game::kbutton_t in_leanleft;
		static Game::kbutton_t in_leanright;

		static void IN_LeanLeft_Up();
		static void IN_LeanLeft_Down();

		static void IN_LeanRight_Up();
		static void IN_LeanRight_Down();

		static void ApplyLeanFlags(Game::usercmd_s* cmd);

		static void PM_UpdateLean_Stub(Game::playerState_s* ps, float msec, Game::usercmd_s* cmd, void(*capsuleTrace)(Game::trace_t*, const float*, const float*, const Game::Bounds*, int, int));
	};
}
