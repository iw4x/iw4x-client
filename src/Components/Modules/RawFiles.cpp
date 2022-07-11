#include <STDInclude.hpp>

namespace Components
{
	char* RawFiles::ReadRawFile(const char* filename, char* buf, int size)
	{
		auto fileHandle = 0;
		auto fileSize = Game::FS_FOpenFileRead(filename, &fileHandle);

		if (fileHandle != 0)
		{
			if ((fileSize + 1) <= size)
			{
				Game::FS_Read(buf, fileSize, fileHandle);
				buf[fileSize] = 0;
				Game::FS_FCloseFile(fileHandle);
				return buf;
			}

			Game::FS_FCloseFile(fileHandle);
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "Ignoring raw file '{}' as it exceeds buffer size {} > {}\n", filename, fileSize, size);
		}

		auto* rawfile = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_RAWFILE, filename).rawfile;
		if (Game::DB_IsXAssetDefault(Game::ASSET_TYPE_RAWFILE, filename))
		{
			return nullptr;
		}

		Game::DB_GetRawBuffer(rawfile, buf, size);
		return buf;
	}

	char* RawFiles::GetMenuBuffer(const char* filename)
	{
		auto fileHandle = 0;
		auto fileSize = Game::FS_FOpenFileRead(filename, &fileHandle);

		if (fileHandle != 0)
		{
			if (fileSize < 0x8000)
			{
				auto* buffer = static_cast<char*>(Game::Z_VirtualAlloc(fileSize + 1));
				Game::FS_Read(buffer, fileSize, fileHandle);
				Game::FS_FCloseFile(fileHandle);
				return buffer;
			}

			Game::FS_FCloseFile(fileHandle);
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "Menu file too large: {} is {}, max allowed is {}\n", filename, fileSize, 0x8000);
		}

		auto* rawfile = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_RAWFILE, filename).rawfile;
		if (Game::DB_IsXAssetDefault(Game::ASSET_TYPE_RAWFILE, filename))
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "Menu file not found: {}, using default\n", filename);
			return nullptr;
		}

		auto* buffer = static_cast<char*>(Game::Z_VirtualAlloc(rawfile->len + 1));
		Game::DB_GetRawBuffer(rawfile, buffer, rawfile->len + 1);
		return buffer;
	}

	RawFiles::RawFiles()
	{
		// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
		Utils::Hook::Nop(0x61AB76, 2);

		Utils::Hook(0x4DA0D0, ReadRawFile, HOOK_JUMP).install()->quick();
		Utils::Hook(0x631640, GetMenuBuffer, HOOK_JUMP).install()->quick();

		Command::Add("dumpraw", [](Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("Specify a filename!\n");
				return;
			}

			FileSystem::File file(params->join(1));
			if (file.exists())
			{
				Utils::IO::WriteFile("raw/" + file.getName(), file.getBuffer());
				Logger::Print("File '{}' written to raw!\n", file.getName());
				return;
			}

			const char* data = Game::Scr_AddSourceBuffer(nullptr, file.getName().data(), nullptr, false);

			if (data != nullptr)
			{
				Utils::IO::WriteFile("raw/" + file.getName(), data);
				Logger::Print("File '{}' written to raw!\n", file.getName());
			}
			else
			{
				Logger::Print("File '{}' does not exist!\n", file.getName());
			}
		});
	}
}
