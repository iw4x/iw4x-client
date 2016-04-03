#include "STDInclude.hpp"

namespace Components
{
	static Dvar::Var cl_modVidRestart;

	ModList::modInfo_t ModList::modInfo;

	bool ModList::hasMod(const char* modName)
	{
		int count;
		auto fs_homepath = Dvar::Var("fs_basepath").Get<const char*>();
		char** mods = Game::Sys_ListFiles((char*)Utils::VA("%s\\%s", fs_homepath, "mods"), NULL, NULL, &count, 1);

		for (int i = 0; i < count; i++)
		{
			if (!_stricmp(modName, mods[i]))
			{
				Game::Sys_FreeFileList(mods);
				return true;
			}
		}

		Game::Sys_FreeFileList(mods);
		return false;
	}

	unsigned int ModList::GetItemCount()
	{
		return ModList::modInfo.max;
	}

	const char* ModList::GetItemText(unsigned int index, int column)
	{
		if (ModList::modInfo.current >= 0 && ModList::modInfo.current < ModList::modInfo.max)
		{
			return ModList::modInfo.mods[index];
		}

		return "...";
	}

	void ModList::Select(unsigned int index)
	{
		ModList::modInfo.current = index;
	}

	void ModList::UIScript_LoadMods()
	{
		if (ModList::modInfo.mods != NULL && *ModList::modInfo.mods != NULL)
		{
			Game::Sys_FreeFileList(ModList::modInfo.mods);
		}

		auto fs_homepath = Dvar::Var("fs_basepath").Get<const char*>();
		auto searchFolder = (char*)Utils::VA("%s\\%s", fs_homepath, "mods");
		Game::Com_Printf(0, "Searching for mods in %s...\n", searchFolder);

		ModList::modInfo.mods = Game::Sys_ListFiles(searchFolder, NULL, NULL, &ModList::modInfo.max, 1);

		Game::Com_Printf(0, "Found %i mods!\n", ModList::modInfo.max);
	}

	void ModList::UIScript_RunMod()
	{
		if (ModList::modInfo.mods != NULL
			&& *ModList::modInfo.mods != NULL
			&& ModList::modInfo.current >= 0
			&& ModList::modInfo.current < ModList::modInfo.max)
		{
			Dvar::Var("fs_game").Set(Utils::VA("mods/%s", ModList::modInfo.mods[ModList::modInfo.current]));
			//Game::Cmd_ExecuteSingleCommand(0, 0, Utils::VA("fs_game \"mods/%s\"", modInfo.mods[modInfo.current]));
			if (cl_modVidRestart.Get<bool>())
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "vid_restart");
			}
			else
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "closemenu mods_menu");
			}
		}
	}

	void ModList::UIScript_ClearMods()
	{
		Dvar::Var("fs_game").Set("");
		//Game::Cmd_ExecuteSingleCommand(0, 0, "fs_game \"\"");
		if (cl_modVidRestart.Get<bool>())
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
		Dvar::OnInit([]()
		{
			cl_modVidRestart = Dvar::Register("cl_modVidRestart", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Perform a vid_restart when loading a mod.");
		});

		ModList::modInfo.max = ModList::modInfo.current = 0;

		UIScript::Add("LoadMods", ModList::UIScript_LoadMods);
		UIScript::Add("RunMod", ModList::UIScript_RunMod);
		UIScript::Add("ClearMods", ModList::UIScript_ClearMods);

		UIFeeder::Add(9.0f, ModList::GetItemCount, ModList::GetItemText, ModList::Select);
	}

	ModList::~ModList()
	{
	}
}