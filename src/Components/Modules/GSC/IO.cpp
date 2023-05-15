#include <STDInclude.hpp>

#include <Components/Modules/Events.hpp>

#include "IO.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	const char* IO::ForbiddenStrings[] = { R"(..)", R"(../)", R"(..\)" };

	FILE* IO::openScriptIOFileHandle;

	std::filesystem::path IO::DefaultDestPath;

	bool IO::ValidatePath(const char* function, const char* path)
	{
		for (std::size_t i = 0; i < std::extent_v<decltype(ForbiddenStrings)>; ++i)
		{
			if (std::strstr(path, ForbiddenStrings[i]) != nullptr)
			{
				Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "{}: directory traversal is not allowed!\n", function);
				return false;
			}
		}

		return true;
	}

	std::filesystem::path IO::BuildPath(const char* path)
	{
		const std::filesystem::path fsGame = (*Game::fs_gameDirVar)->current.string;
		if (!fsGame.empty())
		{
			return fsGame / "scriptdata"s / path;
		}

		return DefaultDestPath / "scriptdata"s / path;
	}

	void IO::GScr_OpenFile()
	{
		const auto* filepath = Game::Scr_GetString(0);
		const auto* mode = Game::Scr_GetString(1);

		if (!ValidatePath("OpenFile", filepath))
		{
			Game::Scr_AddInt(-1);
			return;
		}

		if (mode != "read"s)
		{
			Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "Valid openfile modes are 'read'\n");
			Game::Scr_AddInt(-1);
			return;
		}

		if (openScriptIOFileHandle)
		{
			Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "OpenFile failed. {} files already open\n", 1);
			Game::Scr_AddInt(-1);
			return;
		}

		const auto dest = BuildPath(filepath);

		_set_errno(0);
		const auto result = fopen_s(&openScriptIOFileHandle, dest.string().data(), "r");
		if (result || !openScriptIOFileHandle)
		{
			Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "OpenFile failed. '{}'", result);
			Game::Scr_AddInt(-1);
			return;
		}

		Game::Scr_AddInt(1);
	}

	void IO::GScr_ReadStream()
	{
		if (!openScriptIOFileHandle)
		{
			Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "ReadStream failed. File stream was not opened\n");
			return;
		}

		char line[1024]{};
		if (std::fgets(line, sizeof(line), openScriptIOFileHandle) != nullptr)
		{
			Game::Scr_AddString(line);
			return;
		}

		Logger::Warning(Game::CON_CHANNEL_PARSERSCRIPT, "ReadStream failed.\n");
		
		if (std::feof(openScriptIOFileHandle))
		{
			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "ReadStream: EOF reached\n");
		}
	}

	void IO::GScr_CloseFile()
	{
		if (!openScriptIOFileHandle)
		{
			Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "CloseFile failed. File stream was not opened\n");
			Game::Scr_AddInt(-1);
			return;
		}

		Game::Scr_AddInt(std::fclose(openScriptIOFileHandle));
		openScriptIOFileHandle = nullptr;
	}

	void IO::AddScriptFunctions()
	{
		Script::AddFunction("FileWrite", [] // gsc: FileWrite(<filepath>, <string>, <mode>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			const auto* text = Game::Scr_GetString(1);
			const auto* mode = Game::Scr_GetString(2);

			if (!ValidatePath("FileWrite", filepath))
			{
				return;
			}

			if (mode != "append"s && mode != "write"s)
			{
				Logger::Warning(Game::CON_CHANNEL_PARSERSCRIPT, "FileWrite: mode not defined or was wrong, defaulting to 'write'\n");
				mode = "write";
			}

			const auto append = mode == "append"s;
			const auto dest = BuildPath(filepath);
			Utils::IO::WriteFile(dest.string(), text, append);
		});

		Script::AddFunction("FileRead", [] // gsc: FileRead(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!ValidatePath("FileRead", filepath))
			{
				return;
			}

			const auto dest = BuildPath(filepath);

			std::string file;
			if (!Utils::IO::ReadFile(dest.string(), &file))
			{
				Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "FileRead: file '{}' not found!\n", dest.string());
				return;
			}

			file = file.substr(0, 1024 - 1); // 1024 is the max string size for the SL system
			Game::Scr_AddString(file.data());
		});

		Script::AddFunction("FileExists", [] // gsc: FileExists(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!ValidatePath("FileExists", filepath))
			{
				return;
			}

			const auto dest = BuildPath(filepath);
			Game::Scr_AddBool(Utils::IO::FileExists(dest.string()));
		});

		Script::AddFunction("FileRemove", [] // gsc: FileRemove(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!ValidatePath("FileRemove", filepath))
			{
				return;
			}

			const auto dest = BuildPath(filepath);
			Game::Scr_AddBool(Utils::IO::RemoveFile(dest.string()));
		});

		Script::AddFunction("FileRename", [] // gsc: FileRename(<filepath>, <filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			const auto* destpath = Game::Scr_GetString(0);
			if (!ValidatePath("FileRename", filepath) || !ValidatePath("FileRename", destpath))
			{
				return;
			}

			const auto from = BuildPath(filepath);
			const auto to = BuildPath(destpath);

			std::error_code err;
			std::filesystem::rename(from, to, err);
			if (err.value())
			{
				Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "FileRename: failed to rename file! Error message: {}\n", err.message());
				Game::Scr_AddInt(-1);
				return;
			}

			Game::Scr_AddInt(1);
		});

		Script::AddFunction("FileCopy", [] // gsc: FileCopy(<filepath>, <filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			const auto* destpath = Game::Scr_GetString(0);
			if (!ValidatePath("FileCopy", filepath) || !ValidatePath("FileCopy", destpath))
			{
				return;
			}

			const auto from = BuildPath(filepath);
			const auto to = BuildPath(destpath);

			std::error_code err;
			std::filesystem::copy(from, to, err);
			if (err.value())
			{
				Logger::PrintError(Game::CON_CHANNEL_PARSERSCRIPT, "FileCopy: failed to copy file! Error message: {}\n", err.message());
				Game::Scr_AddInt(-1);
				return;
			}

			Game::Scr_AddInt(1);
		});

		Script::AddFunction("ReadStream", GScr_ReadStream);
	}

	IO::IO()
	{
		openScriptIOFileHandle = nullptr;
		DefaultDestPath = "userraw"s;

		AddScriptFunctions();

		Utils::Hook::Set<Game::BuiltinFunction>(0x79A858, GScr_OpenFile);
		Utils::Hook::Set<int>(0x79A85C, 0);

		Utils::Hook::Set<Game::BuiltinFunction>(0x79A864, GScr_CloseFile);
		Utils::Hook::Set<int>(0x79A868, 0);

		Events::OnVMShutdown([]
		{
			if (openScriptIOFileHandle)
			{
				std::fclose(openScriptIOFileHandle);
				openScriptIOFileHandle = nullptr;
			}
		});
	}
}
