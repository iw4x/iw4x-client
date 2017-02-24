#pragma once

#ifdef _WIN64
#define GAMEOVERLAY_LIB "gameoverlayrenderer64.dll"
#define STEAMCLIENT_LIB "steamclient64.dll"
#define STEAM_REGISTRY_PATH "Software\\Wow6432Node\\Valve\\Steam"
#else
#define GAMEOVERLAY_LIB "gameoverlayrenderer.dll"
#define STEAMCLIENT_LIB "steamclient.dll"
#define STEAM_REGISTRY_PATH "Software\\Valve\\Steam"
#define STEAM_REGISTRY_PROCESS_PATH "Software\\Valve\\Steam\\ActiveProcess"
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

	class IClientEngine
	{
	public:
		virtual void* CreateSteamPipe() = 0;
		virtual bool BReleaseSteamPipe(void* hSteamPipe) = 0;
		virtual void* CreateGlobalUser(void** phSteamPipe) = 0;
		virtual void* ConnectToGlobalUser(void* hSteamPipe) = 0;
		virtual void* CreateLocalUser(void** phSteamPipe, int eAccountType) = 0;
		virtual void CreatePipeToLocalUser(void* hSteamUser, void** phSteamPipe) = 0;
		virtual void ReleaseUser(void* hSteamPipe, void* hUser) = 0;
		virtual bool IsValidHSteamUserPipe(void* hSteamPipe, void* hUser) = 0;
		virtual void *GetIClientUser(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientGameServer(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetLocalIPBinding(uint32_t unIP, uint16_t usPort) = 0;
		virtual char const *GetUniverseName(int eUniverse) = 0;
		virtual void *GetIClientFriends(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientUtils(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientBilling(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMatchmaking(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientApps(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMatchmakingServers(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void RunFrame() = 0;
		virtual uint32_t GetIPCCallCount() = 0;
		virtual void *GetIClientUserStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientGameServerStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientNetworking(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteStorage(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientScreenshots(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void SetWarningMessageHook(void* pFunction) = 0;
		virtual void *GetIClientGameCoordinator(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetOverlayNotificationPosition(int eNotificationPosition) = 0;
		virtual void SetOverlayNotificationInsert(int32_t, int32_t) = 0;
		virtual bool HookScreenshots(bool bHook) = 0;
		virtual bool IsOverlayEnabled() = 0;
		virtual bool GetAPICallResult(void* hSteamPipe, uint64_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed) = 0;
		virtual void *GetIClientProductBuilder(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientDepotBuilder(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientNetworkDeviceManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void ConCommandInit(void *pAccessor) = 0;
		virtual void *GetIClientAppManager(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientConfigStore(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual bool BOverlayNeedsPresent() = 0;
		virtual void *GetIClientGameStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientHTTP(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual bool BShutdownIfAllPipesClosed() = 0;
		virtual void *GetIClientAudio(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMusic(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientUnifiedMessages(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientController(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientParentalSettings(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientStreamLauncher(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientDeviceAuth(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteClientManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void test() = 0; // Too lazy to correct the struct
		virtual void *GetIClientStreamClient(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientShortcuts(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteControlManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void Set_ClientAPI_CPostAPIResultInProcess(void(*)(uint64_t ulUnk, void * pUnk, uint32_t uUnk, int32_t iUnk)) = 0;
		virtual void Remove_ClientAPI_CPostAPIResultInProcess(void(*)(uint64_t ulUnk, void * pUnk, uint32_t uUnk, int32_t iUnk)) = 0;
		virtual void *GetIClientUGC(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientVR(char const * pchVersion) = 0;
	};

	class Apps7
	{
	public:
		virtual bool BIsSubscribed() = 0;
		virtual bool BIsLowViolence() = 0;
		virtual bool BIsCybercafe() = 0;
		virtual bool BIsVACBanned() = 0;
		virtual const char *GetCurrentGameLanguage() = 0;
		virtual const char *GetAvailableGameLanguages() = 0;
		virtual bool BIsSubscribedApp(int nAppID) = 0;
		virtual bool BIsDlcInstalled(int nAppID) = 0;
		virtual uint32_t GetEarliestPurchaseUnixTime(int nAppID) = 0;
		virtual bool BIsSubscribedFromFreeWeekend() = 0;
		virtual int GetDLCCount() = 0;
		virtual bool BGetDLCDataByIndex(int iDLC, int *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize) = 0;
		virtual void InstallDLC(int nAppID) = 0;
		virtual void UninstallDLC(int nAppID) = 0;
		virtual void RequestAppProofOfPurchaseKey(int nAppID) = 0;
		virtual bool GetCurrentBetaName(char *pchName, int cchNameBufferSize) = 0;
		virtual bool MarkContentCorrupt(bool bMissingFilesOnly) = 0;
		virtual uint32_t GetInstalledDepots(int appID, void *pvecDepots, uint32_t cMaxDepots) = 0;
		virtual uint32_t GetAppInstallDir(int appID, char *pchFolder, uint32_t cchFolderBufferSize) = 0;
		virtual bool BIsAppInstalled(int appID) = 0;
		virtual SteamID GetAppOwner() = 0;
		virtual const char *GetLaunchQueryParam(const char *pchKey) = 0;
		virtual bool GetDlcDownloadProgress(uint32_t, uint64_t *, uint64_t *) = 0;
		virtual int GetAppBuildId() = 0;
	};

	class Interface
	{
	public:
		Interface() : interfacePtr(nullptr) {}
		Interface(void* _interfacePtr) : interfacePtr(_interfacePtr) {}

		template<typename T, typename... Args>
		T invoke(std::string methodName, Args... args)
		{
			if(!this->interfacePtr)
			{
				OutputDebugStringA(::Utils::String::VA("Steam interface pointer is invalid (%s)!\n", methodName.data()));
				return T();
			}

			auto method = this->getMethod(methodName);
			if (!method.first)
			{
				OutputDebugStringA(::Utils::String::VA("Steam interface method %s not found!\n", methodName.data()));
				return T();
			}

			size_t argc = method.second;
			constexpr size_t passedArgc = Interface::AddSizes<sizeof(Args)...>::value;
			if(passedArgc != argc)
			{
				OutputDebugStringA(::Utils::String::VA("Steam interface arguments for method %s do not match (expected %d bytes, but got %d bytes)!\n", methodName.data(), argc, passedArgc));
				return T();
			}

			return reinterpret_cast<T(__thiscall*)(void*, Args ...)>(method.first)(this->interfacePtr, args...);
		}

		explicit operator bool() const
		{
			return this->interfacePtr != nullptr;
		}

		size_t paramSize(std::string methodName)
		{
			auto method = this->getMethod(methodName);
			return method.second;
		}

	private:
		// TODO: Use fold expressions once available (C++17)
		template<std::size_t ...>
		struct AddSizes : std::integral_constant<std::size_t, 0> {};

		// This recursively adds the sizes of the individual arguments while respecting the architecture of the CPU
		template<std::size_t X, std::size_t ... Xs>
		struct AddSizes<X, Xs...> : std::integral_constant<std::size_t, X + ((AddSizes<Xs...>::value + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1))> {};

		void* interfacePtr;
		std::unordered_map<std::string, std::pair<void*, uint16_t>> methodCache;
		std::pair<void*, uint16_t> getMethod(std::string method);
		std::pair<void*, uint16_t> lookupMethod(std::string method);
		bool getMethodData(unsigned char* methodPtr, std::string* name, uint16_t* params);
	};

	class KeyValuesBuilder
	{
	private:
		std::stringstream m_buffer;

		inline void packBytes(const void* bytes, size_t size)
		{
			m_buffer << std::string(reinterpret_cast<const char*>(bytes), size);
		}

		inline void packDataType(uint8_t type)
		{
			packBytes(&type, 1);
		}

		inline void packNullTerminated(const char* string)
		{
			packBytes(string, strlen(string) + 1);
		}

	public:
		inline void packString(const char* key, const char* value)
		{
			packDataType(1);
			packNullTerminated(key);
			packNullTerminated(value);
		}

		inline void packUint64(const char* key, uint64_t value)
		{
			packDataType(7);
			packNullTerminated(key);
			packBytes(&value, sizeof(value));
		}

		inline void packEnd()
		{
			packDataType(8);
		}

		inline std::string getString()
		{
			return m_buffer.str();
		}
	};


	class Proxy
	{
	public:
		static bool Inititalize();
		static void Uninititalize();

		static void SetGame(uint32_t appId);
		static void RunGame();

		static void SetMod(std::string mod);
		static void RunMod();

		//Overlay related proxies
		static void SetOverlayNotificationPosition(uint32_t eNotificationPosition);
		static bool IsOverlayEnabled();
		static bool BOverlayNeedsPresent();

		static void RunFrame();

		static void RegisterCall(int32_t callId, uint32_t size, uint64_t call);

		static void RegisterCallback(int32_t callId, void* callback);
		static inline void RegisterCallback(int32_t callId, void(*callback)(void*)) { RegisterCallback(callId, static_cast<void*>(callback)); }
		static void UnregisterCallback(int32_t callId);

		static Friends15* SteamFriends;
		static Apps7* SteamApps;
		static Utils* SteamUtils;
		static User* SteamUser_;
		static Interface ClientFriends;

		static uint32_t AppId;

	private:
		typedef bool(SteamBGetCallbackFn)(void* hpipe, void *pCallbackMsg);
		typedef void(SteamFreeLastCallbackFn)(void* hpipe);
		typedef bool(SteamGetAPICallResultFn)(void* hpipe, uint64_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed);

		struct CallContainer
		{
			uint64_t call;
			bool handled;
			int32_t callId;
			uint32_t dataSize;
		};

		struct CallbackMsg
		{
			int32_t m_hSteamUser;
			int m_iCallback;
			uint8_t *m_pubParam;
			int m_cubParam;
		};

		static ::Utils::Library Client;
		static ::Utils::Library Overlay;

		static ISteamClient008* SteamClient;
		static IClientEngine*   ClientEngine;
		static Interface        ClientUser;

		static void* SteamPipe;
		static void* SteamUser;

		static HANDLE Process;
		static HANDLE CancelHandle;
		static std::thread WatchGuard;

		static std::recursive_mutex CallMutex;
		static std::vector<CallContainer> Calls;
		static std::unordered_map<int32_t, void*> Callbacks;

		static std::function<SteamBGetCallbackFn> SteamBGetCallback;
		static std::function<SteamFreeLastCallbackFn> SteamFreeLastCallback;
		static std::function<SteamGetAPICallResultFn> SteamGetAPICallResult;

		static void RunCallback(int32_t callId, void* data, size_t size);

		static void UnregisterCalls();
		static void StartSteamIfNecessary();
		static void LaunchWatchGuard();

		static void ResetActiveUser();
		static uint32_t GetActiveUser();
		static std::string GetSteamDirectory();
	};
}
