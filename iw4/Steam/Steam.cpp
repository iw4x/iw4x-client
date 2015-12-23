#include "..\STDInclude.hpp"
#include "Steam.hpp"

namespace Steam
{
	extern "C"
	{
		bool SteamAPI_Init()
		{
			return true;
		}

		void SteamAPI_RegisterCallResult()
		{
		}

		void SteamAPI_RegisterCallback()
		{
		}

		void SteamAPI_RunCallbacks()
		{
		}

		void SteamAPI_Shutdown()
		{
		}

		void SteamAPI_UnregisterCallResult()
		{
		}

		void SteamAPI_UnregisterCallback()
		{
		}


		bool SteamGameServer_Init()
		{
			return true;
		}

		void SteamGameServer_RunCallbacks()
		{
		}

		void SteamGameServer_Shutdown()
		{
		}


		Steam::Friends* SteamFriends()
		{
			static Steam::Friends iFriends;
			return &iFriends;
		}

		Steam::Matchmaking* SteamMatchmaking()
		{
			static Steam::Matchmaking iMatchmaking;
			return &iMatchmaking;
		}

		Steam::GameServer* SteamGameServer()
		{
			static Steam::GameServer iGameServer;
			return &iGameServer;
		}

		Steam::MasterServerUpdater* SteamMasterServerUpdater()
		{
			static Steam::MasterServerUpdater iMasterServerUpdater;
			return &iMasterServerUpdater;
		}

		Steam::Networking* SteamNetworking()
		{
			static Steam::Networking iNetworking;
			return &iNetworking;
		}

		Steam::RemoteStorage* SteamRemoteStorage()
		{
			static Steam::RemoteStorage iRemoteStorage;
			return &iRemoteStorage;
		}

		Steam::User* SteamUser()
		{
			static Steam::User iUser;
			return &iUser;
		}

		Steam::Utils* SteamUtils()
		{
			static Steam::Utils iUtils;
			return &iUtils;
		}
	}
}
