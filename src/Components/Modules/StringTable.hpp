#pragma once

namespace Components
{
	class StringTable : public Component
	{
	public:
		StringTable();
		~StringTable();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "StringTable"; };
#endif

	private:
		static std::unordered_map<std::string, Game::StringTable*> StringTableMap;

		static int Hash(const char* data);
		static Game::StringTable* LoadObject(std::string filename);
	};
}
