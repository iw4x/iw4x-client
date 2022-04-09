#include <STDInclude.hpp>

namespace Components
{
	const char* ScriptExtension::QueryStrings[] = { R"(..)", R"(../)", R"(..\)" };

	void ScriptExtension::AddFunctions()
	{
		//File functions
		Script::AddFunction("FileWrite", []() // gsc: FileWrite(<filepath>, <string>, <mode>)
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

			for (auto i = 0u; i < ARRAYSIZE(ScriptExtension::QueryStrings); ++i)
			{
				if (std::strstr(path, ScriptExtension::QueryStrings[i]) != nullptr)
				{
					Logger::Print("^1FileWrite: directory traversal is not allowed!\n");
					return;
				}
			}

			if (mode != "append"s && mode != "write"s)
			{
				Logger::Print("^3FileWrite: mode not defined or was wrong, defaulting to 'write'\n");
				mode = "write";
			}

			if (mode == "write"s)
			{
				FileSystem::FileWriter(path).write(text);
			}
			else if (mode == "append"s)
			{
				FileSystem::FileWriter(path, true).write(text);
			}
		});

		Script::AddFunction("FileRead", []() // gsc: FileRead(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (path == nullptr)
			{
				Game::Scr_ParamError(0, "^1FileRead: filepath is not defined!\n");
				return;
			}

			for (auto i = 0u; i < ARRAYSIZE(ScriptExtension::QueryStrings); ++i)
			{
				if (std::strstr(path, ScriptExtension::QueryStrings[i]) != nullptr)
				{
					Logger::Print("^1FileRead: directory traversal is not allowed!\n");
					return;
				}
			}

			if (!FileSystem::FileReader(path).exists())
			{
				Logger::Print("^1FileRead: file not found!\n");
				return;
			}

			Game::Scr_AddString(FileSystem::FileReader(path).getBuffer().data());
		});

		Script::AddFunction("FileExists", []() // gsc: FileExists(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (path == nullptr)
			{
				Game::Scr_ParamError(0, "^1FileExists: filepath is not defined!\n");
				return;
			}

			for (auto i = 0u; i < ARRAYSIZE(ScriptExtension::QueryStrings); ++i)
			{
				if (std::strstr(path, ScriptExtension::QueryStrings[i]) != nullptr)
				{
					Logger::Print("^1FileExists: directory traversal is not allowed!\n");
					return;
				}
			}

			Game::Scr_AddInt(FileSystem::FileReader(path).exists());
		});

		Script::AddFunction("FileRemove", []() // gsc: FileRemove(<filepath>)
		{
			const auto* path = Game::Scr_GetString(0);

			if (path == nullptr)
			{
				Game::Scr_ParamError(0, "^1FileRemove: filepath is not defined!\n");
				return;
			}

			for (auto i = 0u; i < ARRAYSIZE(ScriptExtension::QueryStrings); ++i)
			{
				if (std::strstr(path, ScriptExtension::QueryStrings[i]) != nullptr)
				{
					Logger::Print("^1FileRemove: directory traversal is not allowed!\n");
					return;
				}
			}

			const auto p = std::filesystem::path(path);
			const auto& folder = p.parent_path().string();
			const auto& file = p.filename().string();
			Game::Scr_AddInt(FileSystem::DeleteFile(folder, file));
		});
	}

	void ScriptExtension::AddMethods()
	{
		// ScriptExtension methods
		Script::AddMethod("GetIp", [](Game::scr_entref_t entref) // gsc: self GetIp()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			std::string ip = Game::NET_AdrToString(client->netchan.remoteAddress);

			if (const auto pos = ip.find_first_of(":"); pos != std::string::npos)
				ip.erase(ip.begin() + pos, ip.end()); // Erase port

			Game::Scr_AddString(ip.data());
		});

		Script::AddMethod("GetPing", [](Game::scr_entref_t entref) // gsc: self GetPing()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			Game::Scr_AddInt(client->ping);
		});
	}

	void ScriptExtension::Scr_TableLookupIStringByRow()
	{
		if (Game::Scr_GetNumParam() < 3)
		{
			Game::Scr_Error("USAGE: tableLookupIStringByRow( filename, rowNum, returnValueColumnNum )\n");
			return;
		}

		const auto* fileName = Game::Scr_GetString(0);
		const auto rowNum = Game::Scr_GetInt(1);
		const auto returnValueColumnNum = Game::Scr_GetInt(2);

		const auto* table = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_STRINGTABLE, fileName).stringTable;

		if (table == nullptr)
		{
			Game::Scr_ParamError(0, Utils::String::VA("%s does not exist\n", fileName));
			return;
		}

		const auto* value = Game::StringTable_GetColumnValueForRow(table, rowNum, returnValueColumnNum);
		Game::Scr_AddIString(value);
	}

	ScriptExtension::ScriptExtension()
	{
		ScriptExtension::AddFunctions();
		ScriptExtension::AddMethods();
		// Correct builtin function pointer
		Utils::Hook::Set<void(*)()>(0x79A90C, ScriptExtension::Scr_TableLookupIStringByRow);
	}
}
