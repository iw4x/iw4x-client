#include <STDInclude.hpp>
#include "ModList.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	std::vector<std::string> ModList::Mods;
	unsigned int ModList::CurrentMod;

	bool ModList::HasMod(const std::string& modName)
	{
		auto list = FileSystem::GetSysFileList(Dvar::Var("fs_basepath").get<std::string>() + "\\mods", "", true);

		for (auto mod : list)
		{
			if (mod == modName)
			{
				return true;
			}
		}

		return false;
	}

	unsigned int ModList::GetItemCount()
	{
		return ModList::Mods.size();
	}

	const char* ModList::GetItemText(unsigned int index, int /*column*/)
	{
		if (index < ModList::Mods.size())
		{
			return ModList::Mods[index].data();
		}

		return "...";
	}

	void ModList::Select(unsigned int index)
	{
		ModList::CurrentMod = index;
	}

	void ModList::UIScript_LoadMods([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		auto folder = (*Game::fs_basepath)->current.string + "\\mods"s;
		Logger::Debug("Searching for mods in {}...", folder);
		ModList::Mods = FileSystem::GetSysFileList(folder, "", true);
		Logger::Debug("Found {} mods!", ModList::Mods.size());
	}

	void ModList::UIScript_RunMod([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		if (ModList::CurrentMod < ModList::Mods.size())
		{
			ModList::RunMod(ModList::Mods[ModList::CurrentMod]);
		}
	}

	void ModList::UIScript_ClearMods([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		Game::Dvar_SetString(*Game::fs_gameDirVar, "");
		const_cast<Game::dvar_t*>((*Game::fs_gameDirVar))->modified = true;

		if (Dvar::Var("cl_modVidRestart").get<bool>())
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "vid_restart");
		}
		else
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "closemenu mods_menu");
		}
	}

	void ModList::RunMod(const std::string& mod)
	{
		Game::Dvar_SetString(*Game::fs_gameDirVar, Utils::String::Format("mods/{}", mod));
		const_cast<Game::dvar_t*>((*Game::fs_gameDirVar))->modified = true;

		if (Dvar::Var("cl_modVidRestart").get<bool>())
		{
			Command::Execute("vid_restart", false);
		}
		else
		{
			Command::Execute("closemenu mods_menu", false);
		}
	}

	ModList::ModList()
	{
		if (Dedicated::IsEnabled()) return;

		ModList::CurrentMod = 0;
		Dvar::Register("cl_modVidRestart", true, Game::DVAR_ARCHIVE, "Perform a vid_restart when loading a mod.");

		UIScript::Add("LoadMods", ModList::UIScript_LoadMods);
		UIScript::Add("RunMod", ModList::UIScript_RunMod);
		UIScript::Add("ClearMods", ModList::UIScript_ClearMods);

		UIFeeder::Add(9.0f, ModList::GetItemCount, ModList::GetItemText, ModList::Select);
	}
}
