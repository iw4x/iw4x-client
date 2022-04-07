#pragma once

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

	private:
		static bool UpdateVersionOffsets(Game::StructuredDataDefSet *set, Game::StructuredDataBuffer *buffer, Game::StructuredDataDef *oldDef);

		static void PatchPlayerDataEnum(Game::StructuredDataDef* data, PlayerDataType type, std::vector<std::string>& entries);
		static void PatchAdditionalData(Game::StructuredDataDef* data, std::unordered_map<std::string, std::string>& patches);

		static void PatchCustomClassLimit(Game::StructuredDataDef* data, int count);
		static Utils::Memory::Allocator MemAllocator;

		static const char* EnumTranslation[ENUM_MAX];
	};
}
