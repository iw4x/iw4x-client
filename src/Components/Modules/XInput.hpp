#pragma once

namespace Components
{
	class XInput : public Component
	{
	public:
		XInput();

	private:
		static XINPUT_STATE xiStates[XUSER_MAX_COUNT];
		static int xiPlayerNum;

		static void CL_FrameStub();
		static void PollXInputDevices();

		static void CL_CreateCmdStub();
		static void CL_GamepadMove(int, Game::usercmd_s*);

		static void MSG_WriteDeltaUsercmdKeyStub();

		static void MSG_ReadDeltaUsercmdKeyStub();
		static void MSG_ReadDeltaUsercmdKeyStub2();
	};
}
