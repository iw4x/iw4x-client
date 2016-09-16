#ifdef _WIN64
#define GAMEOVERLAY_LIB "gameoverlayrenderer64.dll"
#define STEAMCLIENT_LIB "steamclient64.dll"
#define STEAM_REGISTRY_PATH "Software\\Wow6432Node\\Valve\\Steam"
#else
#define GAMEOVERLAY_LIB "gameoverlayrenderer.dll"
#define STEAMCLIENT_LIB "steamclient.dll"
#define STEAM_REGISTRY_PATH "Software\\Valve\\Steam"
#endif

namespace Steam
{
	class Proxy
	{
	public:
		static bool Inititalize();
		static void Uninititalize();

		//Overlay related proxies
		static void SetOverlayNotificationPosition(uint32_t eNotificationPosition);
		static bool IsOverlayEnabled();
		static bool BOverlayNeedsPresent();

	private:
		static ::Utils::Library Client;
		static ::Utils::Library Overlay;

		static std::string GetSteamDirectory();
	};
}
