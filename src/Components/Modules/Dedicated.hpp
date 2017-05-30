#pragma once

namespace Components
{
	class Dedicated : public Component
	{
	public:
		typedef void(Callback)();

		Dedicated();
		~Dedicated();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Dedicated"; };
#endif

		static SteamID PlayerGuids[18][2];

		static bool IsEnabled();

		static void Heartbeat();

		static void OnFrame(Utils::Slot<Callback> callback);
		static void Once(Utils::Slot<Callback> callback);

	private:
		static Utils::Signal<Callback> FrameSignal;
		static Utils::Signal<Callback> FrameOnceSignal;

		static bool SendChat;

		static void MapRotate();
		static void FrameHandler();
		static void FrameStub();
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();

		static const char* EvaluateSay(char* text);

		static void PreSayStub();
		static void PostSayStub();

		static void TransmitGuids();

		static void TimeWrapStub(int code, const char* message);
	};
}
