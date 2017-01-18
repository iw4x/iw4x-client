namespace Components
{
	class FrameTime : public Component
	{
	public:
		FrameTime();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "FrameTime"; };
#endif

	private:
		static void SVFrameWaitStub();
		static void SVFrameWaitFunc();

		static void NetSleep(int msec);

		static int ComTimeVal(int minMsec);
		static uint32_t ComFrameWait(int minMsec);
		static void ComFrameWaitStub();
	};
}
