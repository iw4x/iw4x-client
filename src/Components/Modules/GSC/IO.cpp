#include <STDInclude.hpp>
#include "IO.hpp"
#include "Script.hpp"

namespace Components
{
	const char* IO::QueryStrings[] = { R"(..)", R"(../)", R"(..\)" };

	void IO::AddScriptFunctions()
	{
		Script::AddFunction("FileWrite", [] // gsc: FileWrite(<filepath>, <string>, <mode>)
		{
			const auto* path = Game::Scr_GetString(0);
			auto* text = Game::Scr_GetString(1);
			auto* mode = Game::Scr_GetString(2);

			if (!path)
			{
				Game::Scr_ParamError(0, "^1FileWrite: filepath is not defined!\n");
				return;
			}

			if (!text || !mode)
			{
				Game::Scr_Error("^1FileWrite: Illegal parameters!\n");
				return;
			}

			for (std::size_t i = 0; i < ARRAYSIZE(QueryStrings); ++i)
			{
				if (std::strstr(path, QueryStrings[i]) != nullptr)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileWrite: directory traversal is not allowed!\n");
					return;
				}
			}

			if (mode != "append"s && mode != "write"s)
			{
				Logger::Warning(Game::CON_CHANNEL_SCRIPT, "FileWrite: mode not defined or was wrong, defaulting to 'write'\n");
				mode = "write";
			}

			const auto* scriptData = Utils::String::VA("%s/%s", "scriptdata", path);

			if (mode == "write"s)
			{
				FileSystem::FileWriter(scriptData).write(text);
			}
			else if (mode == "append"s)
			{
				FileSystem::FileWriter(scriptData, true).write(text);
			}
		});

		Script::AddFunction("FileRead", [] // gsc: FileRead(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (!path)
			{
				Game::Scr_ParamError(0, "^1FileRead: filepath is not defined!\n");
				return;
			}

			for (std::size_t i = 0; i < ARRAYSIZE(QueryStrings); ++i)
			{
				if (std::strstr(path, QueryStrings[i]) != nullptr)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileRead: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto* scriptData = Utils::String::VA("%s/%s", "scriptdata", path);

			const auto file = FileSystem::FileReader(scriptData);
			if (!file.exists())
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileRead: file '{}' not found!\n", scriptData);
				return;
			}

			auto buffer = file.getBuffer();
			buffer = buffer.substr(0, 1024 - 1); // 1024 is the max string size for the SL system
			Game::Scr_AddString(buffer.data());
		});

		Script::AddFunction("FileExists", [] // gsc: FileExists(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (!path)
			{
				Game::Scr_ParamError(0, "^1FileExists: filepath is not defined!\n");
				return;
			}

			for (std::size_t i = 0; i < ARRAYSIZE(QueryStrings); ++i)
			{
				if (std::strstr(path, QueryStrings[i]) != nullptr)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileExists: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto* scriptData = Utils::String::VA("%s/%s", "scriptdata", path);
			Game::Scr_AddBool(FileSystem::FileReader(scriptData).exists());
		});

		Script::AddFunction("FileRemove", [] // gsc: FileRemove(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (!path)
			{
				Game::Scr_ParamError(0, "^1FileRemove: filepath is not defined!\n");
				return;
			}

			for (std::size_t i = 0; i < ARRAYSIZE(QueryStrings); ++i)
			{
				if (std::strstr(path, QueryStrings[i]) != nullptr)
				{
					Logger::Print("^1FileRemove: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto p = "scriptdata"s / std::filesystem::path(path);
			const auto folder = p.parent_path().string();
			const auto file = p.filename().string();
			Game::Scr_AddInt(FileSystem::_DeleteFile(folder, file));
		});
	}

	IO::IO()
	{
		AddScriptFunctions();
	}
}
