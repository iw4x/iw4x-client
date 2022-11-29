#include <STDInclude.hpp>
#include "Script.hpp"

namespace Components
{
	std::vector<Script::ScriptFunction> Script::CustomScrFunctions;
	std::vector<Script::ScriptMethod> Script::CustomScrMethods;

	std::string Script::ScriptName;
	std::vector<std::string> Script::ScriptNameStack;
	unsigned short Script::FunctionName;
	std::unordered_map<int, std::string> Script::ScriptBaseProgramNum;
	int Script::LastFrameTime = -1;

	std::unordered_map<const char*, const char*> Script::ReplacedFunctions;
	const char* Script::ReplacedPos = nullptr;

	std::unordered_map<std::string, int> Script::ScriptMainHandles;
	std::unordered_map<std::string, int> Script::ScriptInitHandles;

	void Script::FunctionError()
	{
		const auto* funcName = Game::SL_ConvertToString(FunctionName);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "\n");
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "******* script compile error *******\n");
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "Error: unknown function {} in {}\n", funcName, ScriptName);
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "************************************\n");

		Logger::Error(Game::ERR_SCRIPT_DROP, "script compile error\nunknown function {}\n{}\n\n", funcName, ScriptName);
	}

	__declspec(naked) void Script::StoreFunctionNameStub()
	{
		__asm
		{
			mov eax, [esp - 8h]
			mov FunctionName, ax

			sub esp, 0Ch
			push 0
			push edi

			mov eax, 612DB6h
			jmp eax
		}
	}

	void Script::RuntimeError(const char* codePos, unsigned int index, const char* msg, const char* dialogMessage)
	{
		// Allow error messages to be printed if developer mode is on
		// Should check scrVarPub.developer but it's absent
		// in this version of the game so let's check the dvar
		if (!Game::scrVmPub->terminal_error && !(*Game::com_developer)->current.integer)
			return;

		// If were are developing let's call RuntimeErrorInternal
		// scrVmPub.debugCode seems to be always false
		if (Game::scrVmPub->debugCode || Game::scrVarPub->developer_script)
		{
			Game::RuntimeErrorInternal(Game::CON_CHANNEL_PARSERSCRIPT, codePos, index, msg);
		}
		else
		{
			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "{}\n", msg);
		}

		// Let's not throw error unless we have to
		if (Game::scrVmPub->terminal_error)
		{
			if (dialogMessage == nullptr)
				dialogMessage = "";

			Logger::Error(Game::ERR_SCRIPT_DROP, "\x15script runtime error\n(see console for details)\n{}\n{}", msg, dialogMessage);
		}
	}

	void Script::StoreScriptName(const char* name)
	{
		ScriptNameStack.push_back(ScriptName);
		ScriptName = name;

		if (!Utils::String::EndsWith(ScriptName, ".gsc"))
		{
			ScriptName.append(".gsc");
		}
	}

	__declspec(naked) void Script::StoreScriptNameStub()
	{
		__asm
		{
			pushad

			lea ecx, [esp + 30h]
			push ecx

			call StoreScriptName
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
		ScriptName = ScriptNameStack.back();
		ScriptNameStack.pop_back();
	}

	__declspec(naked) void Script::RestoreScriptNameStub()
	{
		__asm
		{
			pushad
			call RestoreScriptName
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

			auto line = 1, lineOffset = 0, inlineOffset = 0;

			for (size_t i = 0; i < buffer.size(); ++i)
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
					lineOffset = static_cast<int>(i); // Includes the line break!
					inlineOffset = 0;
				}
				else
				{
					++inlineOffset;
				}
			}

			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "in file {}, line {}:", filename, line);
			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "{}\n", buffer.substr(lineOffset));

			for (auto i = 0; i < (inlineOffset - 1); ++i)
			{
				Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, " ");
			}

			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "*\n");
		}
		else
		{
			Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "in file {}, offset {}\n", filename, offset);
		}
	}

	void Script::CompileError(unsigned int offset, const char* message, ...)
	{
		char msgbuf[1024] = {0};
		va_list va;
		va_start(va, message);
		_vsnprintf_s(msgbuf, _TRUNCATE, message, va);
		va_end(va);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "\n");
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "******* script compile error *******\n");
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "Error: {} ", msgbuf);
		PrintSourcePos(ScriptName.data(), offset);
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "************************************\n\n");

		Logger::Error(Game::ERR_SCRIPT_DROP, "script compile error\n{}\n{}\n(see console for actual details)\n", msgbuf, ScriptName);
	}

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
				if (std::ranges::find(func.aliases, name) != func.aliases.end())
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
				const auto& name = func.aliases[0];
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
				if (std::ranges::find(meth.aliases, name) != meth.aliases.end())
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
				const auto& name = meth.aliases[0];
				Game::Scr_RegisterFunction(reinterpret_cast<int>(meth.actionFunc), name.data());
			}
		}

		// If no method was found let's call game's function
		return Utils::Hook::Call<Game::BuiltinMethod(const char**, int*)>(0x5FA360)(pName, type); // Player_GetMethod
	}

	void Script::StoreScriptBaseProgramNum()
	{
		ScriptBaseProgramNum.insert_or_assign(Utils::Hook::Get<int>(0x1CFEEF8), ScriptName);
	}

	void Script::Scr_PrintPrevCodePos(int scriptPos)
	{
		auto bestCodePos = -1, nextCodePos = -1, offset = -1;
		std::string file;

		for (const auto& [key, value] : ScriptBaseProgramNum)
		{
			const auto codePos = key;

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

		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "\n@ {} ({} - {})\n", scriptPos, bestCodePos, nextCodePos);
		Logger::Print(Game::CON_CHANNEL_PARSERSCRIPT, "in {} ({} through the source)\n\n", file, ((offset * 100.0f) / (nextCodePos - bestCodePos)));
	}

	__declspec(naked) void Script::Scr_PrintPrevCodePosStub()
	{
		__asm
		{
			push esi
			call Scr_PrintPrevCodePos
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
			call StoreScriptBaseProgramNum
			popad

			// execute overwritten code caused by the jump hook
			sub eax, ds:201A460h // gScrVarPub_programBuffer
			add esp, 0Ch
			mov ds:1CFEEF8h, eax // gScrCompilePub_programLen

			// jump back to the original code
			push 426C3Bh
			retn
		}
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

			Game::Scr_AddBool((ent->client->flags & Game::PLAYER_FLAG_FROZEN) != 0);
		});
	}

	Script::Script()
	{
		Utils::Hook(0x612DB0, StoreFunctionNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427E71, RestoreScriptNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427DBC, StoreScriptNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x426C2D, StoreScriptBaseProgramNumStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x42281B, Scr_PrintPrevCodePosStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x61E3AD, RuntimeError, HOOK_CALL).install()->quick();
		Utils::Hook(0x621976, RuntimeError, HOOK_CALL).install()->quick();
		Utils::Hook(0x62246E, RuntimeError, HOOK_CALL).install()->quick();
		// Skip check in GScr_CheckAllowedToSetPersistentData to prevent log spam in RuntimeError.
		// On IW5 the function is entirely nullsubbed
		Utils::Hook::Set<std::uint8_t>(0x5F8DBF, 0xEB);

		Utils::Hook(0x612E8D, FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x612EA2, FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x434260, CompileError, HOOK_JUMP).install()->quick();

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
