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

		Logger::Error(5, "script compile error\nunknown function %s\n%s\n\n", funcName.data(), Script::ScriptName.data());
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

	__declspec(naked) void Script::RestoreScriptNameStub()
	{
		__asm
		{
			call Script::RestoreScriptName

			mov ds:1CDEAA8h, ebp

			mov eax, 427E77h
			jmp eax
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
		char msgbuf[1024] = { 0 };
		va_list v;
		va_start(v, message);
		_vsnprintf_s(msgbuf, sizeof(msgbuf), message, v);
		va_end(v);

		Game::Scr_ShutdownAllocNode();

		Logger::Print(23, "\n");
		Logger::Print(23, "******* script compile error *******\n");
		Logger::Print(23, "Error: %s ", msgbuf);
		Script::PrintSourcePos(Script::ScriptName.data(), offset);
		Logger::Print(23, "************************************\n\n");

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

			if (Utils::String::EndsWith(file, ".gsc"))
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
		Utils::Hook(0x612DB0, Script::StoreFunctionNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427E71, Script::RestoreScriptNameStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427DBC, Script::StoreScriptNameStub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x612E8D, Script::FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x612EA2, Script::FunctionError, HOOK_CALL).install()->quick();
		Utils::Hook(0x434260, Script::CompileError, HOOK_JUMP).install()->quick();

		Utils::Hook(0x48EFFE, Script::LoadGameType, HOOK_CALL).install()->quick();
		Utils::Hook(0x45D44A, Script::LoadGameTypeScript, HOOK_CALL).install()->quick();
	}

	Script::~Script()
	{
		Script::ScriptName.clear();
		Script::ScriptHandles.clear();
		Script::ScriptNameStack.clear();
	}
}
