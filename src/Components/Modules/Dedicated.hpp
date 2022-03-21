#pragma once

namespace Components
{
	class Dedicated : public Component
	{
	public:
		Dedicated();

		static SteamID PlayerGuids[18][2];

		static bool IsEnabled();

		static void Heartbeat();

	private:
		static Dvar::Var SVRandomMapRotation;

		static void RandomizeMapRotation();
		static void MapRotate();
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();

		static void FrameStub();

		static void TransmitGuids();

		static void TimeWrapStub(Game::errorParm_t code, const char* message);

		static Game::dvar_t* Dvar_RegisterSVNetworkFps(const char* dvarName, int value, int min, int max, int flags, const char* description);
	};
}
