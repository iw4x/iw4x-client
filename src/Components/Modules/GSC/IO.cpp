#include <STDInclude.hpp>
#include "IO.hpp"
#include "Script.hpp"

namespace Components
{
	const char* IO::QueryStrings[] = { R"(..)", R"(../)", R"(..\)" };

	void IO::AddScriptFunctions()
	{
		Script::AddFunction("FileWrite", [] // gsc: iw4x_FileWrite(<filepath>, <string>, <mode>)
		{
			const auto* path = Game::Scr_GetString(0);
			auto* text = Game::Scr_GetString(1);
			auto* mode = Game::Scr_GetString(2);

			if (path == nullptr)
			{
				Game::Scr_ParamError(0, "^1FileWrite: filepath is not defined!\n");
				return;
			}

			if (text == nullptr || mode == nullptr)
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

		Script::AddFunction("FileRead", [] // gsc: iw4x_FileRead(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (path == nullptr)
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

			if (!FileSystem::FileReader(scriptData).exists())
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileRead: file '{}' not found!\n", scriptData);
				return;
			}

			Game::Scr_AddString(FileSystem::FileReader(scriptData).getBuffer().data());
		});

		Script::AddFunction("FileExists", [] // gsc: iw4x_FileExists(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (path == nullptr)
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
	}

	IO::IO()
	{
		AddScriptFunctions();
	}
}
