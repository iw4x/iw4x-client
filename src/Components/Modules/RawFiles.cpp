#include <STDInclude.hpp>
#include "RawFiles.hpp"

namespace Components
{
	char* RawFiles::ReadRawFile(const char* filename, char* buf, int size)
	{
		auto fileHandle = 0;
		auto fileSize = Game::FS_FOpenFileRead(filename, &fileHandle);

		if (fileHandle)
		{
			if ((fileSize + 1) <= size)
			{
				Game::FS_Read(buf, fileSize, fileHandle);
				buf[fileSize] = '\0';
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

		if (fileHandle)
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

	char* RawFiles::Com_LoadInfoString_LoadObj(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer)
	{
		auto fileHandle = 0;

		const auto fileLen = Game::FS_FOpenFileByMode(fileName, &fileHandle, Game::FS_READ);
		if (fileLen < 0)
		{
			Logger::Debug("Could not load {} [{}] as rawfile", fileDesc, fileName);
			return nullptr;
		}

		const auto identLen = static_cast<int>(std::strlen(ident));
		Game::FS_Read(loadBuffer, identLen, fileHandle);
		loadBuffer[identLen] = '\0';

		if (std::strncmp(loadBuffer, ident, identLen) != 0)
		{
			Game::Com_Error(Game::ERR_DROP, "\x15" "File [%s] is not a %s\n", fileName, fileDesc);
			return nullptr;
		}

		if ((fileLen - identLen) >= 0x4000)
		{
			Game::Com_Error(Game::ERR_DROP, "\x15" "File [%s] is too long of a %s to parse\n", fileName, fileDesc);
			return nullptr;
		}

		Game::FS_Read(loadBuffer, fileLen - identLen, fileHandle);
		loadBuffer[fileLen - identLen] = '\0';
		Game::FS_FCloseFile(fileHandle);

		return loadBuffer;
	}

	const char* RawFiles::Com_LoadInfoString_Hk(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer)
	{

		const auto* buffer = Com_LoadInfoString_LoadObj(fileName, fileDesc, ident, loadBuffer);
		if (!buffer)
		{
			buffer = Game::Com_LoadInfoString_FastFile(fileName, fileDesc, ident, loadBuffer);
		}

		if (!Game::Info_Validate(buffer))
		{
			Game::Com_Error(Game::ERR_DROP, "\x15" "File [%s] is not a valid %s\n", fileName, fileDesc);
			return nullptr;
		}

		return buffer;
	}

	RawFiles::RawFiles()
	{
		// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
		Utils::Hook::Nop(0x61AB76, 2);

		Utils::Hook(0x4DA0D0, ReadRawFile, HOOK_JUMP).install()->quick();
		Utils::Hook(0x631640, GetMenuBuffer, HOOK_JUMP).install()->quick();
		Utils::Hook(0x463500, Com_LoadInfoString_Hk, HOOK_JUMP).install()->quick();

		Command::Add("dumpraw", [](const Command::Params* params)
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

			const auto* data = Game::Scr_AddSourceBuffer(nullptr, file.getName().data(), nullptr, false);

			if (data)
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
