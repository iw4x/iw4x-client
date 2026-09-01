#pragma once

namespace Components
{
	class RemoteControl : public Component
	{
	public:
		RemoteControl();

	private:
		static Dvar::Var MRemoteControlSensitivity;

		static float PendingYaw;
		static float PendingPitch;
		static float DeflectionYaw;
		static float DeflectionPitch;
		static int LastTime;

		static float DeflectionLimit(unsigned int dvarAddress);
		static void  Steer(float* value, float limit, float& pending, float& deflection, float referenceSeconds, float step, float frameSeconds);
		static void  ScaleMouseMovement(float* yaw, float* pitch);
		static void  CL_GetMouseMovementWithSensitivity_Stub();
	};
}
