#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, const char*> MusicalTalent::SoundAliasList;

	void MusicalTalent::Replace(const std::string& sound, const char* file)
	{
		MusicalTalent::SoundAliasList[Utils::String::ToLower(sound)] = file;
	}

	Game::XAssetHeader MusicalTalent::ModifyAliases(Game::XAssetType type, const std::string& filename)
	{
		Game::XAssetHeader header = { nullptr };

		if (MusicalTalent::SoundAliasList.find(Utils::String::ToLower(filename)) != MusicalTalent::SoundAliasList.end())
		{
			Game::snd_alias_list_t* aliases = Game::DB_FindXAssetHeader(type, filename.data()).sound;

			if (aliases && aliases->count > 0 && aliases->head && aliases->head->soundFile)
			{
				if (aliases->head->soundFile->type == Game::snd_alias_type_t::SAT_STREAMED)
				{
					aliases->head->soundFile->u.streamSnd.filename.info.raw.name = MusicalTalent::SoundAliasList[Utils::String::ToLower(filename)];
				}

				header.sound = aliases;
			}
		}

		return header;
	}

	MusicalTalent::MusicalTalent()
	{
		if (ZoneBuilder::IsEnabled() || Dedicated::IsEnabled()) return;

		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_SOUND, MusicalTalent::ModifyAliases);

		MusicalTalent::Replace("music_mainmenu_mp", "hz_t_menumusic.mp3");
	}
}
