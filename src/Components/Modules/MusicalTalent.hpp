#pragma once

namespace Components
{
	class MusicalTalent : public Component
	{
	public:
		MusicalTalent();

		static void Replace(const std::string& sound, const char* file);

	private:
		static std::unordered_map<std::string, const char*> SoundAliasList;
		static Game::XAssetHeader ModifyAliases(Game::XAssetType type, const std::string& filename);
	};
}
