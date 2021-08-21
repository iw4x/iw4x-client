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

		static void StripMaterialTextIcons(char* text);

	private:
		static bool SendChat;

		static void MapRotate();
		static void InitDedicatedServer();

		static void PostInitialization();
		static void PostInitializationStub();

		static const char* EvaluateSay(char* text, Game::gentity_t* player);

		static void PreSayStub();
		static void PostSayStub();

		static void FrameStub();

		static void TransmitGuids();

		static void TimeWrapStub(int code, const char* message);
	};
}
