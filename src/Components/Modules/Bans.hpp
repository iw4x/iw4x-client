namespace Components
{
	class Bans : public Component
	{
	public:
		typedef std::pair<SteamID, Game::netIP_t> Entry;

		Bans();
		~Bans();

#ifdef DEBUG
		const char* GetName() { return "Bans"; };
#endif

		static void BanClientNum(int num, std::string reason);

		static bool IsBanned(Entry entry);
		static void InsertBan(Entry entry);

	private:
		class BanList
		{
		public:
			std::vector<SteamID> IDList;
			std::vector<Game::netIP_t> IPList;
		};

		static std::mutex AccessMutex;

		static void LoadBans(BanList* list);
	};
}
