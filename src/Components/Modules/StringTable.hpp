#pragma once

namespace Components
{
	class StringTable
	{
	public:
		StringTable();

	private:
		static std::unordered_map<std::string, Game::StringTable*> StringTableMap;

		static Game::StringTable* LoadObject(std::string filename);
	};
}
