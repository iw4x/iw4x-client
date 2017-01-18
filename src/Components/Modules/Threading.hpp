namespace Components
{
	class Threading : public Component
	{
	public:
		Threading();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Threading"; };
#endif

	private:
		static void FrameEpilogueStub();
		static void PacketEventStub();
	};
}
