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
	STEAM_EXPORT bool SteamAPI_Init();
	STEAM_EXPORT void SteamAPI_RegisterCallResult();
	STEAM_EXPORT void SteamAPI_RegisterCallback();
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
