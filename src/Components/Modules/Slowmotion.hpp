#define BUTTON_FLAG_LEANLEFT 0x40
#define BUTTON_FLAG_LEANRIGHT 0x80

namespace Components
{
	class SlowMotion : public Component
	{
	public:
		SlowMotion();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "SlowMotion"; };
#endif

	private:
		static int Delay;

		static void SetSlowMotion();
		static void ApplySlowMotion(int timePassed);
		static void ApplySlowMotionStub();
	};
}
