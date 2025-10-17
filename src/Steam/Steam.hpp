#pragma once

#define STEAM_EXPORT extern "C" __declspec(dllexport)

#define STEAM_IGNORE_WARNINGS_START __pragma(warning(push)) \
                                    __pragma(warning(disable: 4100))

#define STEAM_IGNORE_WARNINGS_END   __pragma(warning(pop))

typedef union
{
	struct
	{
		unsigned int accountID : 32;
		unsigned int accountInstance : 20;
		unsigned int accountType : 4;
		int          universe : 8;
	};

	unsigned long long bits;
} SteamID;


#pragma pack( push, 1 )
typedef union
{
	struct
	{
		unsigned int appID : 24;
		unsigned int type : 8;
		unsigned int modID : 32;
	};

	unsigned long long bits;
} GameID_t;
#pragma pack( pop )

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef void* unknown_ret;

#include "Interfaces/SteamUser.hpp"
#include "Interfaces/SteamUtils.hpp"
#include "Interfaces/SteamFriends.hpp"
#include "Interfaces/SteamGameServer.hpp"
#include "Interfaces/SteamNetworking.hpp"
#include "Interfaces/SteamMatchmaking.hpp"
#include "Interfaces/SteamRemoteStorage.hpp"
#include "Interfaces/SteamMasterServerUpdater.hpp"

#include "Proxy.hpp"

namespace Steam
{
	class Callbacks
	{
	public:
		class Base
		{
		public:
			Base() : Flags(0), Callback(0) {};

			virtual void Run(void *pvParam) = 0;
			virtual void Run(void *pvParam, bool bIOFailure, uint64_t hSteamAPICall) = 0;
			virtual int GetCallbackSizeBytes() = 0;

			int GetICallback() { return Callback; }
			void SetICallback(int iCallback) { Callback = iCallback; }

		protected:
			~Base() = default;

			unsigned char Flags;
			int Callback;
		};

		struct Result
		{
			void* data;
			int size;
			int type;
			uint64_t call;
		};

		static uint64_t RegisterCall();
		static void RegisterCallback(Base* handler, int callback);
		static void RegisterCallResult(uint64_t call, Base* result);
		static void ReturnCall(void* data, int size, int type, uint64_t call);
		static void RunCallbacks();

		static void RunCallback(int32_t callback, void* data);

		static void Uninitialize();

	private:
		static uint64_t CallID;
		static std::map<uint64_t, bool> Calls;
		static std::map<uint64_t, Base*> ResultHandlers;
		static std::vector<Result> Results;
		static std::vector<Base*> CallbackList;
		static std::recursive_mutex Mutex;
	};

	bool Enabled();

	STEAM_EXPORT bool SteamAPI_Init();
	STEAM_EXPORT void SteamAPI_RegisterCallResult(Callbacks::Base* result, uint64_t call);
	STEAM_EXPORT void SteamAPI_RegisterCallback(Callbacks::Base* handler, int callback);
	STEAM_EXPORT void SteamAPI_RunCallbacks();
	STEAM_EXPORT void SteamAPI_Shutdown();
	STEAM_EXPORT void SteamAPI_UnregisterCallResult();
	STEAM_EXPORT void SteamAPI_UnregisterCallback();

	STEAM_EXPORT bool SteamGameServer_Init();
	STEAM_EXPORT void SteamGameServer_RunCallbacks();
	STEAM_EXPORT void SteamGameServer_Shutdown();

	STEAM_EXPORT Steam::Friends* SteamFriends();
	STEAM_EXPORT Steam::Matchmaking* SteamMatchmaking();
	STEAM_EXPORT Steam::GameServer* SteamGameServer();
	STEAM_EXPORT Steam::MasterServerUpdater* SteamMasterServerUpdater();
	STEAM_EXPORT Steam::Networking* SteamNetworking();
	STEAM_EXPORT Steam::RemoteStorage* SteamRemoteStorage();
	STEAM_EXPORT Steam::User* SteamUser();
	STEAM_EXPORT Steam::Utils* SteamUtils();
}
