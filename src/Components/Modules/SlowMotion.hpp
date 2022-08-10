#pragma once

namespace Components
{
	class SlowMotion : public Component
	{
	public:
		SlowMotion();

	private:
		static int Delay;

		static void SetSlowMotion();
		static void ApplySlowMotion(int timePassed);
		static void ApplySlowMotionStub();

		static void DrawConnectionInterruptedStub(int a1);
	};
}
