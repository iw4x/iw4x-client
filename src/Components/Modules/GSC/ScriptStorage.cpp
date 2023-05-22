#include <STDInclude.hpp>

#include "ScriptStorage.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	std::unordered_map<std::string, std::string> ScriptStorage::Data;

	void ScriptStorage::AddScriptFunctions()
	{
		Script::AddFunction("StorageSet", [] // gsc: StorageSet(<str key>, <str data>);
		{
			const auto* key = Game::Scr_GetString(0);
			const auto* value = Game::Scr_GetString(1);

			if (!key || !value)
			{
				Game::Scr_Error("StorageSet: Illegal parameters!");
				return;
			}

			Data.insert_or_assign(key, value);
		});

		Script::AddFunction("StorageRemove", [] // gsc: StorageRemove(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (!key)
			{
				Game::Scr_ParamError(0, "StorageRemove: Illegal parameter!");
				return;
			}

			if (!Data.contains(key))
			{
				Game::Scr_Error(Utils::String::VA("StorageRemove: Store does not have key '%s'!", key));
				return;
			}

			Data.erase(key);
		});

		Script::AddFunction("StorageGet", [] // gsc: StorageGet(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (!key)
			{
				Game::Scr_ParamError(0, "StorageGet: Illegal parameter!");
				return;
			}

			if (!Data.contains(key))
			{
				Game::Scr_Error(Utils::String::VA("StorageGet: Store does not have key '%s'!", key));
			}

			const auto& data = Data.at(key);
			Game::Scr_AddString(data.data());
		});

		Script::AddFunction("StorageHas", [] // gsc: StorageHas(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (!key)
			{
				Game::Scr_ParamError(0, "StorageHas: Illegal parameter!");
				return;
			}

			Game::Scr_AddBool(Data.contains(key));
		});

		Script::AddFunction("StorageDump", [] // gsc: StorageDump();
		{
			if (Data.empty())
			{
				Game::Scr_Error("StorageDump: ScriptStorage is empty!");
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
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}. File {} is invalid\n", ex.what(), storageFile.getName());
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
