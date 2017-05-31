#pragma once

namespace Components
{
	class Threading : public Component
	{
	public:
		Threading();

	private:
		static void FrameEpilogueStub();
		static void PacketEventStub();
	};
}
