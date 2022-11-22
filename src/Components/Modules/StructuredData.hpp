#pragma once

namespace Components
{
	class StructuredData : public Component
	{
	public:
		enum PlayerDataType
		{
			FEATURES,
			WEAPONS,
			ATTACHEMENTS,
			CHALLENGES,
			CAMOS,
			PERKS,
			KILLSTREAKS,
			ACCOLADES,
			CARDICONS,
			CARDTITLES,
			CARDNAMEPLATES,
			TEAMS,
			GAMETYPES,

			COUNT
		};

		StructuredData();

	private:
		static bool UpdateVersionOffsets(Game::StructuredDataDefSet *set, Game::StructuredDataBuffer *buffer, Game::StructuredDataDef *oldDef);

		static void PatchPlayerDataEnum(Game::StructuredDataDef* data, PlayerDataType type, std::vector<std::string>& entries);
		static void PatchAdditionalData(Game::StructuredDataDef* data, std::unordered_map<std::string, std::string>& patches);

		static void PatchCustomClassLimit(Game::StructuredDataDef* data, int count);
		static Utils::Memory::Allocator MemAllocator;

		static const char* EnumTranslation[COUNT];
	};
}
