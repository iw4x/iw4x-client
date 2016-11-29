#include "STDInclude.hpp"

namespace Components
{
	std::map<std::string, const char*> MusicalTalent::SoundAliasList;

	void MusicalTalent::Replace(std::string sound, const char* file)
	{
		MusicalTalent::SoundAliasList[Utils::String::ToLower(sound)] = file;
	}

	Game::XAssetHeader MusicalTalent::ModifyAliases(Game::XAssetType type, std::string filename)
	{
		Game::XAssetHeader header = { 0 };

		if (MusicalTalent::SoundAliasList.find(Utils::String::ToLower(filename)) != MusicalTalent::SoundAliasList.end())
		{
			Game::snd_alias_list_t* aliases = Game::DB_FindXAssetHeader(type, filename.data()).aliasList;

			if (aliases)
			{
				if (aliases->head->soundFile->type == 2)
				{
					aliases->head->soundFile->data.stream.name = MusicalTalent::SoundAliasList[Utils::String::ToLower(filename)];
				}

				header.aliasList = aliases;
			}
		}

		return header;
	}

	MusicalTalent::MusicalTalent()
	{
		if (ZoneBuilder::IsEnabled()) return;

		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_SOUND, MusicalTalent::ModifyAliases);

		MusicalTalent::Replace("music_mainmenu_mp", "hz_t_menumusic.mp3");
	}

	MusicalTalent::~MusicalTalent()
	{
		MusicalTalent::SoundAliasList.clear();
	}
}
