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
	class ISteamClient008
	{
	public:
		virtual void* CreateSteamPipe() = 0;
		virtual bool ReleaseSteamPipe(void* hSteamPipe) = 0;
		virtual void* ConnectToGlobalUser(void* hSteamPipe) = 0;
		virtual void* CreateLocalUser(void* *phSteamPipe, int eAccountType) = 0;
		virtual void ReleaseUser(void* hSteamPipe, void* hUser) = 0;
		virtual void *GetISteamUser(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamGameServer(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetLocalIPBinding(uint32_t unIP, uint16_t usPort) = 0;
		virtual void *GetISteamFriends(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamUtils(void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMatchmaking(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMasterServerUpdater(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMatchmakingServers(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamGenericInterface(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamUserStats(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamApps(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamNetworking(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void*GetISteamRemoteStorage(void* hSteamuser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void RunFrame() = 0;
		virtual uint32_t GetIPCCallCount() = 0;
		virtual void SetWarningMessageHook(void* pFunction) = 0;
	};

	class Proxy
	{
	public:
		static bool Inititalize();
		static void Uninititalize();

		//Overlay related proxies
		static void SetOverlayNotificationPosition(uint32_t eNotificationPosition);
		static bool IsOverlayEnabled();
		static bool BOverlayNeedsPresent();

		static Friends* SteamFriends;
		static Utils* SteamUtils;

	private:
		static ::Utils::Library Client;
		static ::Utils::Library Overlay;

		static ISteamClient008* SteamClient;

		static void* SteamPipe;
		static void* SteamUser;

		static std::string GetSteamDirectory();
	};
}
