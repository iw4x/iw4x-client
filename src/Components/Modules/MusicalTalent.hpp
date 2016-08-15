namespace Components
{
	class MusicalTalent : public Component
	{
	public:
		MusicalTalent();
		~MusicalTalent();

#ifdef DEBUG
		const char* GetName() { return "MusicalTalent"; };
#endif

		static void Replace(std::string sound, const char* file);

	private:
		static std::map<std::string, const char*> SoundAliasList;
		static Game::XAssetHeader ModifyAliases(Game::XAssetType type, std::string filename);
	};
}
