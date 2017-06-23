#include "STDInclude.hpp"

namespace Components
{
	unsigned int Gametypes::GetGametypeCount()
	{
		return *Game::gameTypeCount;
	}

	const char* Gametypes::GetGametypeText(unsigned int index, int)
	{
		if (static_cast<unsigned int>(*Game::gameTypeCount) > index)
		{
			return Game::SEH_StringEd_GetString(Game::gameTypes[index].uiName);
		}

		return "";
	}

	void Gametypes::SelectGametype(unsigned int index)
	{
		if (!*Game::gameTypeCount) return;
		if (static_cast<unsigned int>(*Game::gameTypeCount) <= index) index = 0;

		std::string gametype = Game::gameTypes[index].gameType;

		Dvar::Var("ui_gametype").set(gametype);
		//Dvar::Var("g_gametype").set(gametype);
	}

	void* Gametypes::BuildGametypeList(const char*, void* buffer, size_t size)
	{
		std::vector<std::string> gametypes;

		auto pushGametype = [&](std::string gametype)
		{
			auto pos = gametype.find_last_of("/\\");
			if (pos != std::string::npos)
			{
				gametype = gametype.substr(pos + 1);
			}

			if (Utils::String::EndsWith(gametype, ".txt"))
			{
				gametype = gametype.substr(0, gametype.size() - 4);
			}

			// No need for that :)
			if (gametype == "_gametypes") return;

			if (std::find(gametypes.begin(), gametypes.end(), gametype) == gametypes.end())
			{
				gametypes.push_back(gametype);
			}
		};

		// Get the gametypes we can find in the filesystem
		std::vector<std::string> rawGametypes = FileSystem::GetFileList("maps/mp/gametypes/", "txt");

		// Get the gametypes we can find in the database
		Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_RAWFILE, [](Game::XAssetHeader header, void* data)
		{
			std::string name = header.rawfile->name;
			std::vector<std::string>* rawGametypes = reinterpret_cast<std::vector<std::string>*>(data);

			if (Utils::String::StartsWith(name, "maps/mp/gametypes/") && Utils::String::EndsWith(name, ".txt"))
			{
				if (std::count(name.begin(), name.end(), '/') == 3 && std::count(name.begin(), name.end(), '\\') == 0)
				{
					rawGametypes->push_back(name);
				}
			}

		}, &rawGametypes, false);

		std::for_each(rawGametypes.begin(), rawGametypes.end(), pushGametype);

		std::string data;
		for (auto& gametype : gametypes)
		{
			if (Game::LoadModdableRawfile(0, Utils::String::VA("maps/mp/gametypes/%s.txt", gametype.data())))
			{
				data.append(gametype);
				data.append("\r\n");
			}
		}

		// Copy to the actual buffer
		std::memcpy(buffer, data.data(), std::min(size, data.size() + 1));

		return (data.empty() ? nullptr : buffer);
	}

	Gametypes::Gametypes()
	{
		UIFeeder::Add(29.0f, Gametypes::GetGametypeCount, Gametypes::GetGametypeText, Gametypes::SelectGametype);

		// Dynamically grab gametypes
		Utils::Hook(0x5FA46C, Gametypes::BuildGametypeList, HOOK_CALL).install()->quick(); // Scr_UpdateGameTypeList
		Utils::Hook(0x632155, Gametypes::BuildGametypeList, HOOK_CALL).install()->quick(); // UI_UpdateGameTypesList

		// This is placed here in case the anticheat has been disabled!
		// Make sure this is called after every onther anticheat check!
#ifndef DISABLE_ANTICHEAT
		Utils::Hook(0x5ACBA3, []() // Somewhere in the renderer, past other renderer hooks!
		{
			AntiCheat::FlagIntegrityCheck();
			return Utils::Hook::Call<void()>(0x50AB20)();
		}, HOOK_CALL).install()->quick();
#endif
	}
}
