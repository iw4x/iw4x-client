#include <STDInclude.hpp>

#include <json.hpp>

#include "ScriptStorage.hpp"
#include "Script.hpp"

namespace Components
{
	std::unordered_map<std::string, std::string> ScriptStorage::Data;

	void ScriptStorage::AddScriptFunctions()
	{
		Script::AddFunction("StorageSet", [] // gsc: StorageSet(<str key>, <str data>);
		{
			const auto* key = Game::Scr_GetString(0);
			const auto* value = Game::Scr_GetString(1);

			if (key == nullptr || value == nullptr)
			{
				Game::Scr_Error("^1StorageSet: Illegal parameters!\n");
				return;
			}

			Data.insert_or_assign(key, value);
		});

		Script::AddFunction("StorageRemove", [] // gsc: StorageRemove(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_ParamError(0, "^1StorageRemove: Illegal parameter!\n");
				return;
			}

			if (!Data.contains(key))
			{
				Game::Scr_Error(Utils::String::VA("^1StorageRemove: Store does not have key '%s'!\n", key));
				return;
			}

			Data.erase(key);
		});

		Script::AddFunction("StorageGet", [] // gsc: StorageGet(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_ParamError(0, "^1StorageGet: Illegal parameter!\n");
				return;
			}

			if (!Data.contains(key))
			{
				Game::Scr_Error(Utils::String::VA("^1StorageGet: Store does not have key '%s'!\n", key));
			}

			const auto& data = Data.at(key);
			Game::Scr_AddString(data.data());
		});

		Script::AddFunction("StorageHas", [] // gsc: StorageHas(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_ParamError(0, "^1StorageHas: Illegal parameter!\n");
				return;
			}

			Game::Scr_AddBool(Data.contains(key));
		});

		Script::AddFunction("StorageDump", [] // gsc: StorageDump();
		{
			if (Data.empty())
			{
				Game::Scr_Error("^1StorageDump: ScriptStorage is empty!\n");
				return;
			}

			const nlohmann::json json = Data;

			FileSystem::FileWriter("scriptdata/scriptstorage.json").write(json.dump());
		});

		Script::AddFunction("StorageLoad", [] // gsc: StorageLoad();
		{
			FileSystem::File storageFile("scriptdata/scriptstorage.json");
			if (!storageFile.exists())
			{
				return;
			}

			const auto& buffer = storageFile.getBuffer();
			try
			{
				const nlohmann::json storageDef = nlohmann::json::parse(buffer);
				const auto& newData = storageDef.get<std::unordered_map<std::string, std::string>>();
				Data.insert(newData.begin(), newData.end());
			}
			catch (const std::exception& ex)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "Json Parse Error: {}. File {} is invalid\n", ex.what(), storageFile.getName());
			}
		});

		Script::AddFunction("StorageClear", [] // gsc: StorageClear();
		{
			Data.clear();
		});
	}

	ScriptStorage::ScriptStorage()
	{
		AddScriptFunctions();
	}
}
