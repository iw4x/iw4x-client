namespace Components
{
	class MusicalTalent : public Component
	{
	public:
		MusicalTalent();
		~MusicalTalent();
		const char* GetName() { return "MusicalTalent"; };

		static void Replace(std::string sound, const char* file);

	private:
		static std::map<std::string, const char*> SoundAliasList;
		static Game::XAssetHeader ManipulateAliases(Game::XAssetType type, const char* filename);
	};
}
