#pragma once

#define STEAM_EXPORT extern "C" __declspec(dllexport)

typedef union
{
	struct SteamIDComponent_t
	{
		unsigned int 		m_unAccountID : 32;
		unsigned int		m_unAccountInstance : 20;
		unsigned int		m_EAccountType : 4;
		int					m_EUniverse : 8;
	} m_comp;

	unsigned long long m_Bits;
} SteamID;

#include "Interfaces\SteamUser.hpp"
#include "Interfaces\SteamUtils.hpp"
#include "Interfaces\SteamFriends.hpp"
#include "Interfaces\SteamGameServer.hpp"
#include "Interfaces\SteamNetworking.hpp"
#include "Interfaces\SteamMatchmaking.hpp"
#include "Interfaces\SteamRemoteStorage.hpp"
#include "Interfaces\SteamMasterServerUpdater.hpp"

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

	private:
		static uint64_t CallID;
		static std::map<uint64_t, bool> Calls;
		static std::map<uint64_t, Base*> ResultHandlers;
		static std::vector<Result> Results;
		static std::vector<Base*> CallbackList;
	};

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
