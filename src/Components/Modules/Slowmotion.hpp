#pragma once

#define BUTTON_FLAG_LEANLEFT 0x40
#define BUTTON_FLAG_LEANRIGHT 0x80

namespace Components
{
	class SlowMotion : public Component
	{
	public:
		SlowMotion();

	private:
		static int Delay;

		static void SetSlowMotion();
		static void ApplySlowMotionStub(int timePassed);

		static void DrawConnectionInterruptedStub(int a1);
	};
}