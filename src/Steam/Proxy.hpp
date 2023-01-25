#pragma once

#include <STDInclude.hpp>

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
		virtual unknown_ret CreateSteamPipe() = 0;
		virtual unknown_ret BReleaseSteamPipe(int32) = 0;
		virtual unknown_ret CreateGlobalUser(int32 *) = 0;
		virtual unknown_ret ConnectToGlobalUser(int32) = 0;
		virtual unknown_ret CreateLocalUser(int32 *, int) = 0;
		virtual unknown_ret CreatePipeToLocalUser(int32, int32 *) = 0;
		virtual unknown_ret ReleaseUser(int32, int32) = 0;
		virtual unknown_ret IsValidHSteamUserPipe(int32, int32) = 0;
		virtual unknown_ret GetIClientUser(int32, int32) = 0;
		virtual unknown_ret GetIClientGameServer(int32, int32, const char *) = 0;
		virtual unknown_ret SetLocalIPBinding(uint32, uint16) = 0;
		virtual unknown_ret GetUniverseName(int) = 0;

		virtual unknown_ret Placeholder(int) = 0;

		virtual unknown_ret GetIClientFriends(int32, int32) = 0;
		virtual unknown_ret GetIClientUtils(int32) = 0;
		virtual unknown_ret GetIClientBilling(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientMatchmaking(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientApps(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientMatchmakingServers(int32, int32, const char *) = 0;
		virtual unknown_ret RunFrame() = 0;
		virtual unknown_ret GetIPCCallCount() = 0;
		virtual unknown_ret GetIClientUserStats(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientGameServerStats(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientNetworking(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientRemoteStorage(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientScreenshots(int32, int32, const char *) = 0;
		virtual unknown_ret SetWarningMessageHook(void(*)(int32, const char *)) = 0;
		virtual unknown_ret GetIClientGameCoordinator(int32, int32, const char *) = 0;
		virtual unknown_ret SetOverlayNotificationPosition(int) = 0;
		virtual unknown_ret SetOverlayNotificationInset(int32, int32) = 0;
		virtual unknown_ret HookScreenshots(bool) = 0;
		virtual unknown_ret IsOverlayEnabled() = 0;
		virtual unknown_ret GetAPICallResult(int32, uint64, void *, int32, int32, bool *) = 0;
		virtual unknown_ret GetIClientProductBuilder(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientDepotBuilder(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientNetworkDeviceManager(int32, const char *) = 0;
		virtual unknown_ret ConCommandInit(void *) = 0;
		virtual unknown_ret GetIClientAppManager(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientConfigStore(int32, int32, const char *) = 0;
		virtual unknown_ret BOverlayNeedsPresent() = 0;
		virtual unknown_ret GetIClientGameStats(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientHTTP(int32, int32, const char *) = 0;
		virtual unknown_ret BShutdownIfAllPipesClosed() = 0;
		virtual unknown_ret GetIClientAudio(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientMusic(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientUnifiedMessages(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientController(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientParentalSettings(int32, const char *) = 0;
		virtual unknown_ret GetIClientStreamLauncher(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientDeviceAuth(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientRemoteClientManager(int32, const char *) = 0;
		virtual unknown_ret GetIClientStreamClient(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientShortcuts(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientRemoteControlManager(int32) = 0;
		virtual unknown_ret Set_ClientAPI_CPostAPIResultInProcess(void(*)(uint64, void *, uint32, int32)) = 0;
		virtual unknown_ret Remove_ClientAPI_CPostAPIResultInProcess(void(*)(uint64, void *, uint32, int32)) = 0;
		virtual unknown_ret GetIClientUGC(int32, int32, const char *) = 0;
		//virtual unknown_ret GetIClientInventory(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientVR(int32, const char *) = 0;
		virtual unknown_ret GetIClientTabletop(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientHTMLSurface(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientVideo(int32, int32, const char *) = 0;
		virtual unknown_ret GetIClientControllerSerialized(int32, const char *) = 0;
		virtual unknown_ret GetIClientAppDisableUpdate(int32, int32, const char *) = 0;
		virtual unknown_ret Set_Client_API_CCheckCallbackRegisteredInProcess(uint32(*)(int32)) = 0;
		virtual unknown_ret GetIClientBluetoothManager(int32, const char *) = 0;
		//virtual unknown_ret ~CSteamClient() = 0;
		//virtual unknown_ret ~CSteamClient() = 0;
		virtual unknown_ret GetIPCServerMap() = 0;
		virtual unknown_ret OnDebugTextArrived(const char *) = 0;
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
		Interface(void* _interfacePtr) : interfacePtr(static_cast<VInterface*>(_interfacePtr)) {}

		template<typename T, typename... Args>
		T invoke(const std::string& methodName, Args... args)
		{
			if (!this->interfacePtr)
			{
#ifdef _DEBUG
				OutputDebugStringA(::Utils::String::Format("Steam interface pointer is invalid '{}'!\n", methodName));
#endif
				return T();
			}

			auto method = this->getMethod(methodName);
			if (!method.first)
			{
#ifdef _DEBUG
				OutputDebugStringA(::Utils::String::Format("Steam interface method '{}' not found!\n", methodName));
#endif
				return T();
			}

			std::size_t argc = method.second;
			constexpr std::size_t passedArgc = Interface::AddSizes<sizeof(Args)...>::value;
			if (passedArgc != argc)
			{
#ifdef _DEBUG
				OutputDebugStringA(::Utils::String::Format("Steam interface arguments for method '{}' do not match (expected {} bytes, but got {} bytes)!\n", methodName, argc, passedArgc));
#endif
				return T();
			}

			return reinterpret_cast<T(__thiscall*)(void*, Args ...)>(method.first)(this->interfacePtr, args...);
		}

		explicit operator bool() const
		{
			return this->interfacePtr != nullptr;
		}

		std::size_t paramSize(const std::string& methodName)
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

		struct VInterface
		{
			union VMethod
			{
				unsigned char* data;
				unsigned int value;
				FARPROC func;
			}* vftbl;
		};

		VInterface* interfacePtr;
		std::unordered_map<std::string, std::pair<void*, uint16_t>> methodCache;
		std::pair<void*, uint16_t> getMethod(const std::string& method);
		std::pair<void*, uint16_t> lookupMethod(const std::string& method);
		bool getMethodData(VInterface::VMethod method, std::string* name, uint16_t* params);
	};

	class KeyValuesBuilder
	{
	private:
		std::string buffer;

		inline void packBytes(const void* bytes, std::size_t size)
		{
			this->buffer.append(static_cast<const char*>(bytes), size);
		}

		inline void packDataType(uint8_t type)
		{
			this->packBytes(&type, 1);
		}

		inline void packNullTerminated(const char* string)
		{
			this->packBytes(string, strlen(string) + 1);
		}

	public:
		inline void packString(const char* key, const char* value)
		{
			this->packDataType(1);
			this->packNullTerminated(key);
			this->packNullTerminated(value);
		}

		inline void packUint64(const char* key, uint64_t value)
		{
			this->packDataType(7);
			this->packNullTerminated(key);
			this->packBytes(&value, sizeof(value));
		}

		inline void packEnd()
		{
			this->packDataType(8);
		}

		inline std::string getString()
		{
			return this->buffer;
		}
	};


	class Proxy
	{
	public:
		static bool Inititalize();
		static void Uninititalize();

		static void SetGame(uint32_t appId);
		static void RunGame();

		static void SetMod(const std::string& mod);
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

		static Interface Placeholder;

		static uint32_t AppId;

	private:
		typedef bool(SteamBGetCallbackFn)(void* hpipe, void *pCallbackMsg);
		typedef void(SteamFreeLastCallbackFn)(void* hpipe);
		typedef bool(SteamGetAPICallResultFn)(void* hpipe, uint64_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed);

		class CallContainer
		{
		public:
			uint64_t call;
			bool handled;
			int32_t callId;
			uint32_t dataSize;
		};

		class CallbackMsg
		{
		public:
			int32_t m_hSteamUser;
			int m_iCallback;
			uint8_t *m_pubParam;
			int m_cubParam;
		};

		class Handle
		{
		public:
			Handle(void* _handle) : handle(_handle) {}
			Handle(int32_t _handle) : Handle(reinterpret_cast<void*>(_handle)) {}
			Handle() : Handle(nullptr) {}

			operator int32_t() const { return reinterpret_cast<int32_t>(this->handle); }
			operator void*() const { return this->handle; }

		private:
			void* handle;
		};

		static ::Utils::Library Client;
		static ::Utils::Library Overlay;

		static ISteamClient008* SteamClient;
		static IClientEngine*   ClientEngine;
		static Interface        ClientUser;

		static Handle SteamPipe;
		static Handle SteamUser;

		static HANDLE Process;
		static HANDLE CancelHandle;
		static std::thread WatchGuard;

		static std::recursive_mutex CallMutex;
		static std::vector<CallContainer> Calls;
		static std::unordered_map<int32_t, void*> Callbacks;

		static std::function<SteamBGetCallbackFn> SteamBGetCallback;
		static std::function<SteamFreeLastCallbackFn> SteamFreeLastCallback;
		static std::function<SteamGetAPICallResultFn> SteamGetAPICallResult;

		static void RunCallback(int32_t callId, void* data, std::size_t size);

		static void UnregisterCalls();
		static void LaunchWatchGuard();

		static void ResetActiveUser();
		static uint32_t GetActiveUser();
		static std::string GetSteamDirectory();
	};
}
