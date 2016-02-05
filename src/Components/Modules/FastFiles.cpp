#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> FastFiles::ZonePaths;

	// This has to be called only once, when the game starts
	void FastFiles::LoadInitialZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		std::vector<Game::XZoneInfo> data;
		Utils::Merge(&data, zoneInfo, zoneCount);

		// Load custom weapons, if present (force that later on)
		if (FastFiles::Exists("weapons_mp"))
		{
			data.push_back({ "weapons_mp", 1, 0 });
		}

		return FastFiles::LoadDLCUIZones(data.data(), data.size(), sync);
	}

	// This has to be called every time the cgame is reinitialized
	void FastFiles::LoadDLCUIZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		std::vector<Game::XZoneInfo> data;
		Utils::Merge(&data, zoneInfo, zoneCount);

		Game::XZoneInfo info = { nullptr, 2, 0 };

		// Custom ui stuff
		if (FastFiles::Exists("iw4x_ui_mp"))
		{
			info.name = "iw4x_ui_mp";
			data.push_back(info);
		}
		else // Fallback
		{
			info.name = "dlc1_ui_mp";
			data.push_back(info);

			info.name = "dlc2_ui_mp";
			data.push_back(info);
		}

		return FastFiles::LoadLocalizeZones(data.data(), data.size(), sync);
	}

	// This has to be called every time fastfiles are loaded :D
	void FastFiles::LoadLocalizeZones(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		std::vector<Game::XZoneInfo> data;
		Utils::Merge(&data, zoneInfo, zoneCount);

		Game::XZoneInfo info = { nullptr, 4, 0 };

		// Not sure how they should be loaded :S
		std::string langZone = Utils::VA("localized_iw4x_%s", Game::Win_GetLanguage());

		if (FastFiles::Exists(langZone))
		{
			info.name = langZone.data();
		}
		else if (FastFiles::Exists("localized_iw4x_english")) // Fallback
		{
			info.name = "localized_iw4x_english";
		}

		data.push_back(info);

		Game::DB_LoadXAssets(data.data(), data.size(), sync);
	}

	// Name is a bit weird, due to FasFileS and ExistS :P
	bool FastFiles::Exists(std::string file)
	{
		std::string path = FastFiles::GetZoneLocation(file.data());
		path.append(file);

		if (!Utils::EndsWith(path.data(), ".ff"))
		{
			path.append(".ff");
		}

		return (GetFileAttributes(path.data()) != INVALID_FILE_ATTRIBUTES);
	}

	const char* FastFiles::GetZoneLocation(const char* file)
	{
		const char* dir = Dvar::Var("fs_basepath").Get<const char*>();
		
		for (auto &path : FastFiles::ZonePaths)
		{
			std::string absoluteFile = Utils::VA("%s\\%s%s", dir, path.data(), file);

			// No ".ff" appended, append it manually
			if (!Utils::EndsWith(absoluteFile.data(), ".ff"))
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

		if (file == reinterpret_cast<char*>(4))
		{
			return "";
		}

		return file;
	}

	FastFiles::FastFiles()
	{
		Dvar::Register<bool>("ui_zoneDebug", false, Game::dvar_flag::DVAR_FLAG_SAVED, "Display current loaded zone.");

		// Redirect zone paths
		Utils::Hook(0x44DA90, FastFiles::GetZoneLocation, HOOK_JUMP).Install()->Quick();

		// Allow dlc ui zone loading
		if (!ZoneBuilder::IsEnabled())
		{
			Utils::Hook(0x506BC7, FastFiles::LoadInitialZones, HOOK_CALL).Install()->Quick();
			Utils::Hook(0x60B4AC, FastFiles::LoadDLCUIZones, HOOK_CALL).Install()->Quick();
		}

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

		Renderer::OnFrame([] ()
		{
			if (!FastFiles::Current().size() || !Dvar::Var("ui_zoneDebug").Get<bool>()) return;

			Game::Font* font = Game::R_RegisterFont("fonts/consoleFont"); // Inlining that seems to skip xpos, no idea why xD
			float color[4] = { 1.0f, 1.0f, 1.0f, (Game::CL_IsCgameInitialized() ? 0.3f : 1.0f) };
			Game::R_AddCmdDrawText(Utils::VA("Loading FastFile: %s", FastFiles::Current().data()), 0x7FFFFFFF, font, 5.0f, static_cast<float>(Renderer::Height() - 5), 1.0f, 1.0f, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
		});

		Command::Add("loadzone", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			Game::XZoneInfo info;
			info.name = params[1];
			info.allocFlags = 1;//0x01000000;
			info.freeFlags = 0;

			Game::DB_LoadXAssets(&info, 1, true);
		});
	}

	FastFiles::~FastFiles()
	{
		FastFiles::ZonePaths.clear();
	}
}
