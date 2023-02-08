#pragma once

namespace Components
{
	class Party : public Component
	{
	public:
		Party();

		static Network::Address Target();
		static void Connect(Network::Address target);
		static const char* GetLobbyInfo(SteamID lobby, const std::string& key);
		static void RemoveLobby(SteamID lobby);

		static bool PlaylistAwaiting();
		static void PlaylistContinue();
		static void PlaylistError(const std::string& error);

		static void ConnectError(const std::string& message);

		static bool IsInUserMapLobby();
		static bool IsInLobby();

		static bool IsEnabled();

		static std::string GetMotd();
		static std::string GetHostName();
		static int GetMaxClients();

	private:
		static std::map<std::uint64_t, Network::Address> LobbyMap;

		static Dvar::Var PartyEnable;

		static SteamID GenerateLobbyId();

		static DWORD UIDvarIntStub(char* dvar);
	};
}
