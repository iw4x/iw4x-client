#include "..\STDInclude.hpp"

namespace Components
{
	std::map<std::string, const char*> MusicalTalent::SoundAliasList;

	void MusicalTalent::Replace(std::string sound, const char* file)
	{
		MusicalTalent::SoundAliasList[Utils::StrToLower(sound)] = file;
	}

	Game::XAssetHeader MusicalTalent::ManipulateAliases(Game::XAssetType type, const char* filename)
	{
		if (MusicalTalent::SoundAliasList.find(Utils::StrToLower(filename)) != MusicalTalent::SoundAliasList.end())
		{
			Game::snd_alias_list_t* aliases = (Game::snd_alias_list_t*)Game::DB_FindXAssetHeader(type, filename);

			if (aliases)
			{
				if (aliases->aliases->stream->type == 2)
				{
					aliases->aliases->stream->file = MusicalTalent::SoundAliasList[Utils::StrToLower(filename)];
				}

				return aliases;
			}
		}

		return NULL;
	}

	MusicalTalent::MusicalTalent()
	{
		AssetHandler::On(Game::XAssetType::ASSET_TYPE_SOUND, MusicalTalent::ManipulateAliases);

		MusicalTalent::Replace("music_mainmenu_mp", "hz_boneyard_intro_LR_1.mp3");
	}

	MusicalTalent::~MusicalTalent()
	{
		MusicalTalent::SoundAliasList.clear();
	}
}
