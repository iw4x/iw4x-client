#include "..\STDInclude.hpp"

namespace Components
{
	std::vector<std::string> FastFiles::ZonePaths;

	void FastFiles::LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		Game::XZoneInfo* data = new Game::XZoneInfo[zoneCount + 2];
		memcpy(data, zoneInfo, sizeof(Game::XZoneInfo) * zoneCount);

		data[zoneCount].name = "dlc1_ui_mp";
		data[zoneCount].allocFlags = 2;
		data[zoneCount].freeFlags = 0;
		zoneCount++;

		data[zoneCount].name = "dlc2_ui_mp";
		data[zoneCount].allocFlags = 2;
		data[zoneCount].freeFlags = 0;
		zoneCount++;

		Game::DB_LoadXAssets(data, zoneCount, sync);

		delete[] data;
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

	FastFiles::FastFiles()
	{
		// Redirect zone paths
		Utils::Hook(0x44DA90, FastFiles::GetZoneLocation, HOOK_JUMP).Install()->Quick();

		// Allow dlc ui zone loading
		Utils::Hook(0x506BC7, FastFiles::LoadDLCUIZones, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x60B4AC, FastFiles::LoadDLCUIZones, HOOK_CALL).Install()->Quick();

		// Add custom zone paths
		FastFiles::AddZonePath("zone\\patch\\");
		FastFiles::AddZonePath("zone\\dlc\\");
	}

	FastFiles::~FastFiles()
	{
		FastFiles::ZonePaths.clear();
	}
}
