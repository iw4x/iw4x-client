#include <STDInclude.hpp>
#include "Script.hpp"

namespace Components
{
	std::vector<Script::ScriptFunction> Script::CustomScrFunctions;
	std::vector<Script::ScriptMethod> Script::CustomScrMethods;

	int Script::LastFrameTime = -1;

	std::unordered_map<const char*, const char*> Script::ReplacedFunctions;
	const char* Script::ReplacedPos = nullptr;

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

	// Do not use C++ objects because Scr_LoadScript may longjmp
	void Script::GScr_LoadGameTypeScript_Stub()
	{
		// Clear handles (from previous GSC loading session)
		ScriptMainHandles.clear();
		ScriptInitHandles.clear();

		char path[MAX_PATH]{};

		auto numFiles = 0;
		const auto** files = Game::FS_ListFiles("scripts/", "gsc", Game::FS_LIST_ALL, &numFiles, 10);

		for (auto i = 0; i < numFiles; ++i)
		{
			const auto* scriptFile = files[i];
			Logger::Print("Loading script {}...\n", scriptFile);

			sprintf_s(path, "%s/%s", "scripts", scriptFile);

			// Scr_LoadScriptInternal will add the '.gsc' suffix so we remove it
			path[std::strlen(path) - 4] = '\0';

			if (!Game::Scr_LoadScript(path))
			{
				Logger::Print("Script {} encountered an error while loading. (doesn't exist?)", path);
				continue;
			}

			Logger::Print("Script {}.gsc loaded successfully.\n", path);

			const auto initHandle = Game::Scr_GetFunctionHandle(path, "init");
			if (initHandle != 0)
			{
				ScriptInitHandles.insert_or_assign(path, initHandle);
			}

			const auto mainHandle = Game::Scr_GetFunctionHandle(path, "main");
			if (mainHandle != 0)
			{
				ScriptMainHandles.insert_or_assign(path, mainHandle);
			}

			// Allow scripts with no handles
		}

		Game::FS_FreeFileList(files, 10);
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
			for (const auto& func : CustomScrFunctions)
			{
				if (Utils::Contains(&func.aliases, name))
				{
					*type = func.type;
					return func.actionFunc;
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
			for (const auto& meth : CustomScrMethods)
			{
				if (Utils::Contains(&meth.aliases, name))
				{
					*type = meth.type;
					return meth.actionFunc;
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

	const char* Script::GetCodePosForParam(int index)
	{
		if (static_cast<unsigned int>(index) >= Game::scrVmPub->outparamcount)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "^1GetCodePosForParam: Index is out of range!\n");
			return "";
		}

		const auto* value = &Game::scrVmPub->top[-index];

		if (value->type != Game::VAR_FUNCTION)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "^1GetCodePosForParam: Expects a function as parameter!\n");
			return "";
		}

		return value->u.codePosValue;
	}

	void Script::GetReplacedPos(const char* pos)
	{
		if (ReplacedFunctions.contains(pos))
		{
			ReplacedPos = ReplacedFunctions[pos];
		}
	}

	void Script::SetReplacedPos(const char* what, const char* with)
	{
		if (what[0] == '\0' || with[0] == '\0')
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "Invalid parameters passed to ReplacedFunctions\n");
			return;
		}

		if (ReplacedFunctions.contains(what))
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "ReplacedFunctions already contains codePosValue for a function\n");
		}

		ReplacedFunctions[what] = with;
	}

	__declspec(naked) void Script::VMExecuteInternalStub()
	{
		__asm
		{
			pushad

			push edx
			call GetReplacedPos

			pop edx
			popad

			cmp ReplacedPos, 0
			jne SetPos

			movzx eax, byte ptr [edx]
			inc edx

		Loc1:
			cmp eax, 0x8B

			push ecx

			mov ecx, 0x2045094
			mov [ecx], eax

			mov ecx, 0x2040CD4
			mov [ecx], edx

			pop ecx

			push 0x61E944
			retn

		SetPos:
			mov edx, ReplacedPos
			mov ReplacedPos, 0

			movzx eax, byte ptr [edx]
			inc edx

			jmp Loc1
		}
	}

	Game::client_t* Script::GetClient(const Game::gentity_t* ent)
	{
		assert(ent);

		if (ent->client == nullptr)
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

	void Script::AddFunctions()
	{
		AddFunction("ReplaceFunc", [] // gsc: ReplaceFunc(<function>, <function>)
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("^1ReplaceFunc: Needs two parameters!\n");
				return;
			}

			const auto what = GetCodePosForParam(0);
			const auto with = GetCodePosForParam(1);

			SetReplacedPos(what, with);
		});

		// System time
		AddFunction("GetSystemMilliseconds", [] // gsc: GetSystemMilliseconds()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wMilliseconds);
		});

		// Executes command to the console
		AddFunction("Exec", [] // gsc: Exec(<string>)
		{
			const auto str = Game::Scr_GetString(0);

			if (str == nullptr)
			{
				Game::Scr_ParamError(0, "^1Exec: Illegal parameter!\n");
				return;
			}

			Command::Execute(str, false);
		});

		// Allow printing to the console even when developer is 0
		AddFunction("PrintConsole", [] // gsc: PrintConsole(<string>)
		{
			for (std::size_t i = 0; i < Game::Scr_GetNumParam(); ++i)
			{
				const auto* str = Game::Scr_GetString(i);

				if (str == nullptr)
				{
					Game::Scr_ParamError(i, "^1PrintConsole: Illegal parameter!\n");
					return;
				}

				Logger::Print(Game::level->scriptPrintChannel, "{}", str);
			}
		});

		// PlayerCmd_AreControlsFrozen GSC function from Black Ops 2
		AddMethod("AreControlsFrozen", [](Game::scr_entref_t entref) // Usage: self AreControlsFrozen();
		{
			const auto* ent = Scr_GetPlayerEntity(entref);

			Game::Scr_AddBool((ent->client->flags & Game::PF_FROZEN) != 0);
		});
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

		Utils::Hook(0x61E92E, VMExecuteInternalStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x61E933, 1);

		Scheduler::Loop([]
		{
			if (!Game::SV_Loaded())
				return;

			const auto nowMs = Game::Sys_Milliseconds();

			if (LastFrameTime != -1)
			{
				const auto timeTaken = (nowMs - LastFrameTime) * static_cast<int>((*Game::com_timescale)->current.value);

				if (timeTaken >= 500)
				{
					Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "Hitch warning: {} msec frame time\n", timeTaken);
				}
			}

			LastFrameTime = nowMs;
		}, Scheduler::Pipeline::SERVER);

#ifdef _DEBUG 
		AddFunction("DebugBox", []
		{
			const auto* message = Game::Scr_GetString(0);

			if (message == nullptr)
			{
				Game::Scr_Error("^1DebugBox: Illegal parameter!\n");
			}

			MessageBoxA(nullptr, message, "DEBUG", MB_OK);
		}, true);
#endif

		AddFunctions();

		Events::OnVMShutdown([]
		{
			ReplacedFunctions.clear();
		});
	}
}
