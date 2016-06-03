#include "STDInclude.hpp"

namespace Components
{
	std::string Script::ScriptName;
	std::vector<int> Script::ScriptHandles;
	std::vector<std::string> Script::ScriptNameStack;
	unsigned short Script::FunctionName;

	void Script::FunctionError()
	{
		std::string funcName = Game::SL_ConvertToString(Script::FunctionName);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(23, "\n");
		Logger::Print(23, "******* script compile error *******\n");
		Logger::Print(23, "Error: unknown function %s in %s\n", funcName.data(), Script::ScriptName.data());
		Logger::Print(23, "************************************\n");

		Logger::Error(5, "script compile error\nunknown function %s\n%s\n", funcName.data(), Script::ScriptName.data());
	}

	void __declspec(naked) Script::StoreFunctionNameStub()
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

	void Script::StoreScriptName(const char* name)
	{
		Script::ScriptNameStack.push_back(Script::ScriptName);
		Script::ScriptName = name;

		if (!Utils::EndsWith(Script::ScriptName, ".gsc"))
		{
			Script::ScriptName.append(".gsc");
		}
	}

	void __declspec(naked) Script::StoreScriptNameStub()
	{
		__asm
		{
			lea ecx, [esp + 10h]
			push ecx

			call Script::StoreScriptName
			add esp, 4h

			push ebp
			mov ebp, ds:1CDEAA8h
			mov ecx, 427DC3h
			jmp ecx
		}
	}

	void Script::RestoreScriptName()
	{
		Script::ScriptName = Script::ScriptNameStack.back();
		Script::ScriptNameStack.pop_back();
	}

	void __declspec(naked) Script::RestoreScriptNameStub()
	{
		__asm
		{
			call Script::RestoreScriptName

			mov ds:1CDEAA8h, ebp

			mov eax, 427E77h
			jmp eax
		}
	}

	// TODO: Optimize that!
	void Script::PrintSourcePos(const char* filename, int offset)
	{
		int line = 0;
		int inLineOffset = 0;
		char* scriptFile = 0;
		char* currentLine = 0;
		bool freeScript = false;

		if (Game::FS_ReadFile(filename, &scriptFile) > -1)
		{
			int globalOffset = 0;

			freeScript = true;

			for (char* c = scriptFile; *c != '\0'; c++)
			{
				if (!currentLine || *c == '\n')
				{
					line++;
					inLineOffset = 0;
					currentLine = c;
				}

				if (globalOffset == offset)
				{
					while (*c != '\r' && *c != '\n' && c != '\0')
					{
						c++;
					}

					*c = '\0';
					break;
				}

				if (*c == '\t')
				{
					*c = ' ';
				}

				globalOffset++;
				inLineOffset++;
			}
		}

		Logger::Print(23, "in file %s, line %d:", filename, line);

		if (currentLine)
		{
			Logger::Print(23, "%s\n", currentLine);

			for (int i = 0; i < (inLineOffset - 1); i++)
			{
				Logger::Print(23, " ");
			}

			Logger::Print(23, "*\n");
		}
		else
		{
			Logger::Print(23, "\n");
		}

		if (freeScript)
		{
			Game::FS_FreeFile(scriptFile);
		}
	}

	void Script::CompileError(int offset, const char* message, ...)
	{
		char msgbuf[1024] = { 0 };
		va_list v;
		va_start(v, message);
		_vsnprintf(msgbuf, sizeof(msgbuf), message, v);
		va_end(v);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(23, "\n");
		Logger::Print(23, "******* script compile error *******\n");
		Logger::Print(23, "Error: %s ", msgbuf);
		Script::PrintSourcePos(Script::ScriptName.data(), offset);
		Logger::Print(23, "************************************\n");

		Logger::Error(5, "script compile error\n%s\n%s\n(see console for actual details)\n", msgbuf, Script::ScriptName.data());
	}

	int Script::LoadScriptAndLabel(std::string script, std::string label)
	{
		Logger::Print("Loading script %s.gsc...\n", script.data());

		if (!Game::Scr_LoadScript(script.data()))
		{
			Logger::Print("Script %s encountered an error while loading. (doesn't exist?)", script.data());
			Logger::Error(1, (char*)0x70B810, script.data());
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

			if (Utils::EndsWith(file, ".gsc"))
			{
				file = file.substr(0, file.size() - 4);
			}

			int handle = Script::LoadScriptAndLabel(file, "init");

			if (handle)
			{
				Script::ScriptHandles.push_back(handle);
			}
		}

		Game::GScr_LoadGameTypeScript();
	}

	Script::Script()
	{
		Utils::Hook(0x612DB0, Script::StoreFunctionNameStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x427E71, Script::RestoreScriptNameStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x427DBC, Script::StoreScriptNameStub, HOOK_JUMP).Install()->Quick();

		Utils::Hook(0x612E8D, Script::FunctionError, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x612EA2, Script::FunctionError, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x434260, Script::CompileError, HOOK_JUMP).Install()->Quick();

		Utils::Hook(0x48EFFE, Script::LoadGameType, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x45D44A, Script::LoadGameTypeScript, HOOK_CALL).Install()->Quick();
	}

	Script::~Script()
	{
		Script::ScriptName.clear();
		Script::ScriptHandles.clear();
		Script::ScriptNameStack.clear();
	}
}
