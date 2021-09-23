#pragma once

namespace Components
{
	class Dedicated : public Component
	{
	public:
		Dedicated();
		~Dedicated();

		static SteamID PlayerGuids[18][2];

		static bool IsEnabled();

		static void Heartbeat();

	private:
		static void MapRotate();
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();

		static void FrameStub();

		static void TransmitGuids();

		static void TimeWrapStub(int code, const char* message);
	};
}
