namespace Components
{
	class StringTable : public Component
	{
	public:
		StringTable();
		~StringTable();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "StringTable"; };
#endif

	private:
		static Utils::Memory::Allocator MemAllocator;
		static std::map<std::string, Game::StringTable*> StringTableMap;

		static int Hash(const char* data);
		static Game::StringTable* LoadObject(std::string filename);
	};
}
