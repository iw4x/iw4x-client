#pragma once

namespace Components
{
	class Bans : public Component
	{
	public:
		typedef std::pair<SteamID, Game::netIP_t> Entry;

		Bans();
		~Bans();

		static void BanClientNum(int num, std::string reason);
		static void UnbanClient(SteamID id);
		static void UnbanClient(Game::netIP_t ip);

		static bool IsBanned(Entry entry);
		static void InsertBan(Entry entry);

	private:
		class BanList
		{
		public:
			std::vector<SteamID> idList;
			std::vector<Game::netIP_t> ipList;
		};

		static std::recursive_mutex AccessMutex;
		static void LoadBans(BanList* list);
		static void SaveBans(BanList* list);
	};
}
