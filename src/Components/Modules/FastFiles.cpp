#include "..\..\STDInclude.hpp"

namespace Components
{
	std::vector<std::string> FastFiles::ZonePaths;

	void FastFiles::LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		std::vector<Game::XZoneInfo> data;
		Utils::Merge(data, zoneInfo, zoneCount);

		Game::XZoneInfo info = { nullptr, 2, 0 };

		info.name = "dlc1_ui_mp";
		data.push_back(info);

		info.name = "dlc2_ui_mp";
		data.push_back(info);

		Game::DB_LoadXAssets(data.data(), data.size(), sync);
	}

	const char* FastFiles::GetZoneLocation(const char* file)
	{
		const char* dir = Dvar::Var("fs_basepath").Get<const char*>();
		
		for (auto &path : FastFiles::ZonePaths)
		{
			std::string absoluteFile = Utils::VA("%s\\%s%s", dir, path.data(), file);

			// No ".ff" appended, append it manually
			if (!Utils::EndsWith(file, ".ff"))
			{
				absoluteFile.append(".ff");
			}

			// Check if FastFile exists
			if (GetFileAttributes(absoluteFile.data()) != INVALID_FILE_ATTRIBUTES)
			{
				return Utils::VA("%s", path.data());
			}
		}

		return Utils::VA("zone\\%s\\", Game::Win_GetLanguage());
	}

	void FastFiles::AddZonePath(std::string path)
	{
		FastFiles::ZonePaths.push_back(path);
	}

	std::string FastFiles::Current()
	{
		const char* file = (Utils::Hook::Get<char*>(0x112A680) + 4);

		if ((int)file == 4)
		{
			return "";
		}

		return file;
	}

	FastFiles::FastFiles()
	{
		// Redirect zone paths
		Utils::Hook(0x44DA90, FastFiles::GetZoneLocation, HOOK_JUMP).Install()->Quick();

		// Allow dlc ui zone loading
		Utils::Hook(0x506BC7, FastFiles::LoadDLCUIZones, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x60B4AC, FastFiles::LoadDLCUIZones, HOOK_CALL).Install()->Quick();

		// basic checks (hash jumps, both normal and playlist)
		Utils::Hook::Nop(0x5B97A3, 2);
		Utils::Hook::Nop(0x5BA493, 2);

		Utils::Hook::Nop(0x5B991C, 2);
		Utils::Hook::Nop(0x5BA60C, 2);

		Utils::Hook::Nop(0x5B97B4, 2);
		Utils::Hook::Nop(0x5BA4A4, 2);

		// allow loading of IWffu (unsigned) files
		Utils::Hook::Set<BYTE>(0x4158D9, 0xEB); // main function
		Utils::Hook::Nop(0x4A1D97, 2); // DB_AuthLoad_InflateInit

		// some other, unknown, check
		Utils::Hook::Set<BYTE>(0x5B9912, 0xB8);
		Utils::Hook::Set<DWORD>(0x5B9913, 1);

		Utils::Hook::Set<BYTE>(0x5BA602, 0xB8);
		Utils::Hook::Set<DWORD>(0x5BA603, 1);

		// Add custom zone paths
		FastFiles::AddZonePath("zone\\patch\\");
		FastFiles::AddZonePath("zone\\dlc\\");
	}

	FastFiles::~FastFiles()
	{
		FastFiles::ZonePaths.clear();
	}
}
