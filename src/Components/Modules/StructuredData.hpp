namespace Components
{
	class StructuredData : public Component
	{
	public:
		enum PlayerDataType
		{
			ENUM_FEATURES,
			ENUM_WEAPONS,
			ENUM_ATTACHEMENTS,
			ENUM_CHALLENGES,
			ENUM_CAMOS,
			ENUM_PERKS,
			ENUM_KILLSTREAKS,
			ENUM_ACCOLADES,
			ENUM_CARDICONS,
			ENUM_CARDTITLES,
			ENUM_CARDNAMEPLATES,
			ENUM_TEAMS,
			ENUM_GAMETYPES,
			ENUM_MAX
		};

		StructuredData();
		~StructuredData();
		const char* GetName() { return "StructuredData"; };

		void AddPlayerDataEntry(PlayerDataType type, std::string name);

	private:
		static void PatchPlayerDataEnum(Game::StructuredDataDef* data, PlayerDataType type, std::vector<std::string>& entries);
		static StructuredData* GetSingleton();

		Utils::Memory::Allocator MemAllocator;

		static int IndexCount[ENUM_MAX];
		static Game::StructuredDataEnumEntry* Indices[ENUM_MAX];
		static std::vector<std::string> Entries[ENUM_MAX];

		static StructuredData* Singleton;
	};
}
