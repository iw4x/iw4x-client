namespace Components
{
	class Lean : public Component
	{
	public:
		Lean();

#ifdef DEBUG
		const char* GetName() { return "Lean"; };
#endif

	private:
		static Game::kbutton_t in_leanleft;
		static Game::kbutton_t in_leanright;

		static void IN_LeanLeft_Up();
		static void IN_LeanLeft_Down();

		static void IN_LeanRight_Up();
		static void IN_LeanRight_Down();
	};
}
