#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> ModList::Mods;
	unsigned int ModList::CurrentMod;

	bool ModList::HasMod(std::string modName)
	{
		auto list = FileSystem::GetSysFileList(Dvar::Var("fs_basepath").Get<std::string>() + "\\mods", "", true);

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

	const char* ModList::GetItemText(unsigned int index, int column)
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

	void ModList::UIScript_LoadMods()
	{
		auto folder = Dvar::Var("fs_basepath").Get<std::string>() + "\\mods";
		Game::Com_Printf(0, "Searching for mods in %s...\n", folder.data());
		ModList::Mods = FileSystem::GetSysFileList(folder, "", true);
		Game::Com_Printf(0, "Found %i mods!\n", ModList::Mods.size());
	}

	void ModList::UIScript_RunMod()
	{
		if (ModList::CurrentMod < ModList::Mods.size())
		{
			auto fsGame = Dvar::Var("fs_game");
			fsGame.Set(fmt::sprintf("mods/%s", ModList::Mods[ModList::CurrentMod].data()));
			fsGame.Get<Game::dvar_t*>()->pad2[0] = 1;

			if (Dvar::Var("cl_modVidRestart").Get<bool>())
			{
				Command::Execute("vid_restart", false);
			}
			else
			{
				Command::Execute("closemenu mods_menu", false);
			}
		}
	}

	void ModList::UIScript_ClearMods()
	{
		auto fsGame = Dvar::Var("fs_game");
		fsGame.Set("");
		fsGame.Get<Game::dvar_t*>()->pad2[0] = 1;

		if (Dvar::Var("cl_modVidRestart").Get<bool>())
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "vid_restart");
		}
		else
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "closemenu mods_menu");
		}
	}

	ModList::ModList()
	{
		ModList::CurrentMod = 0;
		Dvar::Register("cl_modVidRestart", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Perform a vid_restart when loading a mod.");

		UIScript::Add("LoadMods", ModList::UIScript_LoadMods);
		UIScript::Add("RunMod", ModList::UIScript_RunMod);
		UIScript::Add("ClearMods", ModList::UIScript_ClearMods);

		UIFeeder::Add(9.0f, ModList::GetItemCount, ModList::GetItemText, ModList::Select);
	}

	ModList::~ModList()
	{
		ModList::Mods.clear();
	}
}