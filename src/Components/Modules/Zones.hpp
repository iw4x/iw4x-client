namespace Components
{
	class Zones : public Component
	{
	public:
		Zones();
		~Zones();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Zones"; };
#endif

		static void InstallPatches(int version);

	//private:
		static int ZoneVersion;
	};
}
