namespace Components
{
	class QuickPatch : public Component
	{
	public:
		QuickPatch();
		const char* GetName() { return "QuickPatch"; };

	private:
		static _int64* GetStatsID();
	};
}
