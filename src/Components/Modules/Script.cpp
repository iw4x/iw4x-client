#include "STDInclude.hpp"

namespace Components
{
	std::string Script::ScriptName;
	std::vector<int> Script::ScriptHandles;
	std::vector<Script::Function> Script::ScriptFunctions;
	std::vector<std::string> Script::ScriptNameStack;
	unsigned short Script::FunctionName;
	std::unordered_map<std::string, std::string> Script::ScriptStorage;
	std::unordered_map<int, std::string> Script::ScriptBaseProgramNum;
	std::unordered_map<const char*, const char*> Script::ReplacedFunctions;
	const char* Script::ReplacedPos = nullptr;
	int Script::LastFrameTime = -1;

	Utils::Signal<Scheduler::Callback> Script::VMShutdownSignal;

	void Script::FunctionError()
	{
		std::string funcName = Game::SL_ConvertToString(Script::FunctionName);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(23, "\n");
		Logger::Print(23, "******* script compile error *******\n");
		Logger::Print(23, "Error: unknown function %s in %s\n", funcName.data(), Script::ScriptName.data());
		Logger::Print(23, "************************************\n");

		Logger::Error(Game::ERR_SCRIPT_DROP, "script compile error\nunknown function %s\n%s\n\n", funcName.data(), Script::ScriptName.data());
	}

	__declspec(naked) void Script::StoreFunctionNameStub()
	{
		__asm
		{
			mov eax, [esp - 8h]
			mov Script::FunctionName, ax

			sub esp, 0Ch
			push 0
			push edi

			mov eax, 612DB6h
			jmp eax
		}
	}

	void Script::RuntimeError(const char* codePos, unsigned int index, const char* msg, const char* dialogMessage)
	{
		const auto developer = Dvar::Var("developer").get<int>();

		// Allow error messages to be printed if developer mode is on
		// Should check scrVarPub.developer but it's absent
		// in this version of the game so let's check the dvar
		if (!Game::scrVmPub->terminal_error && !developer)
			return;

		// If were are developing let's call RuntimeErrorInternal
		// scrVmPub.debugCode seems to be always false
		if (Game::scrVmPub->debugCode || Game::scrVarPub->developer_script)
		{
			Game::RuntimeErrorInternal(23, codePos, index, msg);

			if (!Game::scrVmPub->terminal_error)
				return;
		}
		else
		{
			Logger::Print(23, "%s\n", msg);
			// Let's not throw error unless we have to
			if (Game::scrVmPub->abort_on_error && !Game::scrVmPub->terminal_error)
				return;
		}

		if (dialogMessage == nullptr)
			dialogMessage = "";

		const auto errorLevel = (Game::scrVmPub->terminal_error) ? Game::ERR_SCRIPT_DROP : Game::ERR_SCRIPT;
		Logger::Error(errorLevel, "\x15script runtime error\n(see console for details)\n%s\n%s", msg, dialogMessage);
	}

	void Script::StoreScriptName(const char* name)
	{
		Script::ScriptNameStack.push_back(Script::ScriptName);
		Script::ScriptName = name;

		if (!Utils::String::EndsWith(Script::ScriptName, ".gsc"))
		{
			Script::ScriptName.append(".gsc");
		}
	}

	__declspec(naked) void Script::StoreScriptNameStub()
	{
		__asm
		{
			pushad

			lea ecx, [esp + 30h]
			push ecx

			call Script::StoreScriptName
			add esp, 4h

			popad

			push ebp
			mov ebp, ds:1CDEAA8h

			push 427DC3h
			retn
		}
	}

	void Script::RestoreScriptName()
	{
		Script::ScriptName = Script::ScriptNameStack.back();
		Script::ScriptNameStack.pop_back();
	}

	__declspec(naked) void Script::RestoreScriptNameStub()
	{
		__asm
		{
			pushad
			call Script::RestoreScriptName
			popad

			mov ds:1CDEAA8h, ebp

			push 427E77h
			retn
		}
	}

	void Script::PrintSourcePos(const char* filename, unsigned int offset)
	{
		FileSystem::File script(filename);

		if (script.exists())
		{
			std::string buffer = script.getBuffer();
			Utils::String::Replace(buffer, "\t", " ");

			int line = 1;
			int lineOffset = 0;
			int inlineOffset = 0;

			for (unsigned int i = 0; i < buffer.size(); ++i)
			{
				// Terminate line
				if (i == offset)
				{
					while (buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\0')
					{
						++i;
					}

					buffer[i] = '\0';
					break;
				}

				if (buffer[i] == '\n')
				{
					++line;
					lineOffset = i; // Includes the line break!
					inlineOffset = 0;
				}
				else
				{
					++inlineOffset;
				}
			}

			Logger::Print(23, "in file %s, line %d:", filename, line);
			Logger::Print(23, "%s\n", buffer.data() + lineOffset);

			for (int i = 0; i < (inlineOffset - 1); ++i)
			{
				Logger::Print(23, " ");
			}

			Logger::Print(23, "*\n");
		}
		else
		{
			Logger::Print(23, "in file %s, offset %d\n", filename, offset);
		}
	}

	void Script::CompileError(unsigned int offset, const char* message, ...)
	{
		char msgbuf[1024] = {0};
		va_list va;
		va_start(va, message);
		vsnprintf_s(msgbuf, sizeof(msgbuf), _TRUNCATE, message, va);
		va_end(va);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(23, "\n");
		Logger::Print(23, "******* script compile error *******\n");
		Logger::Print(23, "Error: %s ", msgbuf);
		Script::PrintSourcePos(Script::ScriptName.data(), offset);
		Logger::Print(23, "************************************\n\n");

		Logger::Error(Game::ERR_SCRIPT_DROP, "script compile error\n%s\n%s\n(see console for actual details)\n", msgbuf, Script::ScriptName.data());
	}

	int Script::LoadScriptAndLabel(const std::string& script, const std::string& label)
	{
		Logger::Print("Loading script %s.gsc...\n", script.data());

		if (!Game::Scr_LoadScript(script.data()))
		{
			Logger::Print("Script %s encountered an error while loading. (doesn't exist?)", script.data());
			Logger::Error(Game::ERR_DROP, reinterpret_cast<const char*>(0x70B810), script.data());
		}
		else
		{
			Logger::Print("Script %s.gsc loaded successfully.\n", script.data());
		}

		Logger::Print("Finding script handle %s::%s...\n", script.data(), label.data());
		int handle = Game::Scr_GetFunctionHandle(script.data(), label.data());
		if (handle)
		{
			Logger::Print("Script handle %s::%s loaded successfully.\n", script.data(), label.data());
			return handle;
		}

		Logger::Print("Script handle %s::%s couldn't be loaded. (file with no entry point?)\n", script.data(), label.data());
		return handle;
	}

	void Script::LoadGameType()
	{
		for (auto handle : Script::ScriptHandles)
		{
			Game::Scr_FreeThread(Game::Scr_ExecThread(handle, 0));
		}

		Game::Scr_LoadGameType();
	}

	void Script::LoadGameTypeScript()
	{
		Script::ScriptHandles.clear();

		auto list = FileSystem::GetFileList("scripts/", "gsc");

		for (auto file : list)
		{
			file = "scripts/" + file;

			if (Utils::String::EndsWith(file, ".gsc"))
			{
				file = file.substr(0, file.size() - 4);
			}

			int handle = Script::LoadScriptAndLabel(file, "init");
			if (handle) Script::ScriptHandles.push_back(handle);
			else
			{
				handle = Script::LoadScriptAndLabel(file, "main");
				if (handle) Script::ScriptHandles.push_back(handle);
			}
		}

		Game::GScr_LoadGameTypeScript();
	}

	void Script::AddFunction(const std::string& name, Game::scr_function_t function, bool isDev)
	{
		for (auto i = Script::ScriptFunctions.begin(); i != Script::ScriptFunctions.end();)
		{
			if (i->getName() == name)
			{
				i = Script::ScriptFunctions.erase(i);
				continue;
			}

			++i;
		}

		Script::ScriptFunctions.push_back({ name, function, isDev });
	}

	Game::scr_function_t Script::GetFunction(void* caller, const char** name, int* isDev)
	{
		for (auto& function : Script::ScriptFunctions)
		{
			if (name && *name)
			{
				if (Utils::String::ToLower(*name) == Utils::String::ToLower(function.getName()))
				{
					*name = function.getName();
					*isDev = function.isDev();
					return function.getFunction();
				}
			}
			else if (caller == reinterpret_cast<void*>(0x465781))
			{
				Game::Scr_RegisterFunction(function.getFunction());
			}
		}

		return nullptr;
	}

	__declspec(naked) void Script::GetFunctionStub()
	{
		__asm
		{
			test eax, eax
			jnz returnSafe

			sub esp, 8h
			push [esp + 10h]
			call Script::GetFunction
			add esp, 0Ch

		returnSafe:
			pop edi
			pop esi
			retn
		}
	}

	void Script::StoreScriptBaseProgramNum()
	{
		Script::ScriptBaseProgramNum.insert_or_assign(Utils::Hook::Get<int>(0x1CFEEF8), Script::ScriptName);
	}

	void Script::Scr_PrintPrevCodePos(int scriptPos)
	{
		int bestCodePos = -1;
		int nextCodePos = -1;
		int offset = -1;
		std::string file;

		for (const auto& [key, value] : Script::ScriptBaseProgramNum)
		{
			int codePos = key;

			if (codePos > scriptPos)
			{
				if (nextCodePos == -1 || codePos < nextCodePos)
					nextCodePos = codePos;

				continue;
			}

			if (codePos < bestCodePos)
				continue;

			bestCodePos = codePos;

			file = value;
			offset = scriptPos - bestCodePos;
		}

		if (bestCodePos == -1)
			return;

		float onehundred = 100.0;

		Logger::Print(23, "\n@ %d (%d - %d)\n", scriptPos, bestCodePos, nextCodePos);
		Logger::Print(23, "in %s (%.1f%% through the source)\n\n", file.c_str(), ((offset * onehundred) / (nextCodePos - bestCodePos)));
	}

	__declspec(naked) void Script::Scr_PrintPrevCodePosStub()
	{
		__asm
		{
			push esi
			call Script::Scr_PrintPrevCodePos
			add esp, 4h

			pop esi
			retn
		}
	}

	__declspec(naked) void Script::StoreScriptBaseProgramNumStub()
	{
		__asm
		{
			// execute our hook
			pushad

			call Script::StoreScriptBaseProgramNum

			popad

			// execute overwritten code caused by the jump hook
			sub     eax, ds:201A460h // gScrVarPub_programBuffer
			add     esp, 0Ch
			mov     ds : 1CFEEF8h, eax // gScrCompilePub_programLen

			// jump back to the original code
			push    426C3Bh
			retn
		}
	}

	void Script::OnVMShutdown(Utils::Slot<Scheduler::Callback> callback)
	{
		Script::ScriptBaseProgramNum.clear();
		Script::VMShutdownSignal.connect(callback);
	}

	void Script::ScrShutdownSystemStub(int num)
	{
		Script::VMShutdownSignal();

		// Scr_ShutdownSystem
		Utils::Hook::Call<void(int)>(0x421EE0)(num);
	}

	unsigned int Script::SetExpFogStub()
	{
		if (Game::Scr_GetNumParam() == 6u)
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

		const auto value = &Game::scrVmPub->top[-index];

		if (value->type != Game::VAR_FUNCTION)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "^1GetCodePosForParam: Expects a function as parameter!\n");
			return "";
		}

		return value->u.codePosValue;
	}

	void Script::GetReplacedPos(const char* pos)
	{
		if (Script::ReplacedFunctions.find(pos) != Script::ReplacedFunctions.end())
		{
			Script::ReplacedPos = Script::ReplacedFunctions[pos];
		}
	}

	void Script::SetReplacedPos(const char* what, const char* with)
	{
		if (what[0] == '\0' || with[0] == '\0')
		{
			Logger::Print("Warning: Invalid paramters passed to ReplacedFunctions\n");
			return;
		}

		if (Script::ReplacedFunctions.find(what) != Script::ReplacedFunctions.end())
		{
			Logger::Print("Warning: ReplacedFunctions already contains codePosValue for a function\n");
		}

		Script::ReplacedFunctions[what] = with;
	}

	__declspec(naked) void Script::VMExecuteInternalStub()
	{
		__asm
		{
			pushad

			push edx
			call Script::GetReplacedPos

			pop edx
			popad

			cmp Script::ReplacedPos, 0
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
			mov edx, Script::ReplacedPos
			mov Script::ReplacedPos, 0

			movzx eax, byte ptr [edx]
			inc edx

			jmp Loc1
		}
	}

	Game::gentity_t* Script::GetEntity(const Game::scr_entref_t entref)
	{
		if (entref.classnum != 0 || entref.entnum >= Game::MAX_GENTITIES)
		{
			Game::Scr_ObjectError("Not an entity");
			return nullptr;
		}

		return &Game::g_entities[entref.entnum];
	}

	Game::client_t* Script::GetClient(const Game::gentity_t* gentity)
	{
		if (gentity->client == nullptr)
		{
			Game::Scr_ObjectError(Utils::String::VA("Entity %i is not a player", gentity->s.number));
			return nullptr;
		}

		if (gentity->s.number >= *Game::svs_numclients)
		{
			Game::Scr_ObjectError(Utils::String::VA("Entity %i is out of bounds", gentity->s.number));
			return nullptr;
		}

		return &Game::svs_clients[gentity->s.number];
	}

	void Script::AddFunctions()
	{
		Script::AddFunction("ReplaceFunc", [](Game::scr_entref_t) // gsc: ReplaceFunc(<function>, <function>)
		{
			if (Game::Scr_GetNumParam() != 2u)
			{
				Game::Scr_Error("^1ReplaceFunc: Needs two parameters!\n");
				return;
			}

			const auto what = Script::GetCodePosForParam(0);
			const auto with = Script::GetCodePosForParam(1);

			Script::SetReplacedPos(what, with);
		});

		// System time
		Script::AddFunction("GetSystemTime", [](Game::scr_entref_t) // gsc: GetSystemTime()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wSecond);
		});

		Script::AddFunction("GetSystemMilliseconds", [](Game::scr_entref_t) // gsc: GetSystemMilliseconds()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wMilliseconds);
		});

		// Executes command to the console
		Script::AddFunction("Exec", [](Game::scr_entref_t) // gsc: Exec(<string>)
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
		Script::AddFunction("PrintConsole", [](Game::scr_entref_t) // gsc: PrintConsole(<string>)
		{
			for (auto i = 0u; i < Game::Scr_GetNumParam(); i++)
			{
				const auto str = Game::Scr_GetString(i);

				if (str == nullptr)
				{
					Game::Scr_ParamError(i, "^1PrintConsole: Illegal parameter!\n");
					return;
				}

				Logger::Print(*Game::level_scriptPrintChannel, "%s", str);
			}
		});

		// Script Storage Funcs
		Script::AddFunction("StorageSet", [](Game::scr_entref_t) // gsc: StorageSet(<str key>, <str data>);
		{
			const auto* key = Game::Scr_GetString(0);
			const auto* value = Game::Scr_GetString(1);

			if (key == nullptr || value == nullptr)
			{
				Game::Scr_Error("^1StorageSet: Illegal parameters!\n");
				return;
			}

			Script::ScriptStorage.insert_or_assign(key, value);
		});

		Script::AddFunction("StorageRemove", [](Game::scr_entref_t) // gsc: StorageRemove(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_Error("^1StorageRemove: Illegal parameter!\n");
				return;
			}

			if (!Script::ScriptStorage.count(key))
			{
				Game::Scr_Error(Utils::String::VA("^1StorageRemove: Store does not have key '%s'!\n", key));
				return;
			}

			Script::ScriptStorage.erase(key);
		});

		Script::AddFunction("StorageGet", [](Game::scr_entref_t) // gsc: StorageGet(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_Error("^1StorageGet: Illegal parameter!\n");
				return;
			}

			if (!Script::ScriptStorage.count(key))
			{
				Game::Scr_Error(Utils::String::VA("^1StorageGet: Store does not have key '%s'!\n", key));
				return;
			}

			const auto& data = Script::ScriptStorage.at(key);
			Game::Scr_AddString(data.data());
		});

		Script::AddFunction("StorageHas", [](Game::scr_entref_t) // gsc: StorageHas(<str key>);
		{
			const auto* key = Game::Scr_GetString(0);

			if (key == nullptr)
			{
				Game::Scr_Error("^1StorageHas: Illegal parameter!\n");
				return;
			}

			Game::Scr_AddBool(Script::ScriptStorage.count(key));
		});

		Script::AddFunction("StorageClear", [](Game::scr_entref_t) // gsc: StorageClear();
		{
			Script::ScriptStorage.clear();
		});

		// PlayerCmd_AreControlsFrozen GSC function from Black Ops 2
		Script::AddFunction("AreControlsFrozen", [](Game::scr_entref_t entref) // Usage: self AreControlsFrozen();
		{
			const auto* ent = Script::GetEntity(entref);

			if (ent->client == nullptr)
			{
				Game::Scr_ObjectError(Utils::String::VA("Entity %i is not a player", ent->s.number));
				return;
			}

			Game::Scr_AddBool((ent->client->flags & Game::PLAYER_FLAG_FROZEN) != 0);
		});
	}

	Script::Script()
	{
		Utils::Hook(0x612DB0, Script::StoreFunctionNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427E71, Script::RestoreScriptNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427DBC, Script::StoreScriptNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x426C2D, Script::StoreScriptBaseProgramNumStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x42281B, Script::Scr_PrintPrevCodePosStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x61E3AD, Script::RuntimeError, HOOK_CALL).install()->quick();
		Utils::Hook(0x621976, Script::RuntimeError, HOOK_CALL).install()->quick();
		Utils::Hook(0x62246E, Script::RuntimeError, HOOK_CALL).install()->quick();
		// Nullsub GScr_CheckAllowedToSetPersistentData like it's done on IW5 to prevent spam
		Utils::Hook::Set<BYTE>(0x5F8DA0, 0xC3);

		Utils::Hook(0x612E8D, Script::FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x612EA2, Script::FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x434260, Script::CompileError, HOOK_JUMP).install()->quick();

		Utils::Hook(0x48EFFE, Script::LoadGameType, HOOK_CALL).install()->quick();
		Utils::Hook(0x45D44A, Script::LoadGameTypeScript, HOOK_CALL).install()->quick();

		Utils::Hook(0x44E736, Script::GetFunctionStub, HOOK_JUMP).install()->quick(); // Scr_GetFunction
		Utils::Hook(0x4EC8E5, Script::GetFunctionStub, HOOK_JUMP).install()->quick(); // Scr_GetMethod

		Utils::Hook(0x5F41A3, Script::SetExpFogStub, HOOK_CALL).install()->quick();

		Utils::Hook(0x61E92E, Script::VMExecuteInternalStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x61E933, 1);

		Utils::Hook(0x47548B, Script::ScrShutdownSystemStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D06BA, Script::ScrShutdownSystemStub, HOOK_CALL).install()->quick();

		Scheduler::OnFrame([]()
		{
			if (!Game::SV_Loaded())
				return;

			int nowMs = Game::Sys_Milliseconds();

			if (Script::LastFrameTime != -1)
			{
				int timeTaken = static_cast<int>((nowMs - Script::LastFrameTime) * Dvar::Var("timescale").get<float>());

				if (timeTaken >= 500)
					Logger::Print(23, "Hitch warning: %i msec frame time\n", timeTaken);
			}

			Script::LastFrameTime = nowMs;
		});

		Script::AddFunction("debugBox", [](Game::scr_entref_t)
		{
			MessageBoxA(nullptr, Game::Scr_GetString(0), "DEBUG", 0);
		}, true);

		Script::AddFunctions();

		Script::OnVMShutdown([]
		{
			Script::ReplacedFunctions.clear();
		});
	}

	Script::~Script()
	{
		Script::ScriptName.clear();
		Script::ScriptHandles.clear();
		Script::ScriptNameStack.clear();
		Script::ScriptFunctions.clear();
		Script::ReplacedFunctions.clear();
		Script::VMShutdownSignal.clear();

		Script::ScriptStorage.clear();
		Script::ScriptBaseProgramNum.clear();
	}
}
