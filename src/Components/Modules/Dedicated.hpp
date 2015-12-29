namespace Components
{
	class Dedicated : public Component
	{
	public:
		Dedicated();
		const char* GetName() { return "Dedicated"; };

		static bool IsDedicated();

	private:
		static Dvar::Var Dedi;

		static void InitDedicatedServer();
	};
}
