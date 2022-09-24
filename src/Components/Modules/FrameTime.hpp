#pragma once

namespace Components
{
	class FrameTime : public Component
	{
	public:
		FrameTime();

	private:
		static void SVFrameWaitStub();
		static void SVFrameWaitFunc();

		static void NetSleep(int mSec);

		static int ComTimeVal(int minMsec);
		static int ComFrameWait(int minMsec);
		static void ComFrameWaitStub();
	};
}
