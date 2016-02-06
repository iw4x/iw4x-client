namespace Components
{
	class StringTable : public Component
	{
	public:
		StringTable();
		~StringTable();
		const char* GetName() { return "StringTable"; };

	private:
		static std::map<std::string, Game::StringTable*> StringTableMap;

		static int Hash(const char* data);
		static Game::StringTable* LoadObject(std::string filename);
	};
}
