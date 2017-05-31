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

		static bool IsBanned(Entry entry);
		static void InsertBan(Entry entry);

	private:
		class BanList
		{
		public:
			std::vector<SteamID> idList;
			std::vector<Game::netIP_t> ipList;
		};

		static std::mutex AccessMutex;
		static void LoadBans(BanList* list);
	};
}
