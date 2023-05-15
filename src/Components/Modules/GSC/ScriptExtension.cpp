#include <STDInclude.hpp>

#include <Components/Modules/Events.hpp>

#include "ScriptExtension.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	std::unordered_map<const char*, const char*> ScriptExtension::ReplacedFunctions;
	const char* ScriptExtension::ReplacedPos = nullptr;

	const char* ScriptExtension::GetCodePosForParam(int index)
	{
		if (static_cast<unsigned int>(index) >= Game::scrVmPub->outparamcount)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "GetCodePosForParam: Index is out of range!");
			return "";
		}

		const auto* value = &Game::scrVmPub->top[-index];

		if (value->type != Game::VAR_FUNCTION)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "GetCodePosForParam: Expects a function as parameter!");
			return "";
		}

		return value->u.codePosValue;
	}

	void ScriptExtension::GetReplacedPos(const char* pos)
	{
		if (!pos)
		{
			// This seems to happen often and there should not be pointers to NULL in our map
			return;
		}

		if (const auto itr = ReplacedFunctions.find(pos); itr != ReplacedFunctions.end())
		{
			ReplacedPos = itr->second;
		}
	}

	void ScriptExtension::SetReplacedPos(const char* what, const char* with)
	{
		if (!*what || !*with)
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

	__declspec(naked) void ScriptExtension::VMExecuteInternalStub()
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
			ret

		SetPos:
			mov edx, ReplacedPos
			mov ReplacedPos, 0

			movzx eax, byte ptr [edx]
			inc edx

			jmp Loc1
		}
	}

	void ScriptExtension::AddFunctions()
	{
		Script::AddFunction("IsArray", [] // gsc: IsArray(<object>)
		{
			auto type = Game::Scr_GetType(0);

			bool result;
			if (type == Game::VAR_POINTER)
			{
				type = Game::Scr_GetPointerType(0);
				assert(type >= Game::FIRST_OBJECT);
				result = (type == Game::VAR_ARRAY);
			}
			else
			{
				assert(type < Game::FIRST_OBJECT);
				result = false;
			}

			Game::Scr_AddBool(result);
		});

		Script::AddFunction("ReplaceFunc", [] // gsc: ReplaceFunc(<function>, <function>)
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("ReplaceFunc: Needs two parameters!");
				return;
			}

			const auto what = GetCodePosForParam(0);
			const auto with = GetCodePosForParam(1);

			SetReplacedPos(what, with);
		});


		Script::AddFunction("GetSystemMilliseconds", [] // gsc: GetSystemMilliseconds()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wMilliseconds);
		});

		Script::AddFunction("Exec", [] // gsc: Exec(<string>)
		{
			const auto* str = Game::Scr_GetString(0);
			if (!str)
			{
				Game::Scr_ParamError(0, "Exec: Illegal parameter!");
				return;
			}

			Command::Execute(str, false);
		});

		// Allow printing to the console even when developer is 0
		Script::AddFunction("PrintConsole", [] // gsc: PrintConsole(<string>)
		{
			for (std::size_t i = 0; i < Game::Scr_GetNumParam(); ++i)
			{
				const auto* str = Game::Scr_GetString(i);
				if (!str)
				{
					Game::Scr_ParamError(i, "PrintConsole: Illegal parameter!");
					return;
				}

				Logger::Print(Game::level->scriptPrintChannel, "{}", str);
			}
		});		
	}

	ScriptExtension::ScriptExtension()
	{
		AddFunctions();

		Utils::Hook(0x61E92E, VMExecuteInternalStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x61E933, 1);

		Events::OnVMShutdown([]
		{
			ReplacedFunctions.clear();
		});
	}
}
