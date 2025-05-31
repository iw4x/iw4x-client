
#include "Events.hpp"
#include "ModList.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	static std::unordered_map<unsigned int, CHAR*> customClassesPrefixedNames{};

	std::vector<std::string> ModList::Mods;
	unsigned int ModList::CurrentMod;

	Dvar::Var ModList::cl_modVidRestart;

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

	void ModList::ClearMods()
	{
		// Clear mod sequence (make sure fs_game is actually set)
		if (*Game::fs_gameDirVar == nullptr || *(*Game::fs_gameDirVar)->current.string == '\0')
		{
			return;
		}

		Game::Dvar_SetString(*Game::fs_gameDirVar, "");

		if (cl_modVidRestart.get<bool>())
		{
			Command::Execute("vid_restart", false);
		}
		else
		{
			Command::Execute("closemenu mods_menu", false);
		}
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
		ClearMods();
	}

	void ModList::RunMod(const std::string& mod)
	{
		Game::Dvar_SetString(*Game::fs_gameDirVar, Utils::String::Format("mods/{}", mod));

		if (cl_modVidRestart.get<bool>())
		{
			Command::Execute("vid_restart", false);
		}
		else
		{
			Command::Execute("closemenu mods_menu", false);
		}
	}

	CHAR * ModList::StructuredData_GetString(Game::StructuredDataLookup *lookup, Game::StructuredDataBuffer *buffer)
	{
		CHAR* result = Utils::Hook::Call<CHAR*(Game::StructuredDataLookup*, Game::StructuredDataBuffer*)>(0x4D1CE0)(lookup, buffer);

		if (lookup->error == Game::LOOKUP_ERROR_NONE &&
			lookup->type->type == Game::StructuredDataTypeCategory::DATA_STRING &&
			lookup->type->u.stringDataLength == 21)
		{
			// This is a custom class name
			if (*Game::fs_gameDirVar != nullptr && *(*Game::fs_gameDirVar)->current.string != '\0')
			{
				std::string currentName = result;
				if (currentName.empty())
				{
					return result;
				}

				// We're in a modded section - let's prefix that string
				constexpr char PREFIX[] = "* ";
				constexpr auto PREFIX_LENGTH = ARRAYSIZE(PREFIX)-1; // Null-terminated to we have to remove 1

				if (currentName.starts_with(PREFIX))
				{
					return result; // Already prefixed
				}

				const auto prefixedNameLength = lookup->type->u.stringDataLength + PREFIX_LENGTH;

				// Create string if we had not done it before
				if (!customClassesPrefixedNames.contains(lookup->offset))
				{
					customClassesPrefixedNames[lookup->offset] = Utils::Memory::AllocateArray<CHAR>(prefixedNameLength);
				}

				CHAR* namePtr = customClassesPrefixedNames[lookup->offset];

				std::memcpy(&namePtr[PREFIX_LENGTH], result, lookup->type->u.stringDataLength);
				std::memcpy(namePtr, PREFIX, PREFIX_LENGTH);

				return namePtr;
			}
		}

		return result;
	}

	ModList::ModList()
	{
		if (Dedicated::IsEnabled()) return;

		Utils::Hook(0x62C1A1, StructuredData_GetString, HOOK_CALL).install()->quick();

		// The "IWNET storage" system has a "caching" system that, just assumes any file requested twice
		//	is identical between the two accesses if less than 500ms elapsed between them
		// This destroys everything of course, if you mod reload then join a server again
		// Stats get reloaded except if you do it too fast (<500ms) the new stats file never gets read and loaded
		//	because IWNet Storage simply assumes it didn't move!
		// Breaking with a debugger (for more than 500ms) makes the bug disappear.... nasty!
		// This Nop removes that caching mechanism.
		Utils::Hook::Nop(0x60A6DB, 6);


		ModList::CurrentMod = 0;
		cl_modVidRestart = Dvar::Register("cl_modVidRestart", true, Game::DVAR_ARCHIVE, "Perform a vid_restart when loading a mod.");

		UIScript::Add("LoadMods", ModList::UIScript_LoadMods);
		UIScript::Add("RunMod", ModList::UIScript_RunMod);
		UIScript::Add("ClearMods", ModList::UIScript_ClearMods);

		UIFeeder::Add(9.0f, ModList::GetItemCount, ModList::GetItemText, ModList::Select);

		Events::OnCLDisconnected([](bool wasConnected) -> void
		{
			if (Game::cgsArray->localServer)
			{
				// Do not unload when exiting a private match because GH-70
				// Might be okay to do when that PR and related issues are sorted out
				return;
			}

			if (!wasConnected)
			{
				// That means we exited from the main menu - we don't need to clear mods
				// If the server we joined has mods, the Download handler will set them
				return;
			}

			ClearMods();
		});
	}
}
