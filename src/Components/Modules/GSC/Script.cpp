#include <STDInclude.hpp>
#include "Script.hpp"

namespace Components::GSC
{
	std::vector<Script::ScriptFunction> Script::CustomScrFunctions;
	std::vector<Script::ScriptMethod> Script::CustomScrMethods;

	std::unordered_map<std::string, int> Script::ScriptMainHandles;
	std::unordered_map<std::string, int> Script::ScriptInitHandles;

	void Script::Scr_LoadGameType_Stub()
	{
		for (const auto& handle : ScriptMainHandles)
		{
			Logger::Print("Executing '{}::main'\n", handle.first.data());

			const auto id = Game::Scr_ExecThread(handle.second, 0);
			Game::Scr_FreeThread(static_cast<std::uint16_t>(id));
		}

		Game::Scr_LoadGameType();
	}

	void Script::Scr_StartupGameType_Stub()
	{
		for (const auto& handle : ScriptInitHandles)
		{
			Logger::Print("Executing '{}::init'\n", handle.first.data());

			const auto id = Game::Scr_ExecThread(handle.second, 0);
			Game::Scr_FreeThread(static_cast<std::uint16_t>(id));
		}

		Game::Scr_StartupGameType();
	}

	void Script::LoadCustomScriptsFromFolder(const char* dir)
	{
		char path[MAX_OSPATH]{};
		char searchPath[MAX_OSPATH]{};

		strncpy_s(searchPath, dir, _TRUNCATE);
		strncat_s(searchPath, "/", _TRUNCATE);

		auto numFiles = 0;
		const auto** files = Game::FS_ListFiles(searchPath, "gsc", Game::FS_LIST_ALL, &numFiles, 10);

		for (auto i = 0; i < numFiles; ++i)
		{
			const auto* scriptFile = files[i];
			Logger::Print("Loading script {}...\n", scriptFile);

			const auto len = sprintf_s(path, "%s/%s", dir, scriptFile);
			if (len == -1)
			{
				continue;
			}

			// Scr_LoadScriptInternal will add the '.gsc' suffix so we remove it
			path[len - 4] = '\0';

			if (!Game::Scr_LoadScript(path))
			{
				Logger::Print("Script {} encountered an error while loading. A compilation error is the most likely cause\n", path);
				continue;
			}

			Logger::Print("Script {}.gsc loaded successfully.\n", path);

			const auto initHandle = Game::Scr_GetFunctionHandle(path, "init");
			if (initHandle != 0)
			{
				Logger::Debug("Loaded '{}::init'", path);
				ScriptInitHandles.insert_or_assign(path, initHandle);
			}

			const auto mainHandle = Game::Scr_GetFunctionHandle(path, "main");
			if (mainHandle != 0)
			{
				Logger::Debug("Loaded '{}::main'", path);
				ScriptMainHandles.insert_or_assign(path, mainHandle);
			}

			// Allow scripts with no handles
		}

		Game::FS_FreeFileList(files, 10);
	}

	void Script::LoadCustomScripts()
	{
		LoadCustomScriptsFromFolder("scripts");

		// Game specific
		const auto* gameDir = "scripts/mp";
		LoadCustomScriptsFromFolder(gameDir);

		// Map specific
		const auto* mapDir = Utils::String::Format("scripts/mp/{}", (*Game::sv_mapname)->current.string);
		LoadCustomScriptsFromFolder(mapDir);

		// Mode specific
		const auto* modeDir = Utils::String::Format("scripts/mp/{}", (*Game::g_gametype)->current.string);
		LoadCustomScriptsFromFolder(modeDir);
	}

	// Do not use C++ objects because Scr_LoadScript may longjmp and crash or leak memory
	void Script::GScr_LoadGameTypeScript_Stub()
	{
		// Clear handles (from previous GSC loading session)
		ScriptMainHandles.clear();
		ScriptInitHandles.clear();

		LoadCustomScripts();

		Game::GScr_LoadGameTypeScript();
	}

	void Script::AddFunction(const std::string& name, const Game::BuiltinFunction func, const bool type)
	{
		ScriptFunction toAdd;
		toAdd.actionFunc = func;
		toAdd.type = type;
		toAdd.aliases.push_back({Utils::String::ToLower(name)});

		CustomScrFunctions.emplace_back(toAdd);
	}

	void Script::AddMethod(const std::string& name, const Game::BuiltinMethod func, const bool type)
	{
		ScriptMethod toAdd;
		toAdd.actionFunc = func;
		toAdd.type = type;
		toAdd.aliases.push_back({Utils::String::ToLower(name)});

		CustomScrMethods.emplace_back(toAdd);
	}

	void Script::AddFuncMultiple(Game::BuiltinFunction func, bool type, scriptNames aliases)
	{
		ScriptFunction toAdd;
		auto aliasesToAdd = Utils::String::ApplyToLower(aliases);

		toAdd.actionFunc = func;
		toAdd.type = type;
		toAdd.aliases = std::move(aliasesToAdd);

		CustomScrFunctions.emplace_back(toAdd);
	}

	void Script::AddMethMultiple(Game::BuiltinMethod func, bool type, scriptNames aliases)
	{
		ScriptMethod toAdd;
		auto aliasesToAdd = Utils::String::ApplyToLower(aliases);

		toAdd.actionFunc = func;
		toAdd.type = type;
		toAdd.aliases = std::move(aliasesToAdd);

		CustomScrMethods.emplace_back(toAdd);
	}

	Game::BuiltinFunction Script::BuiltIn_GetFunctionStub(const char** pName, int* type)
	{
		if (pName != nullptr)
		{
			const auto name = Utils::String::ToLower(*pName);
			for (const auto& funcs : CustomScrFunctions)
			{
				if (std::ranges::find(funcs.aliases, name) != funcs.aliases.end())
				{
					*type = funcs.type;
					return funcs.actionFunc;
				}
			}
		}
		else
		{
			for (const auto& func : CustomScrFunctions)
			{
				const auto& name = func.aliases.at(0);
				Game::Scr_RegisterFunction(reinterpret_cast<int>(func.actionFunc), name.data());
			}
		}

		// If no function was found let's call game's function
		return Utils::Hook::Call<Game::BuiltinFunction(const char**, int*)>(0x5FA2B0)(pName, type); // BuiltIn_GetFunction
	}

	Game::BuiltinMethod Script::BuiltIn_GetMethodStub(const char** pName, int* type)
	{
		if (pName != nullptr)
		{
			const auto name = Utils::String::ToLower(*pName);
			for (const auto& meths : CustomScrMethods)
			{
				if (std::ranges::find(meths.aliases, name) != meths.aliases.end())
				{
					*type = meths.type;
					return meths.actionFunc;
				}
			}
		}
		else
		{
			for (const auto& meth : CustomScrMethods)
			{
				const auto& name = meth.aliases.at(0);
				Game::Scr_RegisterFunction(reinterpret_cast<int>(meth.actionFunc), name.data());
			}
		}

		// If no method was found let's call game's function
		return Utils::Hook::Call<Game::BuiltinMethod(const char**, int*)>(0x5FA360)(pName, type); // Player_GetMethod
	}

	unsigned int Script::SetExpFogStub()
	{
		if (Game::Scr_GetNumParam() == 6)
		{
			std::memmove(&Game::scrVmPub->top[-4], &Game::scrVmPub->top[-5], sizeof(Game::VariableValue) * 6);
			Game::scrVmPub->top += 1;
			Game::scrVmPub->top[-6].type = Game::VAR_FLOAT;
			Game::scrVmPub->top[-6].u.floatValue = 0.0f;

			++Game::scrVmPub->outparamcount;
		}

		return Game::Scr_GetNumParam();
	}

	Game::client_s* Script::GetClient(const Game::gentity_s* ent)
	{
		assert(ent);

		if (!ent->client)
		{
			Game::Scr_ObjectError(Utils::String::VA("Entity %i is not a player", ent->s.number));
			return nullptr;
		}

		if (static_cast<std::size_t>(ent->s.number) >= Game::MAX_CLIENTS)
		{
			Game::Scr_ObjectError(Utils::String::VA("Entity %i is out of bounds", ent->s.number));
			return nullptr;
		}

		return &Game::svs_clients[ent->s.number];
	}

	Game::gentity_s* Script::Scr_GetPlayerEntity(Game::scr_entref_t entref)
	{
		if (entref.classnum)
		{
			Game::Scr_ObjectError("not an entity");
			return nullptr;
		}

		assert(entref.entnum < Game::MAX_GENTITIES);

		auto* ent = &Game::g_entities[entref.entnum];
		if (!ent->client)
		{
			Game::Scr_ObjectError(Utils::String::VA("entity %hu is not a player", entref.entnum));
			return nullptr;
		}

		return ent;
	}

	Script::Script()
	{
		// Skip check in GScr_CheckAllowedToSetPersistentData to prevent log spam in RuntimeError.
		// On IW5 the function is entirely nullsubbed
		Utils::Hook::Set<std::uint8_t>(0x5F8DBF, 0xEB);

		Utils::Hook(0x48EFFE, Scr_LoadGameType_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x48F008, Scr_StartupGameType_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x45D44A, GScr_LoadGameTypeScript_Stub, HOOK_CALL).install()->quick();

		// Fetch custom functions
		Utils::Hook(0x44E72E, BuiltIn_GetFunctionStub, HOOK_CALL).install()->quick(); // Scr_GetFunction
		Utils::Hook(0x4EC8DD, BuiltIn_GetMethodStub, HOOK_CALL).install()->quick(); // Scr_GetMethod

		Utils::Hook(0x5F41A3, SetExpFogStub, HOOK_CALL).install()->quick();

		// Restore IW3's compiler behaviour when dealing with 'overriding builtin function'
		Utils::Hook::Nop(0x613EDA, 2); // Scr_GetFunction
		Utils::Hook::Nop(0x613EF0, 2); // Scr_GetMethod
	}
}
