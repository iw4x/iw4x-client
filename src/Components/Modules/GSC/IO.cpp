#include <STDInclude.hpp>
#include "IO.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	const char* IO::ForbiddenStrings[] = { R"(..)", R"(../)", R"(..\)" };

	std::filesystem::path IO::Path;

	void IO::AddScriptFunctions()
	{
		Script::AddFunction("FileWrite", [] // gsc: FileWrite(<filepath>, <string>, <mode>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			auto* text = Game::Scr_GetString(1);
			auto* mode = Game::Scr_GetString(2);

			if (!filepath)
			{
				Game::Scr_ParamError(0, "FileWrite: filepath is not defined!");
				return;
			}

			if (!text || !mode)
			{
				Game::Scr_Error("FileWrite: Illegal parameters!");
				return;
			}

			for (std::size_t i = 0; i < std::extent_v<decltype(ForbiddenStrings)>; ++i)
			{
				if (std::strstr(filepath, ForbiddenStrings[i]) != nullptr)
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

			const auto append = mode == "append"s;
			const auto scriptData = Path / "scriptdata"s / filepath;
			Utils::IO::WriteFile(scriptData.string(), text, append);
		});

		Script::AddFunction("FileRead", [] // gsc: FileRead(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!filepath)
			{
				Game::Scr_ParamError(0, "FileRead: filepath is not defined!");
				return;
			}

			for (std::size_t i = 0; i < std::extent_v<decltype(ForbiddenStrings)>; ++i)
			{
				if (std::strstr(filepath, ForbiddenStrings[i]) != nullptr)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileRead: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto scriptData = Path / "scriptdata"s / filepath;

			std::string file;
			if (!Utils::IO::ReadFile(scriptData.string(), &file))
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileRead: file '{}' not found!\n", scriptData.string());
				return;
			}

			file = file.substr(0, 1024 - 1); // 1024 is the max string size for the SL system
			Game::Scr_AddString(file.data());
		});

		Script::AddFunction("FileExists", [] // gsc: FileExists(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!filepath)
			{
				Game::Scr_ParamError(0, "FileExists: filepath is not defined!");
				return;
			}

			for (std::size_t i = 0; i < std::extent_v<decltype(ForbiddenStrings)>; ++i)
			{
				if (std::strstr(filepath, ForbiddenStrings[i]) != nullptr)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "FileExists: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto scriptData = Path / "scriptdata"s / filepath;
			Game::Scr_AddBool(Utils::IO::FileExists(scriptData.string()));
		});

		Script::AddFunction("FileRemove", [] // gsc: FileRemove(<filepath>)
		{
			const auto* filepath = Game::Scr_GetString(0);
			if (!filepath)
			{
				Game::Scr_ParamError(0, "FileRemove: filepath is not defined!");
				return;
			}

			for (std::size_t i = 0; i < std::extent_v<decltype(ForbiddenStrings)>; ++i)
			{
				if (std::strstr(filepath, ForbiddenStrings[i]) != nullptr)
				{
					Logger::Print("FileRemove: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto scriptData = Path / "scriptdata"s / filepath;
			Game::Scr_AddInt(Utils::IO::RemoveFile(scriptData.string()));
		});
	}

	IO::IO()
	{
		Path = "userraw"s;

		AddScriptFunctions();
	}
}
