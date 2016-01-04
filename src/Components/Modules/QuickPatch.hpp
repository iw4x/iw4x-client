namespace Components
{
	class QuickPatch : public Component
	{
	public:
		QuickPatch();
		const char* GetName() { return "QuickPatch"; };

		static void UnlockStats();

	private:
		static _int64* GetStatsID();
	};
}
