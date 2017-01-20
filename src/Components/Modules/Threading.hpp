#pragma once

namespace Components
{
	class Threading : public Component
	{
	public:
		Threading();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Threading"; };
#endif

	private:
		static void FrameEpilogueStub();
		static void PacketEventStub();
	};
}
