#pragma once

namespace Components
{
	class Threading
	{
	public:
		Threading();

	private:
		static void FrameEpilogueStub();
		static void PacketEventStub();
	};
}
