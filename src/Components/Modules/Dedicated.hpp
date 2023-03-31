#pragma once

namespace Components
{
	class Dedicated : public Component
	{
	public:
		Dedicated();

		static SteamID PlayerGuids[18][2];
		static Dvar::Var SVLanOnly;
		static Dvar::Var SVMOTD;
		static Dvar::Var COMLogFilter;

		static const Game::dvar_t* com_dedicated;

		static bool IsEnabled();
		static bool IsRunning();

		static void Heartbeat();

	private:
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();

		static void Com_ClampMsec(int msec);
		static void Com_ClampMsec_Stub();

		static void TransmitGuids();

		static void TimeWrapStub(Game::errorParm_t code, const char* message);
	};
}
